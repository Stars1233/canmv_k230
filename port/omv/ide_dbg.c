/*
 * This file is part of the OpenMV project.
 * Copyright (c) 2013/2014 Ibrahim Abdelkader <i.abdalkader@gmail.com>
 * This work is licensed under the MIT license, see the file LICENSE for details.
 *
 * IDE debug protocol handler.
 */

/* System headers */
#include <stdarg.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <pthread.h>
#include <semaphore.h>
#include <sys/time.h>
#include <sys/stat.h>
#include <dirent.h>
#include <errno.h>

/* MicroPython core */
#include "py/mpconfig.h"
#include "py/mpstate.h"
#include "py/mpthread.h"
#include "py/obj.h"
#include "py/objstr.h"
#include "py/runtime.h"
#include "shared/readline/readline.h"

/* Project / driver headers */
#include "mbedtls/version.h"

#if MBEDTLS_VERSION_NUMBER >= 0x03000000
#include "mbedtls/compat-2.x.h"
#endif

#include "mbedtls/sha256.h"

#include "ide_dbg.h"
#include "version.h"
#include "canmv_drivers.h"
#include "drv_uart.h"
#include "hal_rvv_ops.h"

#if MBEDTLS_VERSION_NUMBER < 0x02070000 || MBEDTLS_VERSION_NUMBER >= 0x03000000
#define mbedtls_sha256_starts_ret mbedtls_sha256_starts
#define mbedtls_sha256_update_ret mbedtls_sha256_update
#define mbedtls_sha256_finish_ret mbedtls_sha256_finish
#endif

#if CONFIG_CANMV_IDE_SUPPORT

///////////////////////////////////////////////////////////////////////////////
// Debug logging macros ///////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

#define COLOR_NONE "\033[0m"
#define RED "\033[1;31;40m"
#define BLUE "\033[1;34;40m"
#define GREEN "\033[1;32;40m"
#define YELLOW "\033[1;33;40m"

#define IDE_DBG_LOG 0

#if IDE_DBG_LOG
#define pr_verb(fmt,...) fprintf(stderr,BLUE "[ide] " fmt "\n" COLOR_NONE,##__VA_ARGS__)
#define pr_dbg(fmt,...) fprintf(stderr,YELLOW "[ide-dbg] " fmt "\n" COLOR_NONE,##__VA_ARGS__)
#else
#define pr_verb(fmt,...)
#define pr_dbg(fmt,...)
#endif
#define pr_info(fmt,...) fprintf(stderr,GREEN fmt "\n"COLOR_NONE,##__VA_ARGS__)
#define pr_warn(fmt,...) fprintf(stderr,YELLOW fmt "\n"COLOR_NONE,##__VA_ARGS__)
#define pr_err(fmt,...) fprintf(stderr,RED fmt " at line %d\n"COLOR_NONE,##__VA_ARGS__, __LINE__)

#define PRINT_ALL 0
#define IDE_DBG_MAX_FRAME_PAYLOAD (8 * 1024 * 1024)

///////////////////////////////////////////////////////////////////////////////
// Extern declarations & state ////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

pthread_t ide_dbg_task_p;
static struct ide_dbg_svfil_t ide_dbg_sv_file;
static uint8_t ide_dbg_file_read_buffer[USBDBG_FILE_CHUNK_MAX];
static mp_obj_exception_t ide_exception; //IDE interrupt
static mp_obj_str_t ide_exception_str;
static mp_obj_tuple_t* ide_exception_str_tuple = NULL;

extern drv_uart_inst_t *mp_hal_uart_get_instance(void);
extern int mp_hal_uart_tx(const void* buffer, size_t size);
extern bool mp_hal_uart_is_usb(void);
extern void mp_hal_stdin_push(const uint8_t* data, size_t len);
extern void mp_hal_stdin_clear(void);
extern volatile bool is_repl_intr;

static uint64_t sys_mem_total_size = 0;

extern void system_set_exiting_flag(bool exiting);
extern bool in_mp_irq_handler(void);

static pthread_mutex_t vtouch_mutex = PTHREAD_MUTEX_INITIALIZER;
static bool vtouch_enabled = false;
static uint32_t vtouch_range_x = 0;
static uint32_t vtouch_range_y = 0;
static struct ide_dbg_vtouch_event vtouch_queue[USBDBG_VTOUCH_QUEUE_LEN];
static uint32_t vtouch_read_index = 0;
static uint32_t vtouch_count = 0;

///////////////////////////////////////////////////////////////////////////////
// Utility functions //////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

static void wait_mp_irq_handler_done(void)
{
    system_set_exiting_flag(true);

    for (int wait_count = 0; wait_count < 500; wait_count++) {

        if (!in_mp_irq_handler()) {
            break;
        }
        usleep(10000);
    }

#if 0
    for (int wait_count = 0; wait_count < 500; wait_count++) {
        mp_uint_t atomic_state = MICROPY_BEGIN_ATOMIC_SECTION();
        bool is_idle = (MP_STATE_VM(sched_state) == MP_SCHED_IDLE);
        MICROPY_END_ATOMIC_SECTION(atomic_state);

        if (is_idle) {
            break;
        }
        usleep(10000); // Sleep 10ms

    }
#endif
}

void print_raw(const uint8_t* data, size_t size) {
#if IDE_DEBUG_PRINT
    fprintf(stderr, "raw: \"");
    for (size_t i = 0; i < size; i++) {
        fprintf(stderr, "\\x%02X", ((unsigned char*)data)[i]);
    }
    fprintf(stderr, "\"\n");
#endif
}

///////////////////////////////////////////////////////////////////////////////
// TX ring buffer (stdout -> IDE) /////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

#define TX_BUF_SIZE (1024 * 128)
static char tx_buf[TX_BUF_SIZE];
static uint32_t tx_buf_w = 0;
static uint32_t tx_buf_r = 0;
static pthread_mutex_t tx_buf_mutex = PTHREAD_MUTEX_INITIALIZER;

static inline uint32_t tx_buf_readable(void) {
    return (tx_buf_w >= tx_buf_r)
        ? (tx_buf_w - tx_buf_r)
        : (TX_BUF_SIZE - tx_buf_r + tx_buf_w);
}

static inline uint32_t tx_buf_writable(void) {
    return TX_BUF_SIZE - 1 - tx_buf_readable();
}

static uint32_t tx_buf_readable_locked(void) {
    uint32_t len;

    pthread_mutex_lock(&tx_buf_mutex);
    len = tx_buf_readable();
    pthread_mutex_unlock(&tx_buf_mutex);

    return len;
}

static void tx_buf_write(const char* data, size_t len) {
    uint32_t tail = TX_BUF_SIZE - tx_buf_w;
    if (len <= tail) {
        hal_rvv_memcpy(tx_buf + tx_buf_w, data, len);
    } else {
        hal_rvv_memcpy(tx_buf + tx_buf_w, data, tail);
        hal_rvv_memcpy(tx_buf, data + tail, len - tail);
    }
    tx_buf_w = (tx_buf_w + len) % TX_BUF_SIZE;
}

static uint8_t tx_tmp_buf[TX_BUF_SIZE];

static void tx_buf_drain(uint32_t len) {
    uint32_t tail = TX_BUF_SIZE - tx_buf_r;
    uint8_t *tmp_buf = tx_tmp_buf;
    if (len <= tail) {
        hal_rvv_memcpy(tmp_buf, tx_buf + tx_buf_r, len);
        tx_buf_r += len;
    } else {
        hal_rvv_memcpy(tmp_buf, tx_buf + tx_buf_r, tail);
        hal_rvv_memcpy(tmp_buf + tail, tx_buf, len - tail);
        tx_buf_r = len - tail;
    }
    pthread_mutex_unlock(&tx_buf_mutex);
    mp_hal_uart_tx(tmp_buf, len);
    pthread_mutex_lock(&tx_buf_mutex);
}

void ide_dbg_stdout_tx(const char* data, size_t size) {    
    pthread_mutex_lock(&tx_buf_mutex);
    if (size > tx_buf_writable()) {
        // buffer full, drop to prevent UI stutter/locking
        pr_dbg("tx_buf FULL, dropping %zu bytes (readable=%u)", size, tx_buf_readable());
        pthread_mutex_unlock(&tx_buf_mutex);
        return;
    }
    tx_buf_write(data, size);
    pthread_mutex_unlock(&tx_buf_mutex);
}

static void read_until(void* buffer, size_t size) {
    size_t idx = 0;
    do {
        drv_uart_inst_t *inst = mp_hal_uart_get_instance();
        if (inst == NULL) {
            break;
        }

        int ret = drv_uart_poll(inst, 1000);
        if (ret <= 0) {
            MICROPY_EVENT_POLL_HOOK
            continue;
        }

        ssize_t recv = drv_uart_read(inst, (uint8_t*)buffer + idx, size - idx);
        if (recv < 0) {
            MICROPY_EVENT_POLL_HOOK
            continue;
        }

        idx += recv;
    } while (idx < size);
}

static void print_sha256(const uint8_t sha256[32]) {
    #if IDE_DEBUG_PRINT
    fprintf(stderr, "SHA256: ");
    for (unsigned i = 0; i < 32; i++) {
        fprintf(stderr, "%02x", sha256[i]);
    }
    fprintf(stderr, "\n");
    #endif
}

static uint32_t ide_dbg_vtouch_queue_depth_locked(void) {
    return vtouch_count;
}

void ide_dbg_vtouch_clear(void) {
    pthread_mutex_lock(&vtouch_mutex);
    vtouch_read_index = 0;
    vtouch_count = 0;
    pthread_mutex_unlock(&vtouch_mutex);
}

void ide_dbg_vtouch_open(uint32_t range_x, uint32_t range_y) {
    if (range_x == 0) {
        range_x = 1;
    }
    if (range_y == 0) {
        range_y = 1;
    }

    pthread_mutex_lock(&vtouch_mutex);
    vtouch_enabled = true;
    vtouch_range_x = range_x;
    vtouch_range_y = range_y;
    vtouch_read_index = 0;
    vtouch_count = 0;
    pthread_mutex_unlock(&vtouch_mutex);
}

void ide_dbg_vtouch_close(void) {
    pthread_mutex_lock(&vtouch_mutex);
    vtouch_enabled = false;
    vtouch_range_x = 0;
    vtouch_range_y = 0;
    vtouch_read_index = 0;
    vtouch_count = 0;
    pthread_mutex_unlock(&vtouch_mutex);
}

bool ide_dbg_vtouch_is_enabled(void) {
    bool enabled;

    pthread_mutex_lock(&vtouch_mutex);
    enabled = vtouch_enabled;
    pthread_mutex_unlock(&vtouch_mutex);

    return enabled;
}

uint32_t ide_dbg_vtouch_read(struct ide_dbg_vtouch_event *out, uint32_t max_events) {
    uint32_t read_count = 0;

    if (out == NULL || max_events == 0) {
        return 0;
    }

    pthread_mutex_lock(&vtouch_mutex);
    while (read_count < max_events && vtouch_count > 0) {
        out[read_count++] = vtouch_queue[vtouch_read_index];
        vtouch_read_index = (vtouch_read_index + 1) % USBDBG_VTOUCH_QUEUE_LEN;
        vtouch_count--;
    }
    pthread_mutex_unlock(&vtouch_mutex);

    return read_count;
}

static void ide_dbg_vtouch_status(struct ide_dbg_vtouch_info *info) {
    if (info == NULL) {
        return;
    }

    pthread_mutex_lock(&vtouch_mutex);
    info->supported = 1;
    info->enabled = vtouch_enabled ? 1 : 0;
    info->range_x = vtouch_enabled ? vtouch_range_x : 0;
    info->range_y = vtouch_enabled ? vtouch_range_y : 0;
    info->queue_depth = ide_dbg_vtouch_queue_depth_locked();
    pthread_mutex_unlock(&vtouch_mutex);
}

static bool ide_dbg_vtouch_enqueue(const struct ide_dbg_vtouch_event *event) {
    if (event == NULL) {
        return false;
    }

    pthread_mutex_lock(&vtouch_mutex);
    if (!vtouch_enabled) {
        pthread_mutex_unlock(&vtouch_mutex);
        return false;
    }

    uint32_t write_index = (vtouch_read_index + vtouch_count) % USBDBG_VTOUCH_QUEUE_LEN;
    vtouch_queue[write_index] = *event;
    if (vtouch_count < USBDBG_VTOUCH_QUEUE_LEN) {
        vtouch_count++;
    } else {
        vtouch_read_index = (vtouch_read_index + 1) % USBDBG_VTOUCH_QUEUE_LEN;
    }
    pthread_mutex_unlock(&vtouch_mutex);

    return true;
}

bool repl_script_running = false;

///////////////////////////////////////////////////////////////////////////////
// IDE state management ///////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

static uint32_t ide_script_running = 0;
static bool ide_soft_reset_requested = false;

void mpy_start_script(char* filepath);
void mpy_stop_script();
void interrupt_repl(void);
static char *exec_payload = NULL;
static exec_type_t exec_type = EXEC_STRING;
static sem_t script_sem;
static bool ide_attached = false;
static bool ide_disconnect = false;
static enum {
    FB_FROM_NONE,
    FB_FROM_USER_SET,
    FB_FROM_VO_WRITEBACK
} fb_from = FB_FROM_NONE, fb_from_current;

static inline bool ide_dbg_repl_script_running(void) {
    return __atomic_load_n(&repl_script_running, __ATOMIC_ACQUIRE);
}

static inline void ide_dbg_set_repl_script_running(bool running) {
    __atomic_store_n(&repl_script_running, running, __ATOMIC_RELEASE);
}

static inline uint32_t ide_dbg_script_running(void) {
    return __atomic_load_n(&ide_script_running, __ATOMIC_ACQUIRE);
}

bool ide_dbg_is_script_running(void) {
    return ide_dbg_script_running() != 0 ||
           ide_dbg_repl_script_running() ||
           __atomic_load_n(&ide_soft_reset_requested, __ATOMIC_ACQUIRE);
}

bool ide_dbg_has_script(void) {
    return ide_dbg_script_running() != 0;
}

static inline void ide_dbg_set_script_running(uint32_t running) {
    __atomic_store_n(&ide_script_running, running, __ATOMIC_RELEASE);
}

static inline void ide_dbg_set_soft_reset_request(bool requested) {
    __atomic_store_n(&ide_soft_reset_requested, requested, __ATOMIC_RELEASE);
}

bool ide_dbg_take_soft_reset_request(void) {
    return __atomic_load_n(&ide_soft_reset_requested, __ATOMIC_ACQUIRE);
}

void ide_dbg_clear_soft_reset_request(void) {
    ide_dbg_set_soft_reset_request(false);
}

static inline bool ide_dbg_is_attached(void) {
    return __atomic_load_n(&ide_attached, __ATOMIC_ACQUIRE);
}

static void ide_dbg_clear_fb(void);

static inline void ide_dbg_set_attached(bool attached) {
    __atomic_store_n(&ide_attached, attached, __ATOMIC_RELEASE);
}

static inline bool ide_dbg_should_disconnect(void) {
    return __atomic_load_n(&ide_disconnect, __ATOMIC_ACQUIRE);
}

static inline void ide_dbg_set_disconnect(bool disconnect) {
    __atomic_store_n(&ide_disconnect, disconnect, __ATOMIC_RELEASE);
}

static void ide_dbg_queue_script(char *payload, exec_type_t type) {
    exec_payload = payload;
    exec_type = type;
    ide_dbg_set_script_running(1);
    sem_post(&script_sem);
    interrupt_repl();
}

char* ide_dbg_get_script() {
    sem_wait(&script_sem);
    return ide_dbg_is_attached() ? exec_payload : NULL;
}

exec_type_t ide_dbg_get_exec_type(void) {
    return exec_type;
}

bool ide_dbg_attach(void) {
    return ide_dbg_is_attached();
}

void ide_dbg_on_script_start(void) {
    ide_dbg_set_repl_script_running(true);
}

void ide_dbg_on_script_end(void) {
    ide_dbg_clear_fb();
    ide_dbg_set_repl_script_running(false);
    ide_dbg_vtouch_close();
}

void ide_dbg_on_soft_reset(void) {
    ide_dbg_clear_fb();
    ide_dbg_set_repl_script_running(false);
    ide_dbg_vtouch_close();
}

void ide_dbg_on_ide_script_end(void) {
    if (exec_payload) {
        free(exec_payload);
        exec_payload = NULL;
    }
    exec_type = EXEC_STRING;
    // wait print done
    int count = 0;
    while (tx_buf_readable_locked() > 0 && count < 100) {
        usleep(1000);
        count++;
    }
    ide_dbg_set_script_running(0);
    if (ide_dbg_should_disconnect()) {
        ide_dbg_set_disconnect(false);
        ide_dbg_set_attached(false);
    }
    ide_dbg_on_script_end();
}

void interrupt_repl(void) {
    wait_mp_irq_handler_done();
    const uint8_t ctrl_cd[2] = { CHAR_CTRL_C, CHAR_CTRL_D };
    mp_hal_stdin_clear();
    mp_hal_stdin_push(&ctrl_cd[0], 1);
    mp_hal_stdin_push(&ctrl_cd[1], 1);
}

static void ide_dbg_request_soft_reset(void) {
    ide_dbg_set_soft_reset_request(true);
    mp_hal_stdin_clear();
    wait_mp_irq_handler_done();
    if (ide_dbg_is_script_running()) {
        mp_thread_set_exception_main(MP_OBJ_FROM_PTR(&ide_exception));
    }
    const uint8_t ctrl_d = CHAR_CTRL_D;
    mp_hal_stdin_push(&ctrl_d, 1);
}

void ide_dbg_interrupt(void) {
    pr_verb("[usb] exit IDE mode");
    wait_mp_irq_handler_done();
    ide_dbg_set_attached(false);
    sem_post(&script_sem);
}

static bool enable_pic = true;

///////////////////////////////////////////////////////////////////////////////
// Framebuffer management /////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

static void* fb_data = NULL;
static uint32_t fb_size = 0, fb_width = 0, fb_height = 0;

static void* fb_vo_wbc_data = NULL;
static size_t fb_vo_wbc_data_size = 0;
static uint32_t fb_vo_wbc_width = 0, fb_vo_wbc_height = 0;

static pthread_mutex_t fb_mutex;

static void ide_dbg_clear_fb_locked(void)
{
    if (fb_data) {
        free(fb_data);
        fb_data = NULL;
    }
    fb_size = 0;
    fb_width = 0;
    fb_height = 0;

    fb_vo_wbc_data = NULL;
    fb_vo_wbc_data_size = 0;
    fb_vo_wbc_width = 0;
    fb_vo_wbc_height = 0;
    fb_from = FB_FROM_NONE;
    fb_from_current = FB_FROM_NONE;
}

static void ide_dbg_clear_fb(void)
{
    pthread_mutex_lock(&fb_mutex);
    ide_dbg_clear_fb_locked();
    pthread_mutex_unlock(&fb_mutex);
}

// FIXME: reuse buf
void ide_set_fb(const void* data, uint32_t size, uint32_t width, uint32_t height) {
    pthread_mutex_lock(&fb_mutex);
    fb_from = FB_FROM_USER_SET;
    if (fb_data) {
        free(fb_data);
        fb_data = NULL;
        fb_size = 0;
    }
    fb_data = malloc(size);
    if(NULL == fb_data) {
        printf("malloc for fb failed");
        pthread_mutex_unlock(&fb_mutex);
        return;
    }
    hal_rvv_memcpy((void*)fb_data, data, size);
    fb_size = size;
    fb_width = width;
    fb_height = height;
    pthread_mutex_unlock(&fb_mutex);
}

void ide_dbg_enable_vo_wbc(void)
{
    pthread_mutex_lock(&fb_mutex);
    fb_from = FB_FROM_VO_WRITEBACK;
    pthread_mutex_unlock(&fb_mutex);
}

///////////////////////////////////////////////////////////////////////////////
// IDE command handlers ////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

static bool resolve_filepath(const char* name, char* out, size_t out_size) {
    if (name == NULL || out == NULL || out_size == 0) {
        return false;
    }

    int n;
    if (strlen(name) == 0 || strstr(name, "..")) {
        return false;
    }
    if (name[0] == '/') {
        n = snprintf(out, out_size, "%s", name);
    } else {
        n = snprintf(out, out_size, "/sdcard/%s", name);
    }
    return n >= 0 && (size_t)n < out_size;
}

static size_t read_from_frame(ide_dbg_state_t* state, void* buf, size_t size);
static uint32_t read_u32_from_frame(ide_dbg_state_t* state);


static void discard_from_frame(ide_dbg_state_t* state, size_t size) {
    uint8_t discard_buf[256];
    size_t remaining = size;

    while (remaining > 0) {
        size_t chunk = remaining;
        if (chunk > sizeof(discard_buf)) {
            chunk = sizeof(discard_buf);
        }
        read_from_frame(state, discard_buf, chunk);
        remaining -= chunk;
    }
}

static uint32_t file_errno_to_usbdbg(int err) {
    switch (err) {
        case 0: return USBDBG_SVFILE_ERR_NONE;
        case ENOENT: return USBDBG_SVFILE_ERR_FILE_NOT_FOUND;
        case EEXIST: return USBDBG_SVFILE_ERR_FILE_EXISTS;
        case EACCES:
        case EPERM: return USBDBG_SVFILE_ERR_PERM_DENIED;
        case ENOTEMPTY: return USBDBG_SVFILE_ERR_DIR_NOT_EMPTY;
        default: return USBDBG_SVFILE_ERR_IO_ERROR;
    }
}

static void close_active_sv_file(void) {
    if (ide_dbg_sv_file.file != NULL) {
        fclose(ide_dbg_sv_file.file);
        ide_dbg_sv_file.file = NULL;
    }
}

static bool is_dot_or_dotdot(const char *name) {
    return name[0] == '.' &&
        (name[1] == '\0' || (name[1] == '.' && name[2] == '\0'));
}

static void cmd_arch_str(ide_dbg_state_t* state) {
    char buffer[0x40];
    char unit;
    int value;

    if (state->data_length != sizeof(buffer))
        pr_verb("Warning: USBDBG_ARCH_STR data length %u, expected %lu", state->data_length, sizeof(buffer));

    if (sys_mem_total_size >= (1 << 30)) {
        value = sys_mem_total_size / (1 << 30);
        unit = 'G';
    } else if (sys_mem_total_size >= (1 << 20)) {
        value = sys_mem_total_size / (1 << 20);
        unit = 'M';
    } else if (sys_mem_total_size >= (1 << 10)) {
        value = sys_mem_total_size / (1 << 10);
        unit = 'K';
    } else {
        value = sys_mem_total_size;
        unit = 'B';
    }

    snprintf(buffer, sizeof(buffer), "%s [" OMV_BOARD_TYPE ":%08X%08X%08X]",
        OMV_ARCH_STR, value, unit, 0, 0, 0);
    pr_verb("cmd: USBDBG_ARCH_STR %s", buffer);
    mp_hal_uart_tx(buffer, sizeof(buffer));
}

static void cmd_script_exec(ide_dbg_state_t* state) {
    pr_verb("cmd: USBDBG_SCRIPT_EXEC size %u", state->data_length);
    if (ide_dbg_repl_script_running()) {
        mp_thread_set_exception_main(MP_OBJ_FROM_PTR(&ide_exception));
    }
    usleep(1000);
    if (ide_dbg_is_script_running()) {
        discard_from_frame(state, state->data_length);
        return;
    }
    exec_payload = malloc(state->data_length + 1);
    if (exec_payload == NULL) {
        pr_err("cmd: USBDBG_SCRIPT_EXEC alloc failed, size %u", state->data_length + 1);
        discard_from_frame(state, state->data_length);
        return;
    }
    read_from_frame(state, exec_payload, state->data_length);
    exec_payload[state->data_length] = '\0';
    ide_dbg_queue_script(exec_payload, EXEC_STRING);
}

#define IDE_CTRL_A (0x01)
#define IDE_CTRL_B (0x02)
#define IDE_CTRL_D (0x04)
#define IDE_CTRL_E (0x05)

static bool is_ide_terminal_control(uint8_t ch) {
    return ch == IDE_CTRL_A
        || ch == IDE_CTRL_B
        || ch == CHAR_CTRL_C
        || ch == IDE_CTRL_D
        || ch == IDE_CTRL_E;
}

static void cmd_tx_input(ide_dbg_state_t* state) {
    pr_dbg("[ide] cmd: USBDBG_TX_INPUT size %u attached=%d script_running=%u repl_running=%d",
            state->data_length,
            ide_dbg_attach(),
            ide_dbg_script_running(),
            ide_dbg_repl_script_running());

    if (state->data_length == 0) {
        return;
    }

    char *input = malloc(state->data_length);
    if (input == NULL) {
        pr_err("cmd: USBDBG_TX_INPUT alloc failed, size %u", state->data_length);
        discard_from_frame(state, state->data_length);
        return;
    }

    read_from_frame(state, input, state->data_length);
    if (state->data_length > 0) {
        pr_dbg("[ide] USBDBG_TX_INPUT payload first=0x%02x last=0x%02x len=%u",
                (uint8_t)input[0],
                (uint8_t)input[state->data_length - 1],
                state->data_length);
    }

    if (state->data_length == 1 && is_ide_terminal_control((uint8_t)input[0])) {
        uint8_t ctrl = (uint8_t)input[0];
        pr_dbg("[ide] USBDBG_TX_INPUT control=0x%02x attached=%d script_running=%u repl_running=%d",
                ctrl,
                ide_dbg_attach(),
                ide_dbg_script_running(),
                ide_dbg_repl_script_running());

        if (ctrl == IDE_CTRL_D && ide_dbg_is_script_running()) {
            ide_dbg_request_soft_reset();
            pr_dbg("[ide] USBDBG_TX_INPUT requested soft reset");
        } else if (ctrl == CHAR_CTRL_C && ide_dbg_repl_script_running()) {
            wait_mp_irq_handler_done();
            is_repl_intr = true;
#if MICROPY_KBD_EXCEPTION
            mp_thread_set_exception_main(MP_OBJ_FROM_PTR(&MP_STATE_VM(mp_kbd_exception)));
#endif
            pr_dbg("[ide] USBDBG_TX_INPUT raised keyboard interrupt");
        } else {
            mp_hal_stdin_push(&ctrl, 1);
            pr_dbg("[ide] USBDBG_TX_INPUT pushed control byte to stdin");
        }
        free(input);
        return;
    }

    pr_dbg("[ide] USBDBG_TX_INPUT first=0x%02x attached=%d idle=%d",
            (uint8_t)input[0],
            ide_dbg_attach(),
            ide_dbg_script_running() == 0 && !ide_dbg_repl_script_running());

    if (ide_dbg_repl_script_running() && input[0] == CHAR_CTRL_C) {
        wait_mp_irq_handler_done();
        is_repl_intr = true;
#if MICROPY_KBD_EXCEPTION
        mp_thread_set_exception_main(MP_OBJ_FROM_PTR(&MP_STATE_VM(mp_kbd_exception)));
#endif
        pr_dbg("[ide] USBDBG_TX_INPUT raised keyboard interrupt");
    } else if ((uint8_t)input[0] == IDE_CTRL_D && ide_dbg_is_script_running()) {
        ide_dbg_request_soft_reset();
        pr_dbg("[ide] USBDBG_TX_INPUT requested soft reset");
    } else {
        size_t input_len = state->data_length;
        mp_hal_stdin_push((const uint8_t*)input, input_len);
        pr_dbg("[ide] USBDBG_TX_INPUT pushed %zu bytes to stdin", input_len);
    }
    free(input);
}

static int file_sync(FILE *file) {
    if (file == NULL) {
        return -1;
    }
    if (fflush(file) != 0) {
        return -1;
    }
    return fsync(fileno(file));
}

static void svfile_release_chunk_buffer(void) {
    if (ide_dbg_sv_file.chunk_buffer != NULL) {
        free(ide_dbg_sv_file.chunk_buffer);
        ide_dbg_sv_file.chunk_buffer = NULL;
    }
}

static void cmd_verifyfile(void) {
    pr_verb("cmd: USBDBG_VERIFYFILE");
    uint32_t resp = USBDBG_SVFILE_VERIFY_ERR_NONE;

    if (ide_dbg_sv_file.file != NULL) {
        if (file_sync(ide_dbg_sv_file.file) != 0) {
            resp = USBDBG_SVFILE_ERR_WRITE_ERR;
            fclose(ide_dbg_sv_file.file);
            ide_dbg_sv_file.file = NULL;
            svfile_release_chunk_buffer();
            mp_hal_uart_tx(&resp, sizeof(resp));
            return;
        }
        fclose(ide_dbg_sv_file.file);
        ide_dbg_sv_file.file = NULL;
    }

    char filepath[USBDBG_MAX_PATH_LEN + 64];
    if (!resolve_filepath(ide_dbg_sv_file.info.name, filepath, sizeof(filepath))) {
        resp = USBDBG_SVFILE_VERIFY_NOT_OPEN;
        svfile_release_chunk_buffer();
        mp_hal_uart_tx(&resp, sizeof(resp));
        return;
    }

    FILE *f = fopen(filepath, "rb");
    if (f == NULL) {
        resp = USBDBG_SVFILE_VERIFY_NOT_OPEN;
        svfile_release_chunk_buffer();
        mp_hal_uart_tx(&resp, sizeof(resp));
        return;
    }

    unsigned char buffer[256];
    mbedtls_sha256_context sha256;
    mbedtls_sha256_init(&sha256);
    mbedtls_sha256_starts_ret(&sha256, 0);

    size_t nbytes;
    do {
        nbytes = fread(buffer, 1, sizeof(buffer), f);
        mbedtls_sha256_update_ret(&sha256, buffer, nbytes);
    } while (nbytes == sizeof(buffer));

    fclose(f);

    uint8_t sha256_result[32];
    mbedtls_sha256_finish_ret(&sha256, sha256_result);
    mbedtls_sha256_free(&sha256);
    if (memcmp(sha256_result, ide_dbg_sv_file.info.sha256, sizeof(sha256_result)) != 0) {
        resp = USBDBG_SVFILE_VERIFY_SHA2_ERR;
        pr_verb("file sha256 unmatched");
        print_sha256(sha256_result);
    }

    svfile_release_chunk_buffer();
    mp_hal_uart_tx(&resp, sizeof(resp));
}

static void cmd_createfile(ide_dbg_state_t* state) {
    pr_verb("cmd: USBDBG_CREATEFILE");
    hal_rvv_memset(&ide_dbg_sv_file.info, 0, sizeof(ide_dbg_sv_file.info));
    if (state->data_length != 0 && sizeof(ide_dbg_sv_file.info) != state->data_length) {
        ide_dbg_sv_file.errcode = USBDBG_SVFILE_ERR_PATH_ERR;
        pr_verb("Warning: CREATEFILE expect data length %lu, got %u",
                sizeof(ide_dbg_sv_file.info), state->data_length);
        discard_from_frame(state, state->data_length);
        return;
    }

    read_from_frame(state, &ide_dbg_sv_file.info, sizeof(ide_dbg_sv_file.info));
    pr_verb("create file: chunk_size(%d), name(%s)",
            ide_dbg_sv_file.info.chunk_size, ide_dbg_sv_file.info.name);
    print_sha256(ide_dbg_sv_file.info.sha256);

    close_active_sv_file();
    svfile_release_chunk_buffer();

    if (ide_dbg_sv_file.info.chunk_size <= 0 ||
        ide_dbg_sv_file.info.chunk_size > USBDBG_FILE_CHUNK_MAX) {
        ide_dbg_sv_file.errcode = USBDBG_SVFILE_ERR_CHUNK_ERR;
        return;
    }

    char filepath[USBDBG_MAX_PATH_LEN + 64];
    if (!resolve_filepath(ide_dbg_sv_file.info.name, filepath, sizeof(filepath))) {
        ide_dbg_sv_file.errcode = USBDBG_SVFILE_ERR_PATH_ERR;
        return;
    }

    ide_dbg_sv_file.file = fopen(filepath, "wb");
    if (ide_dbg_sv_file.file == NULL) {
        ide_dbg_sv_file.errcode = USBDBG_SVFILE_ERR_OPEN_ERR;
        return;
    }

    ide_dbg_sv_file.chunk_buffer = malloc(ide_dbg_sv_file.info.chunk_size);
    if (ide_dbg_sv_file.chunk_buffer == NULL) {
        fclose(ide_dbg_sv_file.file);
        ide_dbg_sv_file.file = NULL;
        ide_dbg_sv_file.errcode = USBDBG_SVFILE_ERR_CHUNK_ERR;
        return;
    }
    ide_dbg_sv_file.errcode = USBDBG_SVFILE_ERR_NONE;
}

static void cmd_frame_size(void) {
    #if PRINT_ALL
    pr_verb("cmd: USBDBG_FRAME_SIZE");
    #endif
    uint32_t resp[3] = { 0, 0, 0 };

    if (!mp_hal_uart_is_usb()) {
        mp_hal_uart_tx(&resp, sizeof(resp));
        return;
    }

    if (enable_pic && fb_from != FB_FROM_NONE) {
        static uint64_t last_frame_ticks_ms = 0;
        uint64_t curr_frame_ticks_ms = mp_hal_ticks_ms();
        if (curr_frame_ticks_ms - last_frame_ticks_ms >= 33) {
            last_frame_ticks_ms = curr_frame_ticks_ms;
            fb_from_current = fb_from;

            pthread_mutex_lock(&fb_mutex);
            if (FB_FROM_USER_SET == fb_from_current) {
                if (fb_data && fb_size) {
                    resp[0] = fb_width;
                    resp[1] = fb_height;
                    resp[2] = fb_size;
                }
            } else if (FB_FROM_VO_WRITEBACK == fb_from_current) {
                extern int ide_dbg_vo_wbc_dump_and_encode(
                    void** buffer, size_t* buffer_size,
                    uint32_t* image_widht, uint32_t* image_height);
                if (!fb_vo_wbc_data) {
                    uint32_t img_width = 0, img_height = 0;
                    if (0x00 != ide_dbg_vo_wbc_dump_and_encode(
                            &fb_vo_wbc_data, &fb_vo_wbc_data_size,
                            &img_width, &img_height)) {
                        fb_vo_wbc_data = NULL;
                        fb_vo_wbc_data_size = 0;
                        fb_vo_wbc_width = 0;
                        fb_vo_wbc_height = 0;
                    } else {
                        fb_vo_wbc_width = img_width;
                        fb_vo_wbc_height = img_height;
                    }
                }
                if (fb_vo_wbc_data && fb_vo_wbc_data_size) {
                    resp[0] = fb_vo_wbc_width;
                    resp[1] = fb_vo_wbc_height;
                    resp[2] = fb_vo_wbc_data_size;
                }
            }
            pthread_mutex_unlock(&fb_mutex);
        }
    }

    mp_hal_uart_tx(&resp, sizeof(resp));
}

static void cmd_frame_dump(void) {
    if (!mp_hal_uart_is_usb()) {
        return;
    }

    if (fb_from_current == FB_FROM_NONE)
        return;

    pthread_mutex_lock(&fb_mutex);
    if (FB_FROM_USER_SET == fb_from_current) {
        if (fb_data && fb_size) {
            mp_hal_uart_tx((void*)fb_data, fb_size);
            free((void*)fb_data);
            fb_data = NULL;
            fb_size = 0;
        }
    } else if (FB_FROM_VO_WRITEBACK == fb_from_current) {
        if (fb_vo_wbc_data && fb_vo_wbc_data_size) {
            mp_hal_uart_tx((void*)fb_vo_wbc_data, fb_vo_wbc_data_size);
            fb_vo_wbc_data = NULL;
            fb_vo_wbc_data_size = 0;
            fb_vo_wbc_width = 0;
            fb_vo_wbc_height = 0;
        }
    }
    pthread_mutex_unlock(&fb_mutex);
}

static size_t read_from_frame(ide_dbg_state_t* state, void* buf, size_t size) {
    uint8_t *dst = (uint8_t*)buf;
    size_t avail = 0;
    if (state->frame_offset < state->frame_length) {
        avail = state->frame_length - state->frame_offset;
    }
    size_t from_buf = (avail < size) ? avail : size;
    if (from_buf > 0) {
        hal_rvv_memcpy(dst, state->frame_data + state->frame_offset, from_buf);
        state->frame_offset += from_buf;
        dst += from_buf;
        size -= from_buf;
    }
    if (size > 0) {
        read_until(dst, size);
        from_buf += size;
    }
    return from_buf;
}

static bool read_path_nt(ide_dbg_state_t* state, char *out, size_t out_size) {
    if (out == NULL || out_size == 0) {
        return false;
    }

    const uint8_t *p = state->frame_data + state->frame_offset;
    size_t remain = 0;
    size_t idx = 0;
    int found_null = 0;
    bool truncated = false;

    if (state->frame_offset < state->frame_length) {
        remain = state->frame_length - state->frame_offset;
    }
    while (remain > 0) {
        if (*p == 0) {
            p++;
            remain--;
            found_null = 1;
            break;
        }
        if (idx < out_size - 1) {
            out[idx++] = (char)*p;
        } else {
            truncated = true;
        }
        p++;
        remain--;
    }
    state->frame_offset = state->frame_length - remain;

    if (!found_null) {
        uint8_t ch;
        while (true) {
            read_until(&ch, 1);
            if (ch == 0) {
                found_null = 1;
                break;
            }
            if (idx < out_size - 1) {
                out[idx++] = (char)ch;
            } else {
                truncated = true;
            }
        }
    }
    out[idx] = '\0';
    return found_null && !truncated && idx > 0 && strstr(out, "..") == NULL;
}

static uint32_t read_u32_from_frame(ide_dbg_state_t* state) {
    uint32_t val = 0;
    read_from_frame(state, &val, sizeof(val));
    return val;
}

static uint8_t stat_type_from_mode(mode_t mode) {
    if (S_ISDIR(mode)) return 1;
    return 0;
}

static void cmd_query_file_stat(ide_dbg_state_t* state) {
    uint32_t resp[4] = { USBDBG_SVFILE_ERR_FILE_NOT_FOUND, 0, 0, 0 };
    char path[USBDBG_MAX_PATH_LEN + 1];
    char resolved[USBDBG_MAX_PATH_LEN + 64];

    if (!read_path_nt(state, path, sizeof(path)) ||
        !resolve_filepath(path, resolved, sizeof(resolved))) {
        resp[0] = USBDBG_SVFILE_ERR_INVALID_PATH;
        mp_hal_uart_tx(resp, sizeof(resp));
        return;
    }

    struct stat st;
    if (stat(resolved, &st) == 0 && (S_ISREG(st.st_mode) || S_ISDIR(st.st_mode))) {
        resp[0] = 0;
        resp[1] = stat_type_from_mode(st.st_mode);
        resp[2] = S_ISREG(st.st_mode) ? (uint32_t)st.st_size : 0;
        resp[3] = (uint32_t)st.st_mtime;
    } else {
        resp[0] = file_errno_to_usbdbg(errno);
    }
    mp_hal_uart_tx(resp, sizeof(resp));
}

static uint32_t listdir_payload_size(DIR *dir, uint32_t *count) {
    struct dirent *ent;
    uint32_t bytes = 0;
    *count = 0;
    while ((ent = readdir(dir)) != NULL) {
        if (is_dot_or_dotdot(ent->d_name)) continue;
        size_t name_len = strlen(ent->d_name);
        if (name_len > 255) name_len = 255;
        bytes += 10 + (uint32_t)name_len;
        (*count)++;
    }
    rewinddir(dir);
    return bytes;
}

static void cmd_listdir(ide_dbg_state_t* state) {
    char path[USBDBG_MAX_PATH_LEN + 1];
    char filepath[USBDBG_MAX_PATH_LEN + 64];
    uint32_t header[3] = { 0, 0, 0 };

    if (!read_path_nt(state, path, sizeof(path)) ||
        !resolve_filepath(path, filepath, sizeof(filepath))) {
        header[0] = USBDBG_SVFILE_ERR_INVALID_PATH;
        mp_hal_uart_tx(header, sizeof(header));
        return;
    }

    const char *root_names[] = { "sdcard", "data", "udisk" };
    if (strcmp(path, "/") == 0) {
        for (size_t i = 0; i < sizeof(root_names) / sizeof(root_names[0]); i++) {
            char root_path[64];
            struct stat st;
            snprintf(root_path, sizeof(root_path), "/%s", root_names[i]);
            if (stat(root_path, &st) == 0 && S_ISDIR(st.st_mode)) {
                header[1] += 10 + (uint32_t)strlen(root_names[i]);
                header[2]++;
            }
        }
        mp_hal_uart_tx(header, sizeof(header));
        for (size_t i = 0; i < sizeof(root_names) / sizeof(root_names[0]); i++) {
            char root_path[64];
            struct stat st;
            snprintf(root_path, sizeof(root_path), "/%s", root_names[i]);
            if (stat(root_path, &st) == 0 && S_ISDIR(st.st_mode)) {
                uint8_t type = 1;
                uint8_t name_len = (uint8_t)strlen(root_names[i]);
                uint32_t size = 0;
                uint32_t mtime = (uint32_t)st.st_mtime;
                mp_hal_uart_tx(&type, 1);
                mp_hal_uart_tx(&name_len, 1);
                mp_hal_uart_tx(&size, 4);
                mp_hal_uart_tx(&mtime, 4);
                mp_hal_uart_tx(root_names[i], name_len);
            }
        }
        return;
    }

    DIR *dir = opendir(filepath);
    if (dir == NULL) {
        header[0] = file_errno_to_usbdbg(errno);
        mp_hal_uart_tx(header, sizeof(header));
        return;
    }

    header[1] = listdir_payload_size(dir, &header[2]);
    mp_hal_uart_tx(header, sizeof(header));

    struct dirent *ent;
    while ((ent = readdir(dir)) != NULL) {
        if (is_dot_or_dotdot(ent->d_name)) continue;

        char child[USBDBG_MAX_PATH_LEN + 512];
        struct stat st;
        snprintf(child, sizeof(child), "%s/%s", filepath, ent->d_name);
        if (stat(child, &st) != 0) {
            hal_rvv_memset(&st, 0, sizeof(st));
        }
        size_t name_len = strlen(ent->d_name);
        if (name_len > 255) name_len = 255;
        uint8_t type = stat_type_from_mode(st.st_mode);
        uint8_t nl = (uint8_t)name_len;
        uint32_t size = S_ISREG(st.st_mode) ? (uint32_t)st.st_size : 0;
        uint32_t mtime = (uint32_t)st.st_mtime;
        mp_hal_uart_tx(&type, 1);
        mp_hal_uart_tx(&nl, 1);
        mp_hal_uart_tx(&size, 4);
        mp_hal_uart_tx(&mtime, 4);
        mp_hal_uart_tx(ent->d_name, nl);
    }
    closedir(dir);
}

static void cmd_readfile(ide_dbg_state_t* state) {
    uint32_t offset = read_u32_from_frame(state);
    uint32_t req_size = read_u32_from_frame(state);
    uint32_t header[3] = { 0, 0, 0 };
    char path[USBDBG_MAX_PATH_LEN + 1];
    char filepath[USBDBG_MAX_PATH_LEN + 64];

    if (req_size > USBDBG_FILE_CHUNK_MAX) req_size = USBDBG_FILE_CHUNK_MAX;
    if (!read_path_nt(state, path, sizeof(path)) ||
        !resolve_filepath(path, filepath, sizeof(filepath))) {
        header[0] = USBDBG_SVFILE_ERR_INVALID_PATH;
        mp_hal_uart_tx(header, sizeof(header));
        return;
    }

    struct stat st;
    if (stat(filepath, &st) != 0) {
        header[0] = file_errno_to_usbdbg(errno);
        mp_hal_uart_tx(header, sizeof(header));
        return;
    }
    if (!S_ISREG(st.st_mode)) {
        header[0] = USBDBG_SVFILE_ERR_INVALID_PATH;
        mp_hal_uart_tx(header, sizeof(header));
        return;
    }

    FILE *f = fopen(filepath, "rb");
    if (f == NULL) {
        header[0] = file_errno_to_usbdbg(errno);
        mp_hal_uart_tx(header, sizeof(header));
        return;
    }

    header[2] = (uint32_t)st.st_size;
    if (fseek(f, (long)offset, SEEK_SET) != 0) {
        header[0] = file_errno_to_usbdbg(errno);
        fclose(f);
        mp_hal_uart_tx(header, sizeof(header));
        return;
    }

    header[1] = (uint32_t)fread(ide_dbg_file_read_buffer, 1, req_size, f);
    fclose(f);
    mp_hal_uart_tx(header, sizeof(header));
    if (header[1] > 0) {
        mp_hal_uart_tx(ide_dbg_file_read_buffer, header[1]);
    }
}

static void cmd_createfile_ack(ide_dbg_state_t* state) {
    cmd_createfile(state);
    uint32_t errcode = (ide_dbg_sv_file.errcode == USBDBG_SVFILE_ERR_NONE) ? 0 : ide_dbg_sv_file.errcode;
    mp_hal_uart_tx(&errcode, sizeof(errcode));
}

static void cmd_writefile(ide_dbg_state_t* state) {
    pr_verb("cmd: USBDBG_WRITEFILE %u bytes", state->data_length);
    if ((ide_dbg_sv_file.file == NULL) ||
        (ide_dbg_sv_file.chunk_buffer == NULL) ||
        (ide_dbg_sv_file.info.chunk_size < state->data_length)) {
        ide_dbg_sv_file.errcode = USBDBG_SVFILE_ERR_WRITE_ERR;
        discard_from_frame(state, state->data_length);
        return;
    }

    read_from_frame(state, ide_dbg_sv_file.chunk_buffer, state->data_length);
    if (ide_dbg_sv_file.file == NULL) {
        ide_dbg_sv_file.errcode = USBDBG_SVFILE_ERR_WRITE_ERR;
    } else if (fwrite(ide_dbg_sv_file.chunk_buffer, 1, state->data_length, ide_dbg_sv_file.file) == state->data_length) {
        ide_dbg_sv_file.errcode = USBDBG_SVFILE_ERR_NONE;
    } else {
        fclose(ide_dbg_sv_file.file);
        ide_dbg_sv_file.file = NULL;
        ide_dbg_sv_file.errcode = USBDBG_SVFILE_ERR_WRITE_ERR;
    }
}

static void cmd_writefile_ack(ide_dbg_state_t* state) {
    uint32_t payload_size = state->data_length;
    uint32_t write_size = payload_size;
    if (write_size == 0) {
        write_size = read_u32_from_frame(state);
        payload_size = write_size;
    }
    if (write_size > USBDBG_FILE_CHUNK_MAX) {
        uint32_t errcode = USBDBG_SVFILE_ERR_CHUNK_ERR;
        discard_from_frame(state, payload_size);
        ide_dbg_sv_file.errcode = errcode;
        mp_hal_uart_tx(&errcode, sizeof(errcode));
        return;
    }

    if (ide_dbg_sv_file.chunk_buffer == NULL ||
        ide_dbg_sv_file.info.chunk_size <= 0 ||
        write_size > (uint32_t)ide_dbg_sv_file.info.chunk_size) {
        uint32_t errcode = USBDBG_SVFILE_ERR_CHUNK_ERR;
        discard_from_frame(state, payload_size);
        ide_dbg_sv_file.errcode = errcode;
        mp_hal_uart_tx(&errcode, sizeof(errcode));
        return;
    }
    read_from_frame(state, ide_dbg_sv_file.chunk_buffer, write_size);

    if (ide_dbg_sv_file.file == NULL && ide_dbg_sv_file.info.name[0] != '\0') {
        char filepath[USBDBG_MAX_PATH_LEN + 64];
        if (resolve_filepath(ide_dbg_sv_file.info.name, filepath, sizeof(filepath))) {
            ide_dbg_sv_file.file = fopen(filepath, "ab");
        }
    }

    uint32_t errcode = 0;
    if (ide_dbg_sv_file.file == NULL) {
        errcode = USBDBG_SVFILE_ERR_WRITE_ERR;
    } else if (fwrite(ide_dbg_sv_file.chunk_buffer, 1, write_size, ide_dbg_sv_file.file) != write_size) {
        errcode = USBDBG_SVFILE_ERR_WRITE_ERR;
    }
    ide_dbg_sv_file.errcode = errcode;
    mp_hal_uart_tx(&errcode, sizeof(errcode));
}

static void cmd_deletefile(ide_dbg_state_t* state) {
    uint32_t errcode;
    char path[USBDBG_MAX_PATH_LEN + 1];
    char filepath[USBDBG_MAX_PATH_LEN + 64];

    if (!read_path_nt(state, path, sizeof(path))) {
        errcode = USBDBG_SVFILE_ERR_INVALID_PATH;
    } else if (!resolve_filepath(path, filepath, sizeof(filepath))) {
        errcode = USBDBG_SVFILE_ERR_INVALID_PATH;
    } else {
        close_active_sv_file();
        if (unlink(filepath) == 0) {
            errcode = USBDBG_SVFILE_ERR_NONE;
        } else {
            errcode = file_errno_to_usbdbg(errno);
        }
    }
    mp_hal_uart_tx(&errcode, sizeof(errcode));
}

static void cmd_renamefile(ide_dbg_state_t* state) {
    uint32_t errcode;
    char old_path[USBDBG_MAX_PATH_LEN + 1];
    char new_path[USBDBG_MAX_PATH_LEN + 1];
    char old_resolved[USBDBG_MAX_PATH_LEN + 64];
    char new_resolved[USBDBG_MAX_PATH_LEN + 64];

    if (!read_path_nt(state, old_path, sizeof(old_path)) ||
        !read_path_nt(state, new_path, sizeof(new_path))) {
        errcode = USBDBG_SVFILE_ERR_INVALID_PATH;
    } else if (!resolve_filepath(old_path, old_resolved, sizeof(old_resolved)) ||
               !resolve_filepath(new_path, new_resolved, sizeof(new_resolved))) {
        errcode = USBDBG_SVFILE_ERR_INVALID_PATH;
    } else {
        close_active_sv_file();
        if (rename(old_resolved, new_resolved) == 0) {
            errcode = 0;
        } else {
            pr_dbg("RENAME_FILE %s -> %s errno=%d (%s)", old_resolved, new_resolved, errno, strerror(errno));
            errcode = file_errno_to_usbdbg(errno);
        }
    }
    mp_hal_uart_tx(&errcode, sizeof(errcode));
}

static void cmd_mkdir(ide_dbg_state_t* state) {
    uint32_t errcode;
    char path[USBDBG_MAX_PATH_LEN + 1];
    char filepath[USBDBG_MAX_PATH_LEN + 64];

    if (!read_path_nt(state, path, sizeof(path))) {
        errcode = USBDBG_SVFILE_ERR_INVALID_PATH;
    } else if (!resolve_filepath(path, filepath, sizeof(filepath))) {
        errcode = USBDBG_SVFILE_ERR_INVALID_PATH;
    } else {
        if (mkdir(filepath, 0777) == 0) {
            errcode = USBDBG_SVFILE_ERR_NONE;
        } else {
            errcode = (errno == EEXIST) ? USBDBG_SVFILE_ERR_FILE_EXISTS
                    : (errno == ENOENT) ? USBDBG_SVFILE_ERR_INVALID_PATH
                    : file_errno_to_usbdbg(errno);
        }
    }
    mp_hal_uart_tx(&errcode, sizeof(errcode));
}

static void cmd_rmdir(ide_dbg_state_t* state) {
    uint32_t errcode;
    char path[USBDBG_MAX_PATH_LEN + 1];
    char filepath[USBDBG_MAX_PATH_LEN + 64];

    if (!read_path_nt(state, path, sizeof(path))) {
        errcode = USBDBG_SVFILE_ERR_INVALID_PATH;
    } else if (!resolve_filepath(path, filepath, sizeof(filepath))) {
        errcode = USBDBG_SVFILE_ERR_INVALID_PATH;
    } else {
        close_active_sv_file();
        if (rmdir(filepath) == 0) {
            errcode = USBDBG_SVFILE_ERR_NONE;
        } else {
            errcode = file_errno_to_usbdbg(errno);
        }
    }
    mp_hal_uart_tx(&errcode, sizeof(errcode));
}

static void file_exec_error(const char *fmt, ...) {
    char buf[256];
    va_list va;
    va_start(va, fmt);
    int n = vsnprintf(buf, sizeof(buf), fmt, va);
    va_end(va);
    if (n > 0) {
        size_t len = (size_t)n;
        if (len >= sizeof(buf)) {
            len = sizeof(buf) - 1;
        }
        ide_dbg_stdout_tx(buf, len);
    }
}

static void cmd_file_exec(ide_dbg_state_t* state) {
    char path[USBDBG_MAX_PATH_LEN + 1];
    char filepath[USBDBG_MAX_PATH_LEN + 64];

    if (!read_path_nt(state, path, sizeof(path)) ||
        !resolve_filepath(path, filepath, sizeof(filepath)))
        return;

    if (ide_dbg_is_script_running()) {
        file_exec_error("FILE_EXEC ERROR: script already running\n");
        return;
    }

    exec_payload = strdup(filepath);
    if (exec_payload == NULL) {
        file_exec_error("FILE_EXEC ERROR: out of memory\n");
        return;
    }
    ide_dbg_queue_script(exec_payload, EXEC_FILE);
}

static bool ide_dbg_cmd_known(uint8_t cmd) {
    switch ((enum usbdbg_cmd)cmd) {
        case USBDBG_NONE:
        case USBDBG_FW_VERSION:
        case USBDBG_FRAME_SIZE:
        case USBDBG_FRAME_DUMP:
        case USBDBG_ARCH_STR:
        case USBDBG_SCRIPT_EXEC:
        case USBDBG_SCRIPT_STOP:
        case USBDBG_SCRIPT_SAVE:
        case USBDBG_SCRIPT_RUNNING:
        case USBDBG_TEMPLATE_SAVE:
        case USBDBG_DESCRIPTOR_SAVE:
        case USBDBG_ATTR_READ:
        case USBDBG_ATTR_WRITE:
        case USBDBG_SYS_RESET:
        case USBDBG_FB_ENABLE:
        case USBDBG_QUERY_STATUS:
        case USBDBG_TX_BUF_LEN:
        case USBDBG_TX_BUF:
        case USBDBG_SENSOR_ID:
        case USBDBG_FW_VERSION_FULL:
        case USBDBG_TX_INPUT:
        case USBDBG_SET_TIME:
        case USBDBG_CREATEFILE:
        case USBDBG_WRITEFILE:
        case USBDBG_QUERY_FILE_STAT:
        case USBDBG_VERIFYFILE:
        case USBDBG_LIST_DIR:
        case USBDBG_READ_FILE:
        case USBDBG_DELETE_FILE:
        case USBDBG_RENAME_FILE:
        case USBDBG_MKDIR:
        case USBDBG_RMDIR:
        case USBDBG_FILE_EXEC:
        case USBDBG_QUERY_FILE_STAT2:
        case USBDBG_CREATEFILE2:
        case USBDBG_WRITEFILE2:
        case USBDBG_SCRIPT_STATUS:
        case USBDBG_CAPABILITIES:
        case USBDBG_VTOUCH_EVENT:
        case USBDBG_VTOUCH_STATUS:
            return true;
        default:
            return false;
    }
}

static bool ide_dbg_frame_header_valid(const uint8_t header[6]) {
    if (header[0] != 0x30 || !ide_dbg_cmd_known(header[1])) {
        return false;
    }

    uint32_t data_length = 0;
    hal_rvv_memcpy(&data_length, header + 2, sizeof(data_length));
    return data_length <= IDE_DBG_MAX_FRAME_PAYLOAD;
}

typedef struct {
    uint8_t pending[6];
    size_t pending_len;
} ide_dbg_rx_router_t;

static ide_dbg_status_t ide_dbg_update(ide_dbg_state_t* state, const uint8_t* data, size_t length);

static void ide_dbg_route_repl(const uint8_t *data, size_t size) {
    if (size == 0) {
        return;
    }

    pr_verb("[uart] repl %zu bytes", size);
    print_raw(data, size);
    mp_hal_stdin_push(data, size);
}

static void ide_dbg_route_pending_flush(ide_dbg_rx_router_t *router) {
    if (router->pending_len == 0) {
        return;
    }
    ide_dbg_route_repl(router->pending, router->pending_len);
    router->pending_len = 0;
}

static void ide_dbg_route_protocol_start(ide_dbg_rx_router_t *router, ide_dbg_state_t *state,
                                         const uint8_t *rest, size_t rest_len) {
    size_t frame_len = router->pending_len + rest_len;
    uint8_t *frame = malloc(frame_len);
    if (frame == NULL) {
        ide_dbg_route_repl(router->pending, router->pending_len);
        ide_dbg_route_repl(rest, rest_len);
        router->pending_len = 0;
        return;
    }

    hal_rvv_memcpy(frame, router->pending, router->pending_len);
    if (rest_len > 0) {
        hal_rvv_memcpy(frame + router->pending_len, rest, rest_len);
    }
    router->pending_len = 0;
    ide_dbg_set_attached(true);
    ide_dbg_update(state, frame, frame_len);
    free(frame);
}

static void ide_dbg_route_bytes(ide_dbg_rx_router_t *router, ide_dbg_state_t *state,
                                const uint8_t *data, size_t size) {
    if (ide_dbg_attach()) {
        ide_dbg_route_pending_flush(router);
        ide_dbg_update(state, data, size);
        return;
    }

    for (size_t i = 0; i < size; i++) {
        uint8_t byte = data[i];

        if (router->pending_len == 0 && byte != 0x30) {
            ide_dbg_route_repl(&byte, 1);
            continue;
        }

        router->pending[router->pending_len++] = byte;
        if (router->pending_len < sizeof(router->pending)) {
            continue;
        }

        if (ide_dbg_frame_header_valid(router->pending)) {
            ide_dbg_route_protocol_start(router, state, data + i + 1, size - i - 1);
            return;
        }

        ide_dbg_route_repl(router->pending, router->pending_len);
        router->pending_len = 0;
    }
}

static void cmd_capabilities(ide_dbg_state_t* state) {
    (void)state;
    uint32_t resp[2] = {
        (uint32_t)USBDBG_CAP_PROTOCOL_VERSION,
        (uint32_t)(USBDBG_CAP_LIST_DIR | USBDBG_CAP_READ_FILE |
                    USBDBG_CAP_WRITE_FILE | USBDBG_CAP_DELETE_FILE |
                    USBDBG_CAP_RENAME_FILE | USBDBG_CAP_MKDIR |
                    USBDBG_CAP_RMDIR | USBDBG_CAP_FILE_EXEC |
                    USBDBG_CAP_VIRTUAL_TOUCH | USBDBG_CAP_REPL_INPUT)
    };
    mp_hal_uart_tx(resp, sizeof(resp));
}

static void cmd_vtouch_status(ide_dbg_state_t* state) {
    (void)state;
    struct ide_dbg_vtouch_info info;

    ide_dbg_vtouch_status(&info);
    mp_hal_uart_tx(&info, sizeof(info));
}

static void cmd_vtouch_event(ide_dbg_state_t* state) {
    struct ide_dbg_vtouch_event event = {0};
    uint32_t payload_len = state->data_length;

    if (payload_len != sizeof(event)) {
        if (payload_len > 0) {
            discard_from_frame(state, payload_len);
        }
        return;
    }

    read_from_frame(state, &event, sizeof(event));
    (void)ide_dbg_vtouch_enqueue(&event);
}

static bool read_fb_enable_legacy_inline(ide_dbg_state_t* state, uint8_t *enable) {
    if (state->frame_offset + 2 > state->frame_length) {
        return false;
    }
    uint8_t lo = state->frame_data[state->frame_offset];
    uint8_t hi = state->frame_data[state->frame_offset + 1];
    if (hi != 0 || (lo != 0 && lo != 1)) {
        return false;
    }
    *enable = lo;
    state->frame_offset += 2;
    return true;
}

static void cmd_fb_enable(ide_dbg_state_t* state) {
    uint8_t enable = 0;
    if (state->data_length > 0) {
        read_from_frame(state, &enable, 1);
        if (state->data_length > 1) {
            discard_from_frame(state, state->data_length - 1);
        }
        enable_pic = enable;
    } else if (read_fb_enable_legacy_inline(state, &enable)) {
        enable_pic = enable;
    }
    if (!enable_pic) {
        ide_dbg_clear_fb();
    }
    pr_verb("cmd: USBDBG_FB_ENABLE, enable(%u)", enable_pic);
}

///////////////////////////////////////////////////////////////////////////////
// IDE protocol parser ////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

static ide_dbg_status_t ide_dbg_update(ide_dbg_state_t* state, const uint8_t* data, size_t length) {
    pr_dbg("update: len=%zu script_running=%u", length, ide_dbg_script_running());
    print_raw(data, length > 32 ? 32 : length);
    for (size_t i = 0; i < length;) {
        switch (state->state) {
            case FRAME_HEAD:
                if (data[i] == 0x30) {
                    state->state = FRAME_CMD;
                } else {
                    pr_dbg("FRAME_HEAD: skip byte 0x%02X at pos %zu", data[i], i);
                }
                i += 1;
                break;
            case FRAME_CMD:
                state->cmd = data[i];
                state->state = FRAME_DATA_LENGTH;
                pr_dbg("FRAME_CMD: cmd=0x%02X", state->cmd);
                i += 1;
                break;
            case FRAME_DATA_LENGTH:
                // recv 4 bytes
                state->recv_lack = 4;
                state->state = FRAME_RECV;
                state->recv_next = FRAME_DISPATCH;
                state->recv_data = &state->data_length;
                break;
            case FRAME_DISPATCH:
                state->frame_data = data;
                state->frame_length = length;
                state->frame_offset = i + 1;
                pr_dbg("DISPATCH cmd=0x%02X data_length=%u script_running=%u",
                       state->cmd, state->data_length, ide_dbg_script_running());
                switch (state->cmd) {
                    case USBDBG_NONE:
                        break;
                    case USBDBG_QUERY_STATUS: {
                        pr_dbg("cmd: USBDBG_QUERY_STATUS -> 0xFFEEBBAA");
                        uint32_t resp = 0xFFEEBBAA;
                        mp_hal_uart_tx(&resp, sizeof(resp));
                        break;
                    }
                    case USBDBG_FW_VERSION: {
                        pr_verb("cmd: USBDBG_FW_VERSION");
                        uint32_t resp[3] = {
                            FIRMWARE_VERSION_MAJOR,
                            FIRMWARE_VERSION_MINOR,
                            FIRMWARE_VERSION_MICRO
                        };
                        mp_hal_uart_tx(&resp, sizeof(resp));
                        break;
                    }
                    case USBDBG_FW_VERSION_FULL: {
                        pr_verb("cmd: USBDBG_FW_VERSION_FULL");
                        // Send git version string like "v1.0-45-g<full-hash>"
                        char ver_buf[128] = {0};
                        snprintf(ver_buf, sizeof(ver_buf), "%s-%s", MICROPY_HW_MCU_NAME, CANMV_VER_FULL);
                        mp_hal_uart_tx(ver_buf, sizeof(ver_buf));
                        break;
                    }
                    case USBDBG_ARCH_STR:
                        cmd_arch_str(state);
                        break;
                    case USBDBG_SCRIPT_EXEC:
                        cmd_script_exec(state);
                        break;
                    case USBDBG_SCRIPT_STOP: {
                        // TODO
                        pr_verb("cmd: USBDBG_SCRIPT_STOP");
                        if (ide_dbg_is_script_running()) {
                            ide_dbg_request_soft_reset();
                        }
                        break;
                    }
                    case USBDBG_SCRIPT_SAVE: {
                        // TODO
                        pr_verb("cmd: USBDBG_SCRIPT_SAVE");
                        break;
                    }
                    case USBDBG_TX_INPUT:
                        pr_dbg("[ide] dispatch USBDBG_TX_INPUT");
                        cmd_tx_input(state);
                        break;
                    case USBDBG_QUERY_FILE_STAT: {
                        pr_verb("cmd: USBDBG_QUERY_FILE_STAT");
                        mp_hal_uart_tx(&ide_dbg_sv_file.errcode, sizeof(ide_dbg_sv_file.errcode));
                        break;
                    }
                    case USBDBG_QUERY_FILE_STAT2:
                        cmd_query_file_stat(state);
                        break;
                    case USBDBG_WRITEFILE:
                        cmd_writefile(state);
                        break;
                    case USBDBG_WRITEFILE2:
                        cmd_writefile_ack(state);
                        break;
                    case USBDBG_VERIFYFILE:
                        cmd_verifyfile();
                        break;
                    case USBDBG_CREATEFILE:
                        cmd_createfile(state);
                        break;
                    case USBDBG_CREATEFILE2:
                        cmd_createfile_ack(state);
                        break;
                    case USBDBG_LIST_DIR:
                        cmd_listdir(state);
                        break;
                    case USBDBG_READ_FILE:
                        cmd_readfile(state);
                        break;
                    case USBDBG_DELETE_FILE:
                        cmd_deletefile(state);
                        break;
                    case USBDBG_RENAME_FILE:
                        cmd_renamefile(state);
                        break;
                    case USBDBG_MKDIR:
                        cmd_mkdir(state);
                        break;
                    case USBDBG_RMDIR:
                        cmd_rmdir(state);
                        break;
                    case USBDBG_FILE_EXEC:
                        cmd_file_exec(state);
                        break;
                    case USBDBG_CAPABILITIES:
                        cmd_capabilities(state);
                        break;
                    case USBDBG_VTOUCH_STATUS:
                        cmd_vtouch_status(state);
                        break;
                    case USBDBG_VTOUCH_EVENT:
                        cmd_vtouch_event(state);
                        break;
                    case USBDBG_SCRIPT_RUNNING: {
                        uint32_t running = ide_dbg_script_running();
                        pr_dbg("cmd: USBDBG_SCRIPT_RUNNING -> %u", running);
                        mp_hal_uart_tx(&running, sizeof(running));
                        break;
                    }
                    case USBDBG_SCRIPT_STATUS: {
                        uint32_t running = ide_dbg_is_script_running() ? 1 : 0;
                        pr_dbg("cmd: USBDBG_SCRIPT_STATUS -> %u", running);
                        mp_hal_uart_tx(&running, sizeof(running));
                        break;
                    }
                    case USBDBG_TX_BUF_LEN: {
                        pthread_mutex_lock(&tx_buf_mutex);
                        uint32_t len = tx_buf_readable();
                        pr_dbg("cmd: USBDBG_TX_BUF_LEN -> %u", len);
                        mp_hal_uart_tx((void*)&len, sizeof(len));
                        pthread_mutex_unlock(&tx_buf_mutex);
                        break;
                    }
                    case USBDBG_TX_BUF: {
                        pthread_mutex_lock(&tx_buf_mutex);
                        uint32_t len = tx_buf_readable();
                        if (len > state->data_length) {
                            len = state->data_length;
                        }
                        pr_dbg("cmd: USBDBG_TX_BUF drain %u (requested %u)", len, state->data_length);
                        tx_buf_drain(len);
                        pthread_mutex_unlock(&tx_buf_mutex);
                        break;
                    }
                    case USBDBG_FRAME_SIZE:
                        pr_dbg("cmd: USBDBG_FRAME_SIZE");
                        cmd_frame_size();
                        break;
                    case USBDBG_FRAME_DUMP:
                        pr_dbg("cmd: USBDBG_FRAME_DUMP");
                        cmd_frame_dump();
                        break;
                    case USBDBG_SYS_RESET: {
                        // TODO: reset serialport to REPL mode
                        pr_verb("cmd: USBDBG_SYS_RESET");
                        ide_dbg_clear_fb();
                        if (ide_dbg_is_script_running()) {
                            ide_dbg_set_disconnect(true);
                            mp_thread_set_exception_main(MP_OBJ_FROM_PTR(&ide_exception));
                        } else {
                            ide_dbg_interrupt();
                        }
                        break;
                    }
                    case USBDBG_FB_ENABLE: {
                        cmd_fb_enable(state);
                        break;
                    }
                    default:
                        // unknown command
                        pr_verb("unknown command %02x", state->cmd);
                        break;
                }
                state->state = FRAME_HEAD;
                if (state->frame_offset > i + 1) {
                    i = state->frame_offset;
                } else {
                    i += 1;
                }
                break;
            case FRAME_RECV: {
                size_t avail = length - i;
                if (avail >= state->recv_lack) {
                    hal_rvv_memcpy(state->recv_data, data + i, state->recv_lack);
                    state->state = state->recv_next;
                    state->recv_data = NULL;
                    /* Advance by recv_lack - 1: the next state (FRAME_DISPATCH)
                       will do i += 1, so total advancement = recv_lack. */
                    i += state->recv_lack - 1;
                    state->recv_lack = 0;
                } else {
                    hal_rvv_memcpy(state->recv_data, data + i, avail);
                    state->recv_data = (uint8_t*)state->recv_data + avail;
                    state->recv_lack -= avail;
                    i = length;
                }
                break;
            }
            default:
                state->state = FRAME_HEAD;
                break;
        }
    }
    return IDE_DBG_STATUS_OK;
}

///////////////////////////////////////////////////////////////////////////////
// IDE task & init ////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

void ide_before_python_run(int input_kind, mp_uint_t exec_flags)
{
    // mp_hal_stdio_mode_orig();
    ide_dbg_set_repl_script_running(true);
}

void ide_afer_python_run(int input_kind, mp_uint_t exec_flags, void *ret_val, int ret)
{
    ide_dbg_set_repl_script_running(false);
}

static void* ide_dbg_task(void* args) {
    ide_dbg_state_t state;
    ide_dbg_rx_router_t router = {0};
    state.state = FRAME_HEAD;
    static uint8_t read_buf[512];
    pthread_setcanceltype(PTHREAD_CANCEL_ASYNCHRONOUS, NULL);
    struct sched_param param;
    param.sched_priority = 20;
    pthread_setschedparam(pthread_self(), SCHED_FIFO, &param);
    while (1) {
        drv_uart_inst_t *inst = mp_hal_uart_get_instance();
        if (inst == NULL) {
            usleep(100000);
            continue;
        }

        // Poll UART for data (200ms timeout)
        int result = drv_uart_poll(inst, 200);
        if (result == 0) {
            ide_dbg_route_pending_flush(&router);
            continue;
        } else if (result < 0) {
            // DTR de-asserted or error
            pr_verb("[uart] poll error %d", result);
            if (ide_dbg_attach()) {
                static struct timeval tval_last = {};
                struct timeval tval;
                struct timeval tval_sub;
                gettimeofday(&tval, NULL);
                timersub(&tval, &tval_last, &tval_sub);
                if (tval_sub.tv_sec >= 1) {
                    if (ide_dbg_is_script_running()) {
                        ide_dbg_set_disconnect(true);
                        mp_thread_set_exception_main(MP_OBJ_FROM_PTR(&ide_exception));
                    } else {
                        ide_dbg_interrupt();
                    }
                    tval_last = tval;
                }
            }
            usleep(100000);
            continue;
        }
        ssize_t size = drv_uart_read(inst, read_buf, sizeof(read_buf));
        if (size == 0) {
            continue;
        } else if (size < 0) {
            pr_err("[uart] read error");
        } else {
            ide_dbg_route_bytes(&router, &state, read_buf, size);
        }
    }
    return NULL;
}

void ide_dbg_start(void) {
    pr_info("IDE debugger built %s %s", __DATE__, __TIME__);

    canmv_misc_dev_ioctl(MISC_DEV_CMD_GET_MEMORY_SIZE, &sys_mem_total_size);

    if (mp_hal_uart_get_instance() == NULL) {
        pr_err("ide_dbg_init: UART not initialized");
        return;
    }

    sem_init(&script_sem, 0, 0);
    pthread_mutex_init(&fb_mutex, NULL);
    ide_exception_str.data = (const byte*)"IDE interrupt";
    ide_exception_str.len  = 13;
    ide_exception_str.base.type = &mp_type_str;
    ide_exception_str.hash = qstr_compute_hash(ide_exception_str.data, ide_exception_str.len);
    ide_exception_str_tuple = (mp_obj_tuple_t*)malloc(sizeof(mp_obj_tuple_t)+sizeof(mp_obj_t)*1);
    if(ide_exception_str_tuple==NULL)
        return;
    ide_exception_str_tuple->base.type = &mp_type_tuple;
    ide_exception_str_tuple->len = 1;
    ide_exception_str_tuple->items[0] = MP_OBJ_FROM_PTR(&ide_exception_str);
    ide_exception.base.type = &mp_type_Exception;
    ide_exception.traceback_alloc = 0;
    ide_exception.traceback_len = 0;
    ide_exception.traceback_data = NULL;
    ide_exception.args = ide_exception_str_tuple;
    pthread_create(&ide_dbg_task_p, NULL, ide_dbg_task, NULL);
}

#else
#endif
