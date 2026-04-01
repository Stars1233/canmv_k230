/*
 * This file is part of the MicroPython project, http://micropython.org/
 *
 * The MIT License (MIT)
 *
 * Copyright (c) 2015 Damien P. George
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */

#include <pthread.h>
#include <signal.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

#include "py/mphal.h"
#include "py/mpthread.h"
#include "py/runtime.h"
#include "py/stream.h"

#include "extmod/misc.h"
#include "shared/runtime/interrupt_char.h"

#include "drv_uart.h"
#include "hal_utils.h"
#include "modmachine.h"

#include "ide_dbg.h"

#define MP_HAL_STDIN_WAIT_US (20000)

///////////////////////////////////////////////////////////////////////////////
// K230 UART low-level (used by mp_hal stdio & IDE) //////////////////////////
///////////////////////////////////////////////////////////////////////////////

static pthread_mutex_t mp_hal_uart_tx_mutex = PTHREAD_MUTEX_INITIALIZER;
static pthread_mutex_t mp_hal_uart_reader_mutex = PTHREAD_MUTEX_INITIALIZER;

static drv_uart_inst_t *mp_hal_uart_inst = NULL;

static pthread_t mp_hal_reader_tid;
static bool mp_hal_reader_started = false;
static bool mp_hal_reader_running = false;

drv_uart_inst_t *mp_hal_uart_get_instance(void) {
    return __atomic_load_n(&mp_hal_uart_inst, __ATOMIC_ACQUIRE);
}

static void mp_hal_uart_set_instance(drv_uart_inst_t *inst) {
    __atomic_store_n(&mp_hal_uart_inst, inst, __ATOMIC_RELEASE);
}

static void mp_hal_stdin_wait_deadline(struct timespec *ts) {
    clock_gettime(CLOCK_REALTIME, ts);
    ts->tv_nsec += MP_HAL_STDIN_WAIT_US * 1000;
    if (ts->tv_nsec >= 1000000000) {
        ts->tv_sec += ts->tv_nsec / 1000000000;
        ts->tv_nsec %= 1000000000;
    }
}

void mp_hal_uart_init(int id) {
    int ret;
    drv_uart_inst_t *inst = NULL;

    if (id < 0) {
        ret = drv_uart_inst_create_usb("/dev/ttyUSB", &inst);
    } else {
        ret = drv_uart_inst_create(id, &inst);
    }

    if (ret < 0) {
        fprintf(stderr, "mp_hal_uart_init: failed to open UART %d\n", id);
        return;
    }

    mp_hal_uart_set_instance(inst);
}

int mp_hal_uart_tx(const void* buffer, size_t size) {
#define MP_HAL_UART_TX_BLOCK_SIZE (4 * 1024)
    drv_uart_inst_t *inst = mp_hal_uart_get_instance();

    if (inst == NULL)
        return -1;
    if (drv_uart_is_dtr_asserted(inst) <= 0)
        return 0;

    pthread_mutex_lock(&mp_hal_uart_tx_mutex);
    size_t sent = 0;
    while (sent < size) {
        size_t chunk = size - sent;
        if (chunk > MP_HAL_UART_TX_BLOCK_SIZE)
            chunk = MP_HAL_UART_TX_BLOCK_SIZE;
        ssize_t n = drv_uart_write(inst, (const uint8_t*)buffer + sent, chunk);
        if (n == (ssize_t)chunk) {
            sent += chunk;
        } else {
            break;
        }
    }
    pthread_mutex_unlock(&mp_hal_uart_tx_mutex);
    return sent;
#undef MP_HAL_UART_TX_BLOCK_SIZE
}

///////////////////////////////////////////////////////////////////////////////
// K230 stdin ring buffer & background reader /////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

static pthread_mutex_t mp_hal_stdin_lock = PTHREAD_MUTEX_INITIALIZER;
static pthread_cond_t  mp_hal_stdin_cond = PTHREAD_COND_INITIALIZER;
static char mp_hal_stdin_ringbuf[4096];
static unsigned mp_hal_stdin_wptr = 0;
static unsigned mp_hal_stdin_rptr = 0;

static void mp_hal_interrupt(void)
{
    if (ide_dbg_attach()) {
        ide_dbg_interrupt();
    }

    mp_thread_set_exception_main(MP_OBJ_FROM_PTR(&MP_STATE_VM(mp_kbd_exception)));
    mp_sched_keyboard_interrupt();
}

void mp_hal_stdin_push(const uint8_t* data, size_t len) {
    pthread_mutex_lock(&mp_hal_stdin_lock);
    for (size_t i = 0; i < len; i++) {
        if (data[i] == mp_interrupt_char) {
            pthread_mutex_unlock(&mp_hal_stdin_lock);

            mp_hal_interrupt();
            return;
        }
        mp_hal_stdin_ringbuf[mp_hal_stdin_wptr++] = data[i];
        mp_hal_stdin_wptr %= sizeof(mp_hal_stdin_ringbuf);
    }
    pthread_cond_signal(&mp_hal_stdin_cond);
    pthread_mutex_unlock(&mp_hal_stdin_lock);
}

void mp_hal_stdin_clear(void) {
    pthread_mutex_lock(&mp_hal_stdin_lock);
    mp_hal_stdin_rptr = mp_hal_stdin_wptr;
    pthread_mutex_unlock(&mp_hal_stdin_lock);
}

static void* mp_hal_uart_reader(void* arg) {
    static uint8_t buf[512];

    (void)arg;

    while (__atomic_load_n(&mp_hal_reader_running, __ATOMIC_ACQUIRE)) {
        drv_uart_inst_t *inst = mp_hal_uart_get_instance();
        if (inst == NULL) {
            break;
        }

        int ret = drv_uart_poll(inst, 200);
        if (ret <= 0)
            continue;
        ssize_t n = drv_uart_read(inst, buf, sizeof(buf));
        if (n > 0)
            mp_hal_stdin_push(buf, n);
    }
    return NULL;
}

void mp_hal_uart_reader_start(void) {
    pthread_mutex_lock(&mp_hal_uart_reader_mutex);
    if (mp_hal_reader_started) {
        pthread_mutex_unlock(&mp_hal_uart_reader_mutex);
        return;
    }

    pthread_mutex_lock(&mp_hal_stdin_lock);
    mp_hal_stdin_wptr = 0;
    mp_hal_stdin_rptr = 0;
    pthread_mutex_unlock(&mp_hal_stdin_lock);

    __atomic_store_n(&mp_hal_reader_running, true, __ATOMIC_RELEASE);
    if (pthread_create(&mp_hal_reader_tid, NULL, mp_hal_uart_reader, NULL) == 0) {
        mp_hal_reader_started = true;
    } else {
        __atomic_store_n(&mp_hal_reader_running, false, __ATOMIC_RELEASE);
    }
    pthread_mutex_unlock(&mp_hal_uart_reader_mutex);
}

void mp_hal_uart_reader_stop(void) {
    pthread_t reader_tid;

    pthread_mutex_lock(&mp_hal_uart_reader_mutex);
    if (!mp_hal_reader_started) {
        pthread_mutex_unlock(&mp_hal_uart_reader_mutex);
        return;
    }

    __atomic_store_n(&mp_hal_reader_running, false, __ATOMIC_RELEASE);
    reader_tid = mp_hal_reader_tid;
    pthread_mutex_unlock(&mp_hal_uart_reader_mutex);

    pthread_join(reader_tid, NULL);

    pthread_mutex_lock(&mp_hal_uart_reader_mutex);
    mp_hal_reader_started = false;
    pthread_mutex_unlock(&mp_hal_uart_reader_mutex);
}

///////////////////////////////////////////////////////////////////////////////
// MicroPython HAL: stdin & stdout ////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

void mp_hal_poll_dupterm(void) {
    if (mp_os_dupterm_poll(MP_STREAM_POLL_RD) & MP_STREAM_POLL_RD) {
        int c = mp_os_dupterm_rx_chr();
        if (c >= 0) {
            uint8_t byte = c;
            mp_hal_stdin_push(&byte, 1);
        }
    }
}

int mp_hal_stdin_rx_chr(void) {
    while (1) {
        pthread_mutex_lock(&mp_hal_stdin_lock);
        if (mp_hal_stdin_rptr != mp_hal_stdin_wptr) {
            int c = (unsigned char)mp_hal_stdin_ringbuf[mp_hal_stdin_rptr++];
            mp_hal_stdin_rptr %= sizeof(mp_hal_stdin_ringbuf);
            pthread_mutex_unlock(&mp_hal_stdin_lock);
            return c;
        }

        struct timespec ts;
        mp_hal_stdin_wait_deadline(&ts);
        MP_THREAD_GIL_EXIT();
        pthread_cond_timedwait(&mp_hal_stdin_cond, &mp_hal_stdin_lock, &ts);
        MP_THREAD_GIL_ENTER();
        pthread_mutex_unlock(&mp_hal_stdin_lock);
        mp_hal_poll_dupterm();
        mp_handle_pending(true);
    }
}

void mp_hal_stdout_tx_str(const char *str) {
    mp_hal_stdout_tx_strn(str, strlen(str));
}

void mp_hal_stdout_tx_strn(const char *str, size_t len) {
    if (ide_dbg_attach()) {
        ide_dbg_stdout_tx(str, len);
    } else {
        mp_hal_uart_tx(str, len);
    }
    mp_os_dupterm_tx_strn(str, len);
}

void mp_hal_stdout_tx_strn_cooked(const char *str, size_t len) {
    size_t last_seg = 0;

    for (size_t i = 0; i < len; i++) {
        if (str[i] == '\n') {
            mp_hal_stdout_tx_strn(str + last_seg, i - last_seg);
            mp_hal_stdout_tx_strn("\r\n", 2);
            last_seg = i + 1;
        }
    }
    if (last_seg < len)
        mp_hal_stdout_tx_strn(str + last_seg, len - last_seg);
}

void mp_hal_stdout_tx_str_cooked(const char* str) {
    mp_hal_stdout_tx_strn_cooked(str, strlen(str));
}

///////////////////////////////////////////////////////////////////////////////
// MicroPython HAL: time & delay //////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

mp_uint_t mp_hal_ticks_ms(void) {
    return (mp_uint_t)utils_cpu_ticks_ms();
}

mp_uint_t mp_hal_ticks_us(void) {
    return (mp_uint_t)utils_cpu_ticks_us();
}

uint64_t mp_hal_time_ns(void) {
    return (mp_uint_t)utils_cpu_ticks_ns();
}

mp_uint_t mp_hal_ticks_cpu(void) {
    return (mp_uint_t)utils_cpu_ticks();
}

void mp_hal_delay_us(mp_uint_t us) {
    mp_uint_t start = mp_hal_ticks_us();
    mp_uint_t stop = start + us;

    while ((start + 5000) < stop) {
        mp_thread_exitpoint(EXITPOINT_ENABLE_SLEEP);
        mp_handle_pending(true);
        mp_hal_poll_dupterm();

        MP_THREAD_GIL_EXIT();
        usleep(5000);
        MP_THREAD_GIL_ENTER();

        start = mp_hal_ticks_us();
    }

    if (stop > start) {
        mp_thread_exitpoint(EXITPOINT_ENABLE_SLEEP);
        mp_handle_pending(true);
        mp_hal_poll_dupterm();

        MP_THREAD_GIL_EXIT();
        usleep(stop - start);
        MP_THREAD_GIL_ENTER();
    }
}

void mp_hal_delay_ms(mp_uint_t ms) {
    mp_hal_delay_us(ms * 1000);
}

void mp_hal_delay_us_fast(uint64_t us) {
    uint64_t end = utils_cpu_ticks_us() + us;
    while (utils_cpu_ticks_us() < end) { }
}

///////////////////////////////////////////////////////////////////////////////
// MicroPython HAL: GPIO pin //////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

mp_hal_pin_obj_t mp_hal_get_pin_obj(mp_obj_t o) {
    return machine_pin_get_inst(o);
}

int mp_hal_pin_name(mp_hal_pin_obj_t pin) {
    return pin->pin;
}

void mp_hal_pin_input(mp_hal_pin_obj_t pin) {
    drv_gpio_mode_set(pin, GPIO_DM_INPUT);
}

void mp_hal_pin_output(mp_hal_pin_obj_t pin) {
    drv_gpio_mode_set(pin, GPIO_DM_OUTPUT);
}

void mp_hal_pin_open_drain(mp_hal_pin_obj_t pin) {
    drv_gpio_mode_set(pin, GPIO_DM_OUTPUT_OD);
}

void mp_hal_pin_od_low(mp_hal_pin_obj_t pin) {
    drv_gpio_value_set(pin, 0);
}

void mp_hal_pin_od_high(mp_hal_pin_obj_t pin) {
    drv_gpio_value_set(pin, 1);
}

int mp_hal_pin_read(mp_hal_pin_obj_t pin) {
    return drv_gpio_value_get(pin);
}

void mp_hal_pin_write(mp_hal_pin_obj_t pin, int value) {
    drv_gpio_value_set(pin, value);
}

///////////////////////////////////////////////////////////////////////////////
// MicroPython HAL: IRQ ///////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

uint32_t mp_hal_quiet_timing_enter(void) {
    uint32_t irq_state = 0;
    return irq_state;
}

void mp_hal_quiet_timing_exit(uint32_t irq_state) {
    (void)irq_state;
}
