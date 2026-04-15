/*
 * This file is part of the OpenMV project.
 * Copyright (c) 2013/2014 Ibrahim Abdelkader <i.abdalkader@gmail.com>
 * This work is licensed under the MIT license, see the file LICENSE for details.
 *
 * IDE debug protocol handler.
 */

/* System headers */
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

/* MicroPython core */
#include "mpconfig.h"
#include "mpstate.h"
#include "mpthread.h"
#include "obj.h"
#include "objstr.h"
#include "readline.h"
#include "py/runtime.h"

/* Project / driver headers */
#include <sha256.h>
#include "ide_dbg.h"
#include "version.h"
#include "canmv_drivers.h"
#include "drv_uart.h"
#include "hal_rvv_ops.h"

#if CONFIG_CANMV_IDE_SUPPORT

///////////////////////////////////////////////////////////////////////////////
// Debug logging macros ///////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

#define COLOR_NONE "\033[0m"
#define RED "\033[1;31;40m"
#define BLUE "\033[1;34;40m"
#define GREEN "\033[1;32;40m"
#define YELLOW "\033[1;33;40m"

#define pr_verb(fmt,...)
#define pr_info(fmt,...) fprintf(stderr,GREEN fmt "\n"COLOR_NONE,##__VA_ARGS__)
#define pr_warn(fmt,...) fprintf(stderr,YELLOW fmt "\n"COLOR_NONE,##__VA_ARGS__)
#define pr_err(fmt,...) fprintf(stderr,RED fmt " at line %d\n"COLOR_NONE,##__VA_ARGS__, __LINE__)

#define PRINT_ALL 0

///////////////////////////////////////////////////////////////////////////////
// Extern declarations & state ////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

pthread_t ide_dbg_task_p;
static struct ide_dbg_svfil_t ide_dbg_sv_file;
static mp_obj_exception_t ide_exception; //IDE interrupt
static mp_obj_str_t ide_exception_str;
static mp_obj_tuple_t* ide_exception_str_tuple = NULL;

extern drv_uart_inst_t *mp_hal_uart_get_instance(void);
extern int mp_hal_uart_tx(const void* buffer, size_t size);
extern void mp_hal_stdin_push(const uint8_t* data, size_t len);

static uint64_t sys_mem_total_size = 0;

extern void system_set_exiting_flag(bool exiting);
extern bool in_mp_irq_handler(void);

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

bool repl_script_running = false;

///////////////////////////////////////////////////////////////////////////////
// IDE state management ///////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

static uint32_t ide_script_running = 0;

void mpy_start_script(char* filepath);
void mpy_stop_script();
static char *script_string = NULL;
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

static inline void ide_dbg_set_script_running(uint32_t running) {
    __atomic_store_n(&ide_script_running, running, __ATOMIC_RELEASE);
}

static inline bool ide_dbg_is_attached(void) {
    return __atomic_load_n(&ide_attached, __ATOMIC_ACQUIRE);
}

static inline void ide_dbg_set_attached(bool attached) {
    __atomic_store_n(&ide_attached, attached, __ATOMIC_RELEASE);
}

static inline bool ide_dbg_should_disconnect(void) {
    return __atomic_load_n(&ide_disconnect, __ATOMIC_ACQUIRE);
}

static inline void ide_dbg_set_disconnect(bool disconnect) {
    __atomic_store_n(&ide_disconnect, disconnect, __ATOMIC_RELEASE);
}

char* ide_dbg_get_script() {
    sem_wait(&script_sem);
    return ide_dbg_is_attached() ? script_string : NULL;
}

bool ide_dbg_attach(void) {
    return ide_dbg_is_attached();
}

void ide_dbg_on_script_start(void) {
    ide_dbg_set_script_running(1);
    ide_dbg_set_repl_script_running(true);
}

void ide_dbg_on_script_end(void) {
    if (script_string) {
        free(script_string);
        script_string = NULL;
    }
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
    fb_from = FB_FROM_NONE;

    ide_dbg_set_repl_script_running(false);
}

void interrupt_repl(void) {
    wait_mp_irq_handler_done();
    const uint8_t ctrl_cd[2] = { CHAR_CTRL_C, CHAR_CTRL_D };
    mp_hal_stdin_push(&ctrl_cd[0], 1);
    mp_hal_stdin_push(&ctrl_cd[1], 1);
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

static pthread_mutex_t fb_mutex;

// FIXME: reuse buf
void ide_set_fb(const void* data, uint32_t size, uint32_t width, uint32_t height) {
    pthread_mutex_lock(&fb_mutex);
    fb_from = FB_FROM_USER_SET;
    if (fb_data) {
        printf("Warning: new framebuffer set before the old one is consumed\n");
        pthread_mutex_unlock(&fb_mutex);
        return;
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
    if (strlen(name) == 0 || strstr(name, ".."))
        return false;
    if (name[0] == '/')
        snprintf(out, out_size, "%s", name);
    else
        snprintf(out, out_size, "/sdcard/%s", name);
    return true;
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
    if (ide_dbg_script_running() != 0)
        mp_thread_set_exception_main(MP_OBJ_FROM_PTR(&ide_exception));
    usleep(1000);
    if (ide_dbg_script_running() != 0)
        return;
    script_string = malloc(state->data_length + 1);
    read_until(script_string, state->data_length);
    script_string[state->data_length] = '\0';
    ide_dbg_set_script_running(1);
    sem_post(&script_sem);
}

static void cmd_writefile(ide_dbg_state_t* state) {
    pr_verb("cmd: USBDBG_WRITEFILE %u bytes", state->data_length);
    if ((ide_dbg_sv_file.file == NULL) ||
        (ide_dbg_sv_file.chunk_buffer == NULL) ||
        (ide_dbg_sv_file.info.chunk_size < state->data_length)) {
        ide_dbg_sv_file.errcode = USBDBG_SVFILE_ERR_WRITE_ERR;
        return;
    }
    read_until(ide_dbg_sv_file.chunk_buffer, state->data_length);
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

static void cmd_verifyfile(void) {
    pr_verb("cmd: USBDBG_VERIFYFILE");
    uint32_t resp = USBDBG_SVFILE_VERIFY_ERR_NONE;

    if (ide_dbg_sv_file.file != NULL) {
        fclose(ide_dbg_sv_file.file);
        ide_dbg_sv_file.file = NULL;
    }

    char filepath[120];
    if (!resolve_filepath(ide_dbg_sv_file.info.name, filepath, sizeof(filepath))) {
        resp = USBDBG_SVFILE_VERIFY_NOT_OPEN;
        mp_hal_uart_tx(&resp, sizeof(resp));
        return;
    }

    FILE *f = fopen(filepath, "r");
    if (f == NULL) {
        resp = USBDBG_SVFILE_VERIFY_NOT_OPEN;
        mp_hal_uart_tx(&resp, sizeof(resp));
        return;
    }

    unsigned char buffer[256];
    CRYAL_SHA256_CTX sha256;
    sha256_init(&sha256);

    size_t nbytes;
    do {
        nbytes = fread(buffer, 1, sizeof(buffer), f);
        sha256_update(&sha256, buffer, nbytes);
    } while (nbytes == sizeof(buffer));

    fclose(f);

    uint8_t sha256_result[32];
    sha256_final(&sha256, sha256_result);
    if (strncmp((const char *)sha256_result, (const char *)ide_dbg_sv_file.info.sha256, sizeof(sha256_result)) != 0) {
        resp = USBDBG_SVFILE_VERIFY_SHA2_ERR;
        pr_verb("file sha256 unmatched");
        print_sha256(sha256_result);
    }

    mp_hal_uart_tx(&resp, sizeof(resp));
}

static void cmd_createfile(ide_dbg_state_t* state) {
    pr_verb("cmd: USBDBG_CREATEFILE");
    hal_rvv_memset(&ide_dbg_sv_file.info, 0, sizeof(ide_dbg_sv_file.info));
    if (sizeof(ide_dbg_sv_file.info) != state->data_length) {
        ide_dbg_sv_file.errcode = USBDBG_SVFILE_ERR_PATH_ERR;
        pr_verb("Warning: CREATEFILE expect data length %lu, got %u",
                sizeof(ide_dbg_sv_file.info), state->data_length);
        return;
    }

    read_until(&ide_dbg_sv_file.info, sizeof(ide_dbg_sv_file.info));
    pr_verb("create file: chunk_size(%d), name(%s)",
            ide_dbg_sv_file.info.chunk_size, ide_dbg_sv_file.info.name);
    print_sha256(ide_dbg_sv_file.info.sha256);

    if (ide_dbg_sv_file.file != NULL)
        fclose(ide_dbg_sv_file.file);
    if (ide_dbg_sv_file.chunk_buffer) {
        free(ide_dbg_sv_file.chunk_buffer);
        ide_dbg_sv_file.chunk_buffer = NULL;
    }

    char filepath[120];
    if (!resolve_filepath(ide_dbg_sv_file.info.name, filepath, sizeof(filepath))) {
        ide_dbg_sv_file.errcode = USBDBG_SVFILE_ERR_PATH_ERR;
        return;
    }

    ide_dbg_sv_file.file = fopen(filepath, "w");
    if (ide_dbg_sv_file.file == NULL) {
        ide_dbg_sv_file.errcode = USBDBG_SVFILE_ERR_OPEN_ERR;
        return;
    }

    ide_dbg_sv_file.chunk_buffer = malloc(ide_dbg_sv_file.info.chunk_size);
    ide_dbg_sv_file.errcode = USBDBG_SVFILE_ERR_NONE;
}

static void cmd_frame_size(void) {
    #if PRINT_ALL
    pr_verb("cmd: USBDBG_FRAME_SIZE");
    #endif
    uint32_t resp[3] = { 0, 0, 0 };

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
                uint32_t img_width = 0, img_height = 0;
                if (!fb_vo_wbc_data) {
                    if (0x00 != ide_dbg_vo_wbc_dump_and_encode(
                            &fb_vo_wbc_data, &fb_vo_wbc_data_size,
                            &img_width, &img_height)) {
                        fb_vo_wbc_data = NULL;
                        fb_vo_wbc_data_size = 0;
                    }
                    resp[0] = img_width;
                    resp[1] = img_height;
                    resp[2] = fb_vo_wbc_data_size;
                }
            }
            pthread_mutex_unlock(&fb_mutex);
        }
    }

    mp_hal_uart_tx(&resp, sizeof(resp));
}

static void cmd_frame_dump(void) {
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
        }
    }
    pthread_mutex_unlock(&fb_mutex);
}

///////////////////////////////////////////////////////////////////////////////
// IDE protocol parser ////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

static ide_dbg_status_t ide_dbg_update(ide_dbg_state_t* state, const uint8_t* data, size_t length) {
    for (size_t i = 0; i < length;) {
        switch (state->state) {
            case FRAME_HEAD:
                if (data[i] == 0x30) {
                    state->state = FRAME_CMD;
                }
                i += 1;
                break;
            case FRAME_CMD:
                state->cmd = data[i];
                state->state = FRAME_DATA_LENGTH;
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
                #if !PRINT_ALL
                if ((state->cmd != USBDBG_SCRIPT_RUNNING) &&
                    (state->cmd != USBDBG_FRAME_SIZE) &&
                    (state->cmd != USBDBG_TX_BUF_LEN) &&
                    (state->cmd != USBDBG_FRAME_DUMP))
                #endif
                {
                    print_raw(data, length);
                    pr_verb("cmd: %x", state->cmd);
                }
                switch (state->cmd) {
                    case USBDBG_NONE:
                        break;
                    case USBDBG_QUERY_STATUS: {
                        pr_verb("cmd: USBDBG_QUERY_STATUS");
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
                    case USBDBG_ARCH_STR:
                        cmd_arch_str(state);
                        break;
                    case USBDBG_SCRIPT_EXEC:
                        cmd_script_exec(state);
                        break;
                    case USBDBG_SCRIPT_STOP: {
                        // TODO
                        pr_verb("cmd: USBDBG_SCRIPT_STOP");
                        // raise IDE interrupt
                        if (ide_dbg_script_running() != 0) {
                            wait_mp_irq_handler_done();
                            mp_thread_set_exception_main(MP_OBJ_FROM_PTR(&ide_exception));
                        }
                        break;
                    }
                    case USBDBG_SCRIPT_SAVE: {
                        // TODO
                        pr_verb("cmd: USBDBG_SCRIPT_SAVE");
                        break;
                    }
                    case USBDBG_QUERY_FILE_STAT: {
                        pr_verb("cmd: USBDBG_QUERY_FILE_STAT");
                        mp_hal_uart_tx(&ide_dbg_sv_file.errcode, sizeof(ide_dbg_sv_file.errcode));
                        break;
                    }
                    case USBDBG_WRITEFILE:
                        cmd_writefile(state);
                        break;
                    case USBDBG_VERIFYFILE:
                        cmd_verifyfile();
                        break;
                    case USBDBG_CREATEFILE:
                        cmd_createfile(state);
                        break;
                    case USBDBG_SCRIPT_RUNNING: {
                        // DO NOT PRINT
                        #if PRINT_ALL
                        pr_verb("cmd: USBDBG_SCRIPT_RUNNING");
                        #endif
                        uint32_t running = ide_dbg_script_running();
                        mp_hal_uart_tx(&running, sizeof(running));
                        break;
                    }
                    case USBDBG_TX_BUF_LEN: {
                        pthread_mutex_lock(&tx_buf_mutex);
                        uint32_t len = tx_buf_readable();
                        #if !PRINT_ALL
                        if (len > 0) {
                            pr_verb("cmd: USBDBG_TX_BUF_LEN %u", len);
                        }
                        #endif
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
                        tx_buf_drain(len);
                        pthread_mutex_unlock(&tx_buf_mutex);
                        break;
                    }
                    case USBDBG_FRAME_SIZE:
                        cmd_frame_size();
                        break;
                    case USBDBG_FRAME_DUMP:
                        cmd_frame_dump();
                        break;
                    case USBDBG_SYS_RESET: {
                        // TODO: reset serialport to REPL mode
                        pr_verb("cmd: USBDBG_SYS_RESET");
                        if (ide_dbg_script_running() != 0) {
                            ide_dbg_set_disconnect(true);
                            mp_thread_set_exception_main(MP_OBJ_FROM_PTR(&ide_exception));
                        } else {
                            ide_dbg_interrupt();
                        }
                        break;
                    }
                    case USBDBG_FB_ENABLE: {
                        // FIXME: stream parse
                        if (i + 1 < length) {
                            enable_pic = data[i+1];
                        }
                        pr_verb("cmd: USBDBG_FB_ENABLE, enable(%u)", enable_pic);
                        break;
                    }
                    default:
                        // unknown command
                        pr_verb("unknown command %02x", state->cmd);
                        break;
                }
                state->state = FRAME_HEAD;
                i += 1;
                break;
            case FRAME_RECV: {
                size_t avail = length - i;
                if (avail >= state->recv_lack) {
                    hal_rvv_memcpy(state->recv_data, data + i, state->recv_lack);
                    state->state = state->recv_next;
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

extern volatile bool is_repl_intr;

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
                    if (ide_dbg_script_running() != 0) {
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
        } else if (ide_dbg_attach()) {
            ide_dbg_update(&state, read_buf, size);
        } else {
            // FIXME: IDE connect
            // FIXME: IDE special token
            const char* IDE_TOKEN = "\x30\x8D\x04\x00\x00\x00"; // CanMV IDE
            const char* IDE_TOKEN2 = "\x30\x80\x0C\x00\x00\x00"; // OpenMV IDE
            const char* IDE_TOKEN3 = "\x30\x87\x04\x00\x00\x00";
            if ((size == 6) && (
                (strncmp((const char*)read_buf, IDE_TOKEN, size) == 0) ||
                (strncmp((const char*)read_buf, IDE_TOKEN2, size) == 0) ||
                (strncmp((const char*)read_buf, IDE_TOKEN3, size) == 0)
                )) {
                // switch to ide mode
                pr_verb("[uart] switch to IDE mode");
                if (!ide_dbg_attach()) {
                    interrupt_repl();
                }
                ide_dbg_set_attached(true);
                if (ide_dbg_script_running() != 0)
                    mp_thread_set_exception_main(MP_OBJ_FROM_PTR(&ide_exception));
                ide_dbg_update(&state, read_buf, size);
            } else {
                // FIXME: mock machine.UART, restore this when UART library finish
                const char* MOCK_FOR_IDE[] = {
                    "from machine import UART\r",
                    "repl = UART.repl_uart()\r",
                    "repl.init(1500000, 8, None, 1, read_buf_len=2048, ide=True)\r"
                };
                if ((size >= 23) && (
                    (strncmp((const char*)read_buf, MOCK_FOR_IDE[0], 23) == 0) ||
                    (strncmp((const char*)read_buf, MOCK_FOR_IDE[1], 23) == 0) ||
                    (strncmp((const char*)read_buf, MOCK_FOR_IDE[2], 23) == 0)
                    )) {
                    // ignore
                    continue;
                }
                // normal REPL
                pr_verb("[uart] read %zd bytes ", size);
                print_raw(read_buf, size);

                if(ide_dbg_repl_script_running() && (read_buf[0] == CHAR_CTRL_C)) {
                    static const uint8_t mark[3] = {0x03, 0x0d, 0x0a};

                    wait_mp_irq_handler_done();

                    if(0x01 == size) {
                        is_repl_intr = true;
                    } else if(0x03 == size) {
                        if((mark[0] == read_buf[0]) && (mark[1] == read_buf[1]) && (mark[2] == read_buf[2])) {
                            is_repl_intr = true;
                        }
                    }

                    if(is_repl_intr) {
                        // terminate script running
                        #if MICROPY_KBD_EXCEPTION
                        mp_thread_set_exception_main(MP_OBJ_FROM_PTR(&MP_STATE_VM(mp_kbd_exception)));
                        #endif
                    }
                } else {
                    mp_hal_stdin_push(read_buf, size);
                }
            }
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

