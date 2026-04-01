/* Copyright (c) 2025, Canaan Bright Sight Co., Ltd
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
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
 * BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
 * NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "mpprint.h"
#include "py/obj.h"
#include "py/runtime.h"
#include "py/stream.h"

#include "drv_uart.h"
#include "drv_fpioa.h"

#include "modmachine.h"

typedef struct {
    mp_obj_base_t base;
    int           index;
    int           status;
    int           timeout;

    int baudrate, bitwidth, parity, stop;

    drv_uart_inst_t* inst;

} machine_uart_obj_t;

STATIC void machine_uart_print(const mp_print_t* print, mp_obj_t self_in, mp_print_kind_t kind)
{
    machine_uart_obj_t* self = MP_OBJ_TO_PTR(self_in);
    mp_printf(print, "UART(%u, baudrate=%u, bits=%u, parity=%u, stop=%u)", self->index, self->baudrate, self->bitwidth,
              self->parity, self->stop);
}

STATIC void machine_uart_init_helper(machine_uart_obj_t* self, size_t n_args, const mp_obj_t* pos_args, mp_map_t* kw_args)
{
    enum { ARG_baudrate, ARG_bits, ARG_parity, ARG_stop, ARG_timeout, ARG_tx, ARG_rx };
    static const mp_arg_t allowed_args[] = {
        { MP_QSTR_baudrate, MP_ARG_INT, { .u_int = 115200 } },
        { MP_QSTR_bits, MP_ARG_INT, { .u_int = 8 } },
        { MP_QSTR_parity, MP_ARG_INT, { .u_int = PARITY_NONE } },
        { MP_QSTR_stop, MP_ARG_INT, { .u_int = STOP_BITS_1 } },
        { MP_QSTR_timeout, MP_ARG_KW_ONLY | MP_ARG_INT, { .u_int = 10 } },
        { MP_QSTR_tx, MP_ARG_KW_ONLY | MP_ARG_OBJ, { .u_obj = MP_OBJ_NULL } },
        { MP_QSTR_rx, MP_ARG_KW_ONLY | MP_ARG_OBJ, { .u_obj = MP_OBJ_NULL } },
    };
    mp_arg_val_t args[MP_ARRAY_SIZE(allowed_args)];
    mp_arg_parse_all(n_args, pos_args, kw_args, MP_ARRAY_SIZE(allowed_args), allowed_args, args);

    mp_int_t pin_tx = -1, pin_rx = -1;

    // Set SCL/SDA pins if given
    if (args[ARG_tx].u_obj != MP_OBJ_NULL) {
        pin_tx = mp_obj_get_int(args[ARG_tx].u_obj);
    }
    if (args[ARG_rx].u_obj != MP_OBJ_NULL) {
        pin_rx = mp_obj_get_int(args[ARG_rx].u_obj);
    }

    {
        int uart_id = self->index;

#define UART_TXD_FUNC(id)  ((id) == 0 ? UART0_TXD : \
                            (id) == 1 ? UART1_TXD : \
                            (id) == 2 ? UART2_TXD : \
                            (id) == 3 ? UART3_TXD : \
                            (id) == 4 ? UART4_TXD : -1)

#define UART_RXD_FUNC(id)  ((id) == 0 ? UART0_RXD : \
                            (id) == 1 ? UART1_RXD : \
                            (id) == 2 ? UART2_RXD : \
                            (id) == 3 ? UART3_RXD : \
                            (id) == 4 ? UART4_RXD : -1)

        fpioa_func_t func_tx = UART_TXD_FUNC(uart_id);
        fpioa_func_t func_rx = UART_RXD_FUNC(uart_id);

#undef UART_TXD_FUNC
#undef UART_RXD_FUNC

        // TX validation
        if (pin_tx != -1) {
            if (!drv_fpioa_is_func_supported_by_pin(pin_tx, func_tx)) {
                mp_raise_msg_varg(&mp_type_AssertionError, MP_ERROR_TEXT("Pin(%d) can not set to UART(%d) tx"), pin_tx, uart_id);
            }
            if(0x00 != drv_fpioa_set_pin_func(pin_tx, func_tx)) {
                mp_raise_msg_varg(&mp_type_RuntimeError, MP_ERROR_TEXT("set Pin(%d) to fpioa func %d failed"), pin_tx, func_tx);
            }
        } else {
            if (drv_fpioa_find_pin_by_func(func_tx) < 0) {
                mp_raise_msg_varg(&mp_type_AssertionError, MP_ERROR_TEXT("UART(%d) tx not configured, see machine.FPIOA"), uart_id);
            }
        }

        // RX validation
        if (pin_rx != -1) {
            if (!drv_fpioa_is_func_supported_by_pin(pin_rx, func_rx)) {
                mp_raise_msg_varg(&mp_type_AssertionError, MP_ERROR_TEXT("Pin(%d) can not set to UART(%d) rx"), pin_rx, uart_id);
            }
            if(0x00 != drv_fpioa_set_pin_func(pin_rx, func_rx)) {
                mp_raise_msg_varg(&mp_type_RuntimeError, MP_ERROR_TEXT("set Pin(%d) to fpioa func %d failed"), pin_rx, func_rx);
            }
        } else {
            if (drv_fpioa_find_pin_by_func(func_rx) < 0) {
                mp_raise_msg_varg(&mp_type_AssertionError, MP_ERROR_TEXT("UART(%d) rx not configured, see machine.FPIOA"), uart_id);
            }
        }
    }

    self->baudrate = args[ARG_baudrate].u_int;
    self->bitwidth = args[ARG_bits].u_int;
    self->parity   = args[ARG_parity].u_int;
    self->stop     = args[ARG_stop].u_int;

    if (args[ARG_timeout].u_int != -1) {
        self->timeout = args[ARG_timeout].u_int;
    }

    struct uart_configure cfg = {
        .baud_rate = self->baudrate,
        .data_bits = self->bitwidth,
        .stop_bits = self->stop,
        .parity    = self->parity,
        .bit_order = BIT_ORDER_LSB,
        .invert    = NRZ_NORMAL,
        .bufsz     = 0x400, // default
        .reserved  = 0,
    };

    if (0x00 != drv_uart_set_config(self->inst, &cfg)) {
        mp_printf(&mp_plat_print, "uart%d set config failed", self->index);
    }
}

STATIC mp_obj_t machine_uart_make_new(const mp_obj_type_t* type, size_t n_args, size_t n_kw, const mp_obj_t* args)
{
    mp_arg_check_num(n_args, n_kw, 1, MP_OBJ_FUN_ARGS_MAX, true);

    int index = mp_obj_get_int(args[0]);
    if (index < 0 || index >= KD_HARD_UART_MAX_NUM) {
        mp_raise_ValueError(MP_ERROR_TEXT("invalid UART index"));
    }

    drv_uart_inst_t* inst = NULL;
    if (drv_uart_inst_create(index, &inst) < 0) {
        mp_raise_msg_varg(&mp_type_OSError, MP_ERROR_TEXT("cannot create UART %u"), index);
    }

    machine_uart_obj_t* self = m_new_obj_with_finaliser(machine_uart_obj_t);
    self->base.type          = &machine_uart_type;
    self->index              = index;
    self->inst               = inst;
    self->status             = 1;
    self->timeout            = 0;

    mp_map_t kw_args;
    mp_map_init_fixed_table(&kw_args, n_kw, args + n_args);
    machine_uart_init_helper(self, n_args - 1, args + 1, &kw_args);

    self->status = 2;
    return MP_OBJ_FROM_PTR(self);
}

STATIC mp_obj_t machine_uart_deinit(mp_obj_t self_in)
{
    machine_uart_obj_t* self = MP_OBJ_TO_PTR(self_in);
    if (self->status == 0) {
        return mp_const_none;
    }

    drv_uart_inst_destroy(&self->inst);
    self->status = 0;

    return mp_const_none;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_1(machine_uart_deinit_obj, machine_uart_deinit);

STATIC mp_obj_t machine_uart_init(mp_uint_t n_args, const mp_obj_t* args, mp_map_t* kw_args)
{
    machine_uart_init_helper(args[0], n_args - 1, args + 1, kw_args);
    return mp_const_none;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_KW(machine_uart_init_obj, 1, machine_uart_init);

STATIC mp_obj_t machine_uart_any(mp_obj_t self_in)
{
    machine_uart_obj_t* self = MP_OBJ_TO_PTR(self_in);

    int ret = drv_uart_poll(self->inst, 0);

    return MP_OBJ_NEW_SMALL_INT(ret > 0 ? 1 : 0);
}
STATIC MP_DEFINE_CONST_FUN_OBJ_1(machine_uart_any_obj, machine_uart_any);

STATIC mp_obj_t machine_uart_sendbreak(mp_obj_t self_in)
{
    machine_uart_obj_t* self = MP_OBJ_TO_PTR(self_in);
    drv_uart_send_break(self->inst);
    return mp_const_none;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_1(machine_uart_sendbreak_obj, machine_uart_sendbreak);

STATIC mp_obj_t machine_uart_txdone(mp_obj_t self_in)
{
    mp_printf(&mp_plat_print, "K230 not support UART.txdone()\n");
    return mp_const_false;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_1(machine_uart_txdone_obj, machine_uart_txdone);

STATIC const mp_rom_map_elem_t machine_uart_locals_dict_table[] = {
    { MP_ROM_QSTR(MP_QSTR___del__), MP_ROM_PTR(&machine_uart_deinit_obj) },

    { MP_ROM_QSTR(MP_QSTR_init), MP_ROM_PTR(&machine_uart_init_obj) },
    { MP_ROM_QSTR(MP_QSTR_deinit), MP_ROM_PTR(&machine_uart_deinit_obj) },
    { MP_ROM_QSTR(MP_QSTR_any), MP_ROM_PTR(&machine_uart_any_obj) },
    { MP_ROM_QSTR(MP_QSTR_read), MP_ROM_PTR(&mp_stream_read_obj) },
    { MP_ROM_QSTR(MP_QSTR_readinto), MP_ROM_PTR(&mp_stream_readinto_obj) },
    { MP_ROM_QSTR(MP_QSTR_readline), MP_ROM_PTR(&mp_stream_unbuffered_readline_obj) },
    { MP_ROM_QSTR(MP_QSTR_write), MP_ROM_PTR(&mp_stream_write_obj) },
    { MP_ROM_QSTR(MP_QSTR_sendbreak), MP_ROM_PTR(&machine_uart_sendbreak_obj) },
    { MP_ROM_QSTR(MP_QSTR_flush), MP_ROM_PTR(&mp_stream_flush_obj) },
    { MP_ROM_QSTR(MP_QSTR_txdone), MP_ROM_PTR(&machine_uart_txdone_obj) },

    { MP_ROM_QSTR(MP_QSTR_UART1), MP_ROM_INT(1) },
    { MP_ROM_QSTR(MP_QSTR_UART2), MP_ROM_INT(2) },
    { MP_ROM_QSTR(MP_QSTR_UART3), MP_ROM_INT(3) },
    { MP_ROM_QSTR(MP_QSTR_UART4), MP_ROM_INT(4) },

    { MP_ROM_QSTR(MP_QSTR_FIVEBITS), MP_ROM_INT(DATA_BITS_5) },
    { MP_ROM_QSTR(MP_QSTR_SIXBITS), MP_ROM_INT(DATA_BITS_6) },
    { MP_ROM_QSTR(MP_QSTR_SEVENBITS), MP_ROM_INT(DATA_BITS_7) },
    { MP_ROM_QSTR(MP_QSTR_EIGHTBITS), MP_ROM_INT(DATA_BITS_8) },
    { MP_ROM_QSTR(MP_QSTR_NINEBITS), MP_ROM_INT(DATA_BITS_9) },

    { MP_ROM_QSTR(MP_QSTR_STOPBITS_ONE), MP_ROM_INT(STOP_BITS_1) },
    { MP_ROM_QSTR(MP_QSTR_STOPBITS_ONE_P_FIVE), MP_ROM_INT(STOP_BITS_1) },
    { MP_ROM_QSTR(MP_QSTR_STOPBITS_TWO), MP_ROM_INT(STOP_BITS_2) },

    { MP_ROM_QSTR(MP_QSTR_PARITY_NONE), MP_ROM_INT(PARITY_NONE) },
    { MP_ROM_QSTR(MP_QSTR_PARITY_ODD), MP_ROM_INT(PARITY_ODD) },
    { MP_ROM_QSTR(MP_QSTR_PARITY_EVEN), MP_ROM_INT(PARITY_EVEN) },
};
STATIC MP_DEFINE_CONST_DICT(machine_uart_locals_dict, machine_uart_locals_dict_table);

STATIC mp_uint_t machine_uart_read(mp_obj_t self_in, void* buf, mp_uint_t size, int* errcode)
{
    machine_uart_obj_t* self = MP_OBJ_TO_PTR(self_in);

    // If timeout is -1, treat as non-blocking read (timeout = 0)
    mp_uint_t effective_timeout = (self->timeout == (mp_uint_t)-1) ? 0 : self->timeout;

    size_t    read_bytes = 0;
    mp_uint_t start      = mp_hal_ticks_ms();
    for (;;) {
        int r = drv_uart_read(self->inst, (uint8_t*)buf + read_bytes, size - read_bytes);
        if (r > 0)
            read_bytes += r;
        if (read_bytes >= size || effective_timeout == 0)
            break;
        if (mp_hal_ticks_ms() - start >= effective_timeout)
            break;
        MICROPY_EVENT_POLL_HOOK
    }

    if (read_bytes == 0) {
        *errcode = MP_EAGAIN;
        return MP_STREAM_ERROR;
    }

    return read_bytes;
}

STATIC mp_uint_t machine_uart_write(mp_obj_t self_in, const void* buf, mp_uint_t size, int* errcode)
{
    machine_uart_obj_t* self = MP_OBJ_TO_PTR(self_in);

    int w = drv_uart_write(self->inst, (uint8_t*)buf, size);
    if (w < 0) {
        *errcode = MP_EAGAIN;
        return MP_STREAM_ERROR;
    }

    return w;
}

STATIC mp_uint_t machine_uart_ioctl(mp_obj_t self_in, mp_uint_t request, uintptr_t arg, int* errcode)
{
    machine_uart_obj_t* self = MP_OBJ_TO_PTR(self_in);

    *errcode      = 0;
    mp_uint_t ret = 0;

    switch (request) {
    case MP_STREAM_POLL: {
        mp_uint_t flags = arg;
        ret             = 0;
        if ((flags & MP_STREAM_POLL_RD) && drv_uart_recv_available(self->inst) > 0) {
            ret |= MP_STREAM_POLL_RD;
        }
        if ((flags & MP_STREAM_POLL_WR)) { // assume always writable
            ret |= MP_STREAM_POLL_WR;
        }
        return ret;
    }
    case MP_STREAM_FLUSH:
        // Not implemented
        return 0;
    default:
        *errcode = MP_EINVAL;
        return MP_STREAM_ERROR;
    }
}

STATIC const mp_stream_p_t uart_stream_p = {
    .read    = machine_uart_read,
    .write   = machine_uart_write,
    .ioctl   = machine_uart_ioctl,
    .is_text = false,
};

/* clang-format off */
MP_DEFINE_CONST_OBJ_TYPE(
    machine_uart_type,
    MP_QSTR_UART,
    MP_TYPE_FLAG_ITER_IS_STREAM,
    make_new, machine_uart_make_new,
    print, machine_uart_print,
    protocol, &uart_stream_p,
    locals_dict, &machine_uart_locals_dict
);
/* clang-format on */
