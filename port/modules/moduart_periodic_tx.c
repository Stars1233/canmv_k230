/* Copyright (c) 2026, Canaan Bright Sight Co., Ltd
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 * 1. Redistributions of source code must retain the above copyright
 * notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 * notice, this list of conditions and the following disclaimer in the
 * documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND
 * CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES,
 * INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "py/obj.h"
#include "py/mperrno.h"
#include "py/runtime.h"

#include "drv_timer.h"
#include "drv_uart.h"
#include "modmachine.h"

#define UART_PERIODIC_TX_BUFFER_COUNT      (3)
#define UART_PERIODIC_TX_NO_READER          (UINT32_MAX)
#define UART_PERIODIC_TX_DEFAULT_CAPACITY   (64)
#define UART_PERIODIC_TX_MAX_CAPACITY       (4096)

typedef struct _uart_periodic_tx_obj_t uart_periodic_tx_obj_t;

typedef struct {
    void* volatile owner;
    uint32_t volatile in_callback;
} uart_periodic_tx_slot_t;

typedef struct {
    drv_uart_inst_t* inst;
    struct uart_configure config;
    int fd;
    uint32_t users;
    uint32_t volatile tx_busy;
} uart_periodic_tx_uart_state_t;

struct _uart_periodic_tx_obj_t {
    mp_obj_base_t base;

    drv_hard_timer_inst_t* timer;
    uart_periodic_tx_slot_t* slot;
    uart_periodic_tx_uart_state_t* uart_state;

    uint8_t* buffers[UART_PERIODIC_TX_BUFFER_COUNT];
    size_t buffer_len[UART_PERIODIC_TX_BUFFER_COUNT];
    uint32_t volatile buffer_generation[UART_PERIODIC_TX_BUFFER_COUNT];
    size_t capacity;

    struct uart_configure uart_config;
    int uart_id;
    int timer_id;
    int period_ms;

    bool timer_claimed;
    bool uart_claimed;
    bool repeat_last;
    uint32_t volatile running;
    uint32_t volatile active_idx;
    uint32_t volatile reader_idx;
    uint32_t volatile next_generation;
    uint32_t volatile last_sent_generation;
    uint32_t volatile sent_count;
    uint32_t volatile short_write_count;
    uint32_t volatile error_count;
    uint32_t volatile skipped_count;
};

STATIC uart_periodic_tx_slot_t uart_periodic_tx_slots[KD_TIMER_MAX_NUM];
STATIC uart_periodic_tx_uart_state_t uart_periodic_tx_uarts[KD_HARD_UART_MAX_NUM];
MP_REGISTER_ROOT_POINTER(void* uart_periodic_tx_active_obj[KD_TIMER_MAX_NUM]);

STATIC bool uart_periodic_tx_uart_config_equal(const struct uart_configure* left, const struct uart_configure* right)
{
    return left->baud_rate == right->baud_rate &&
        left->data_bits == right->data_bits &&
        left->stop_bits == right->stop_bits &&
        left->parity == right->parity &&
        left->bit_order == right->bit_order &&
        left->invert == right->invert;
}

STATIC int uart_periodic_tx_uart_acquire(uart_periodic_tx_obj_t* self)
{
    uart_periodic_tx_uart_state_t* state = &uart_periodic_tx_uarts[self->uart_id];

    if (state->users == 0) {
        drv_uart_inst_t* inst = NULL;
        struct uart_configure config = self->uart_config;

        if (drv_uart_inst_create(self->uart_id, &inst) != 0) {
            return -1;
        }
        if (drv_uart_set_config(inst, &config) != 0 || drv_uart_get_fd(inst) < 0) {
            drv_uart_inst_destroy(&inst);
            return -1;
        }

        state->inst = inst;
        state->config = config;
        state->fd = drv_uart_get_fd(inst);
        __atomic_store_n(&state->tx_busy, 0, __ATOMIC_RELEASE);
    } else if (!uart_periodic_tx_uart_config_equal(&state->config, &self->uart_config)) {
        return -2;
    }

    state->users++;
    self->uart_state = state;
    self->uart_claimed = true;
    return 0;
}

STATIC void uart_periodic_tx_uart_release(uart_periodic_tx_obj_t* self)
{
    uart_periodic_tx_uart_state_t* state = self->uart_state;

    if (!self->uart_claimed || state == NULL) {
        return;
    }

    if (state->users != 0) {
        state->users--;
        if (state->users == 0) {
            __atomic_store_n(&state->tx_busy, 0, __ATOMIC_RELEASE);
            state->fd = -1;
            drv_uart_inst_destroy(&state->inst);
            memset(&state->config, 0, sizeof(state->config));
        }
    }

    self->uart_state = NULL;
    self->uart_claimed = false;
}

STATIC void uart_periodic_tx_callback(void* arg)
{
    uart_periodic_tx_slot_t* slot = arg;
    uart_periodic_tx_obj_t*  self;

    // This signal handler uses only atomics and write(2); the static slot also
    // prevents a late signal from dereferencing freed memory.
    __atomic_fetch_add(&slot->in_callback, 1, __ATOMIC_ACQ_REL);
    self = __atomic_load_n(&slot->owner, __ATOMIC_ACQUIRE);

    if (self != NULL && __atomic_load_n(&self->running, __ATOMIC_ACQUIRE)) {
        uint32_t index = UART_PERIODIC_TX_NO_READER;

        // Publish the buffer being read before using it.  update() only writes a
        // buffer that is neither active nor being read by this callback.
        for (size_t attempt = 0; attempt < UART_PERIODIC_TX_BUFFER_COUNT; ++attempt) {
            index = __atomic_load_n(&self->active_idx, __ATOMIC_ACQUIRE);
            __atomic_store_n(&self->reader_idx, index, __ATOMIC_RELEASE);
            if (index == __atomic_load_n(&self->active_idx, __ATOMIC_ACQUIRE)) {
                break;
            }
            index = UART_PERIODIC_TX_NO_READER;
        }

        if (index != UART_PERIODIC_TX_NO_READER) {
            size_t len = self->buffer_len[index];
            uint32_t generation = __atomic_load_n(&self->buffer_generation[index], __ATOMIC_ACQUIRE);
            bool should_send = len != 0 && (self->repeat_last ||
                generation != __atomic_load_n(&self->last_sent_generation, __ATOMIC_ACQUIRE));

            if (should_send) {
                uart_periodic_tx_uart_state_t* uart_state = self->uart_state;
                if (uart_state == NULL ||
                    __atomic_exchange_n(&uart_state->tx_busy, 1, __ATOMIC_ACQUIRE) != 0) {
                    __atomic_fetch_add(&self->skipped_count, 1, __ATOMIC_RELAXED);
                } else {
                    ssize_t written = write(uart_state->fd, self->buffers[index], len);
                    if (written == (ssize_t)len) {
                        __atomic_fetch_add(&self->sent_count, 1, __ATOMIC_RELAXED);
                        // In one-shot mode, retain data after a short or failed
                        // write so it can be retried by a later timer tick.
                        if (!self->repeat_last) {
                            __atomic_store_n(&self->last_sent_generation, generation, __ATOMIC_RELEASE);
                        }
                    } else if (written >= 0) {
                        __atomic_fetch_add(&self->short_write_count, 1, __ATOMIC_RELAXED);
                    } else {
                        __atomic_fetch_add(&self->error_count, 1, __ATOMIC_RELAXED);
                    }
                    __atomic_store_n(&uart_state->tx_busy, 0, __ATOMIC_RELEASE);
                }
            } else {
                __atomic_fetch_add(&self->skipped_count, 1, __ATOMIC_RELAXED);
            }
            __atomic_store_n(&self->reader_idx, UART_PERIODIC_TX_NO_READER, __ATOMIC_RELEASE);
        } else {
            __atomic_store_n(&self->reader_idx, UART_PERIODIC_TX_NO_READER, __ATOMIC_RELEASE);
            __atomic_fetch_add(&self->skipped_count, 1, __ATOMIC_RELAXED);
        }
    }

    __atomic_fetch_sub(&slot->in_callback, 1, __ATOMIC_ACQ_REL);
}

STATIC void uart_periodic_tx_stop_internal(uart_periodic_tx_obj_t* self)
{
    if (self->slot != NULL) {
        __atomic_store_n(&self->slot->owner, NULL, __ATOMIC_RELEASE);
    }
    __atomic_store_n(&self->running, 0, __ATOMIC_RELEASE);

    if (self->timer != NULL) {
        if (drv_hard_timer_is_started(self->timer)) {
            (void)drv_hard_timer_stop(self->timer);
        }
        (void)drv_hard_timer_unregister_irq(self->timer);

        if (self->slot != NULL) {
            while (__atomic_load_n(&self->slot->in_callback, __ATOMIC_ACQUIRE) != 0) {
            }
        }

        drv_hard_timer_inst_destroy(&self->timer);
    }

    if (self->timer_id >= 0 && self->timer_id < KD_TIMER_MAX_NUM &&
        MP_STATE_PORT(uart_periodic_tx_active_obj[self->timer_id]) == self) {
        MP_STATE_PORT(uart_periodic_tx_active_obj[self->timer_id]) = NULL;
    }
    if (self->timer_claimed) {
        machine_timer_native_release(self->timer_id);
        self->timer_claimed = false;
    }
    uart_periodic_tx_uart_release(self);

    self->slot = NULL;
    __atomic_store_n(&self->reader_idx, UART_PERIODIC_TX_NO_READER, __ATOMIC_RELEASE);
}

STATIC void uart_periodic_tx_deinit_internal(uart_periodic_tx_obj_t* self)
{
    uart_periodic_tx_stop_internal(self);

    for (size_t i = 0; i < UART_PERIODIC_TX_BUFFER_COUNT; ++i) {
        free(self->buffers[i]);
        self->buffers[i] = NULL;
        self->buffer_len[i] = 0;
    }
    self->capacity = 0;
}

void uart_periodic_tx_deinit_all(void)
{
    for (size_t i = 0; i < KD_TIMER_MAX_NUM; ++i) {
        uart_periodic_tx_obj_t* self = MP_STATE_PORT(uart_periodic_tx_active_obj[i]);
        if (self != NULL) {
            uart_periodic_tx_deinit_internal(self);
        }
    }
}

STATIC mp_obj_t uart_periodic_tx_make_new(const mp_obj_type_t* type, size_t n_args, size_t n_kw, const mp_obj_t* args)
{
    enum { ARG_uart_id, ARG_timer_id, ARG_period, ARG_max_len, ARG_baudrate, ARG_bits, ARG_parity, ARG_stop, ARG_repeat_last };
    static const mp_arg_t allowed_args[] = {
        { MP_QSTR_uart_id, MP_ARG_REQUIRED | MP_ARG_INT, { .u_int = -1 } },
        { MP_QSTR_timer_id, MP_ARG_REQUIRED | MP_ARG_INT, { .u_int = -1 } },
        { MP_QSTR_period, MP_ARG_INT, { .u_int = 50 } },
        { MP_QSTR_max_len, MP_ARG_KW_ONLY | MP_ARG_INT, { .u_int = UART_PERIODIC_TX_DEFAULT_CAPACITY } },
        { MP_QSTR_baudrate, MP_ARG_KW_ONLY | MP_ARG_INT, { .u_int = 115200 } },
        { MP_QSTR_bits, MP_ARG_KW_ONLY | MP_ARG_INT, { .u_int = DATA_BITS_8 } },
        { MP_QSTR_parity, MP_ARG_KW_ONLY | MP_ARG_INT, { .u_int = PARITY_NONE } },
        { MP_QSTR_stop, MP_ARG_KW_ONLY | MP_ARG_INT, { .u_int = STOP_BITS_1 } },
        { MP_QSTR_repeat_last, MP_ARG_KW_ONLY | MP_ARG_BOOL, { .u_bool = true } },
    };
    mp_map_t kw_args;
    mp_arg_val_t parsed_args[MP_ARRAY_SIZE(allowed_args)];

    mp_arg_check_num(n_args, n_kw, 0, 3, true);
    mp_map_init_fixed_table(&kw_args, n_kw, args + n_args);
    mp_arg_parse_all(n_args, args, &kw_args, MP_ARRAY_SIZE(allowed_args), allowed_args, parsed_args);

    int uart_id = parsed_args[ARG_uart_id].u_int;
    int timer_id = parsed_args[ARG_timer_id].u_int;
    int period_ms = parsed_args[ARG_period].u_int;
    int max_len = parsed_args[ARG_max_len].u_int;
    int baudrate = parsed_args[ARG_baudrate].u_int;
    int bits = parsed_args[ARG_bits].u_int;
    int parity = parsed_args[ARG_parity].u_int;
    int stop = parsed_args[ARG_stop].u_int;
    bool repeat_last = parsed_args[ARG_repeat_last].u_bool;
    if (uart_id < 0 || uart_id >= KD_HARD_UART_MAX_NUM) {
        mp_raise_ValueError(MP_ERROR_TEXT("invalid UART id"));
    }
    if (timer_id < 0 || timer_id >= KD_TIMER_MAX_NUM) {
        mp_raise_ValueError(MP_ERROR_TEXT("invalid timer id"));
    }
    if (period_ms < 1) {
        mp_raise_ValueError(MP_ERROR_TEXT("period must be >= 1 ms"));
    }
    if (max_len < 1 || max_len > UART_PERIODIC_TX_MAX_CAPACITY) {
        mp_raise_ValueError(MP_ERROR_TEXT("invalid max_len"));
    }
    if (baudrate < 1) {
        mp_raise_ValueError(MP_ERROR_TEXT("baudrate must be positive"));
    }
    if (bits < DATA_BITS_5 || bits > DATA_BITS_9 ||
        parity < PARITY_NONE || parity > PARITY_EVEN ||
        stop < STOP_BITS_1 || stop > STOP_BITS_4) {
        mp_raise_ValueError(MP_ERROR_TEXT("invalid UART configuration"));
    }

    uart_periodic_tx_obj_t* self = m_new_obj_with_finaliser(uart_periodic_tx_obj_t);
    memset(self, 0, sizeof(*self));
    self->base.type = type;
    self->uart_id = uart_id;
    self->timer_id = timer_id;
    self->period_ms = period_ms;
    self->capacity = max_len;
    self->repeat_last = repeat_last;
    self->uart_config = (struct uart_configure) {
        .baud_rate = baudrate,
        .data_bits = bits,
        .stop_bits = stop,
        .parity = parity,
        .bit_order = BIT_ORDER_LSB,
        .invert = NRZ_NORMAL,
        .bufsz = 0x400,
        .reserved = 0,
    };
    __atomic_store_n(&self->active_idx, 0, __ATOMIC_RELAXED);
    __atomic_store_n(&self->reader_idx, UART_PERIODIC_TX_NO_READER, __ATOMIC_RELAXED);
    __atomic_store_n(&self->next_generation, 0, __ATOMIC_RELAXED);
    __atomic_store_n(&self->last_sent_generation, UINT32_MAX, __ATOMIC_RELAXED);

    for (size_t i = 0; i < UART_PERIODIC_TX_BUFFER_COUNT; ++i) {
        self->buffers[i] = malloc(self->capacity);
        if (self->buffers[i] == NULL) {
            uart_periodic_tx_deinit_internal(self);
            mp_raise_OSError(MP_ENOMEM);
        }
        __atomic_store_n(&self->buffer_generation[i], 0, __ATOMIC_RELAXED);
    }

    return MP_OBJ_FROM_PTR(self);
}

STATIC mp_obj_t uart_periodic_tx_start(mp_obj_t self_in)
{
    uart_periodic_tx_obj_t* self = MP_OBJ_TO_PTR(self_in);
    if (self->buffers[0] == NULL) {
        mp_raise_ValueError(MP_ERROR_TEXT("object is deinitialized"));
    }
    if (self->timer != NULL) {
        return mp_const_none;
    }
    if (machine_timer_native_claim(self->timer_id) != 0) {
        mp_raise_OSError(MP_EBUSY);
    }
    self->timer_claimed = true;

    int uart_result = uart_periodic_tx_uart_acquire(self);
    if (uart_result != 0) {
        uart_periodic_tx_stop_internal(self);
        if (uart_result == -2) {
            mp_raise_OSError(MP_EBUSY);
        }
        mp_raise_msg(&mp_type_RuntimeError, MP_ERROR_TEXT("configure UART failed"));
    }

    if (drv_hard_timer_inst_create(self->timer_id, &self->timer) != 0 ||
        drv_hard_timer_set_mode(self->timer, HWTIMER_MODE_PERIOD) != 0 ||
        drv_hard_timer_set_period(self->timer, self->period_ms) != 0) {
        uart_periodic_tx_stop_internal(self);
        mp_raise_msg(&mp_type_RuntimeError, MP_ERROR_TEXT("configure hard timer failed"));
    }

    self->slot = &uart_periodic_tx_slots[self->timer_id];
    __atomic_store_n(&self->slot->owner, self, __ATOMIC_RELEASE);
    if (drv_hard_timer_register_irq(self->timer, uart_periodic_tx_callback, self->slot) != 0) {
        uart_periodic_tx_stop_internal(self);
        mp_raise_msg(&mp_type_RuntimeError, MP_ERROR_TEXT("register hard timer callback failed"));
    }

    MP_STATE_PORT(uart_periodic_tx_active_obj[self->timer_id]) = self;
    __atomic_store_n(&self->running, 1, __ATOMIC_RELEASE);
    if (drv_hard_timer_start(self->timer) != 0) {
        uart_periodic_tx_stop_internal(self);
        mp_raise_msg(&mp_type_RuntimeError, MP_ERROR_TEXT("start hard timer failed"));
    }

    return mp_const_none;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_1(uart_periodic_tx_start_obj, uart_periodic_tx_start);

STATIC mp_obj_t uart_periodic_tx_stop(mp_obj_t self_in)
{
    uart_periodic_tx_obj_t* self = MP_OBJ_TO_PTR(self_in);
    uart_periodic_tx_stop_internal(self);
    return mp_const_none;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_1(uart_periodic_tx_stop_obj, uart_periodic_tx_stop);

STATIC mp_obj_t uart_periodic_tx_deinit(mp_obj_t self_in)
{
    uart_periodic_tx_obj_t* self = MP_OBJ_TO_PTR(self_in);
    uart_periodic_tx_deinit_internal(self);
    return mp_const_none;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_1(uart_periodic_tx_deinit_obj, uart_periodic_tx_deinit);

STATIC mp_obj_t uart_periodic_tx_update(mp_obj_t self_in, mp_obj_t data_in)
{
    uart_periodic_tx_obj_t* self = MP_OBJ_TO_PTR(self_in);
    if (self->buffers[0] == NULL) {
        mp_raise_ValueError(MP_ERROR_TEXT("object is deinitialized"));
    }

    mp_buffer_info_t bufinfo;
    mp_get_buffer_raise(data_in, &bufinfo, MP_BUFFER_READ);
    if (bufinfo.len > self->capacity) {
        mp_raise_ValueError(MP_ERROR_TEXT("data exceeds max_len"));
    }

    uint32_t active = __atomic_load_n(&self->active_idx, __ATOMIC_ACQUIRE);
    uint32_t reader = __atomic_load_n(&self->reader_idx, __ATOMIC_ACQUIRE);
    uint32_t target = UART_PERIODIC_TX_NO_READER;
    for (uint32_t i = 0; i < UART_PERIODIC_TX_BUFFER_COUNT; ++i) {
        if (i != active && i != reader) {
            target = i;
            break;
        }
    }
    if (target == UART_PERIODIC_TX_NO_READER) {
        mp_raise_OSError(MP_EBUSY);
    }

    if (bufinfo.len != 0) {
        memcpy(self->buffers[target], bufinfo.buf, bufinfo.len);
    }
    self->buffer_len[target] = bufinfo.len;
    uint32_t generation = __atomic_add_fetch(&self->next_generation, 1, __ATOMIC_RELAXED);
    __atomic_store_n(&self->buffer_generation[target], generation, __ATOMIC_RELEASE);
    __atomic_store_n(&self->active_idx, target, __ATOMIC_RELEASE);

    return mp_const_none;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_2(uart_periodic_tx_update_obj, uart_periodic_tx_update);

STATIC mp_obj_t uart_periodic_tx_active(mp_obj_t self_in)
{
    uart_periodic_tx_obj_t* self = MP_OBJ_TO_PTR(self_in);
    return mp_obj_new_bool(__atomic_load_n(&self->running, __ATOMIC_ACQUIRE));
}
STATIC MP_DEFINE_CONST_FUN_OBJ_1(uart_periodic_tx_active_obj, uart_periodic_tx_active);

STATIC mp_obj_t uart_periodic_tx_stats(mp_obj_t self_in)
{
    uart_periodic_tx_obj_t* self = MP_OBJ_TO_PTR(self_in);
    mp_obj_t stats[] = {
        mp_obj_new_int_from_uint(__atomic_load_n(&self->sent_count, __ATOMIC_RELAXED)),
        mp_obj_new_int_from_uint(__atomic_load_n(&self->short_write_count, __ATOMIC_RELAXED)),
        mp_obj_new_int_from_uint(__atomic_load_n(&self->error_count, __ATOMIC_RELAXED)),
        mp_obj_new_int_from_uint(__atomic_load_n(&self->skipped_count, __ATOMIC_RELAXED)),
    };
    return mp_obj_new_tuple(MP_ARRAY_SIZE(stats), stats);
}
STATIC MP_DEFINE_CONST_FUN_OBJ_1(uart_periodic_tx_stats_obj, uart_periodic_tx_stats);

//| # Auto-generated CanMV stub docs. Edit the signatures/docstrings here.
//| module: uart_periodic_tx
//| """Hardware-timer UART transmission."""
//| class UARTPeriodicTx:
//|     """Periodically transmit complete UART frames from a hardware timer.
//|
//|     The class owns its UART driver instance. Route the UART TX pin with
//|     machine.FPIOA before calling start(). A tick that overlaps another
//|     UARTPeriodicTx on the same UART is skipped.
//|     """
//|     def __init__(self, uart_id: int, timer_id: int, period: int = 50, *, max_len: int = 64, baudrate: int = 115200, bits: int = 8, parity: int = 0, stop: int = 0, repeat_last: bool = True) -> None:
//|         """Create a hardware-timer UART transmitter."""
//|     def start(self) -> None:
//|         """Start periodic transmission."""
//|     def stop(self) -> None:
//|         """Stop periodic transmission without releasing the buffers."""
//|     def deinit(self) -> None:
//|         """Stop transmission and release native resources."""
//|     def update(self, data: Any) -> None:
//|         """Atomically publish a complete frame for the next timer tick."""
//|     def active(self) -> bool:
//|         """Return whether the hardware timer is running."""
//|     def stats(self) -> tuple:
//|         """Return sent, short-write, error, and skipped-tick counts."""

STATIC const mp_rom_map_elem_t uart_periodic_tx_locals_dict_table[] = {
    { MP_ROM_QSTR(MP_QSTR___del__), MP_ROM_PTR(&uart_periodic_tx_deinit_obj) },
    { MP_ROM_QSTR(MP_QSTR_start), MP_ROM_PTR(&uart_periodic_tx_start_obj) },
    { MP_ROM_QSTR(MP_QSTR_stop), MP_ROM_PTR(&uart_periodic_tx_stop_obj) },
    { MP_ROM_QSTR(MP_QSTR_deinit), MP_ROM_PTR(&uart_periodic_tx_deinit_obj) },
    { MP_ROM_QSTR(MP_QSTR_update), MP_ROM_PTR(&uart_periodic_tx_update_obj) },
    { MP_ROM_QSTR(MP_QSTR_active), MP_ROM_PTR(&uart_periodic_tx_active_obj) },
    { MP_ROM_QSTR(MP_QSTR_stats), MP_ROM_PTR(&uart_periodic_tx_stats_obj) },
};
STATIC MP_DEFINE_CONST_DICT(uart_periodic_tx_locals_dict, uart_periodic_tx_locals_dict_table);

/* clang-format off */
MP_DEFINE_CONST_OBJ_TYPE(
    uart_periodic_tx_type,
    MP_QSTR_UARTPeriodicTx,
    MP_TYPE_FLAG_NONE,
    make_new, uart_periodic_tx_make_new,
    locals_dict, &uart_periodic_tx_locals_dict
);
/* clang-format on */

STATIC const mp_rom_map_elem_t uart_periodic_tx_module_globals_table[] = {
    { MP_ROM_QSTR(MP_QSTR___name__), MP_ROM_QSTR(MP_QSTR_uart_periodic_tx) },
    { MP_ROM_QSTR(MP_QSTR_UARTPeriodicTx), MP_ROM_PTR(&uart_periodic_tx_type) },
};
STATIC MP_DEFINE_CONST_DICT(uart_periodic_tx_module_globals, uart_periodic_tx_module_globals_table);

const mp_obj_module_t uart_periodic_tx_module = {
    .base = { &mp_type_module },
    .globals = (mp_obj_dict_t*)&uart_periodic_tx_module_globals,
};

MP_REGISTER_MODULE(MP_QSTR_uart_periodic_tx, uart_periodic_tx_module);
