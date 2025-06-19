/*
 * This file is part of the MicroPython project, http://micropython.org/
 *
 * The MIT License (MIT)
 *
 * Copyright (c) 2019 Damien P. George
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

#include "obj.h"
#include "py/mperrno.h"
#include "py/mphal.h"
#include "py/runtime.h"

#include "extmod/machine_i2c.h"
#include "misc.h"
#include "modmachine.h"

#include "py_assert.h" // use openmv marco, PY_ASSERT_TYPE

#include "drv_i2c.h"
#include <stdint.h>

#define I2C_DEFAULT_TIMEOUT_US (50000) // 50ms

typedef struct _machine_hw_i2c_obj_t {
    mp_obj_base_t   base;
    drv_i2c_inst_t* inst;
} machine_hw_i2c_obj_t;

STATIC machine_hw_i2c_obj_t machine_hw_i2c_obj[10]; /* 0-4: hardware i2c, 5-9: software i2c */

void* machine_i2c_obj_get_inst(mp_obj_t self_in)
{
    PY_ASSERT_TYPE(self_in, &machine_i2c_type);

    machine_hw_i2c_obj_t* self = MP_OBJ_TO_PTR(self_in);

    return self->inst;
}

int machine_i2c_read(mp_obj_base_t* self_in, uint8_t* dest, size_t len, bool nack)
{
    machine_hw_i2c_obj_t* self = MP_OBJ_TO_PTR(self_in);

    int fd = drv_i2c_master_get_fd(self->inst);

    if (0 > fd) {
        mp_raise_msg_varg(&mp_type_RuntimeError, MP_ERROR_TEXT("i2c state error"));
    }

    return read(fd, dest, len);
}

int mp_machine_i2c_write(mp_obj_base_t* self_in, const uint8_t* src, size_t len)
{
    machine_hw_i2c_obj_t* self = MP_OBJ_TO_PTR(self_in);

    int fd = drv_i2c_master_get_fd(self->inst);

    if (0 > fd) {
        mp_raise_msg_varg(&mp_type_RuntimeError, MP_ERROR_TEXT("i2c state error"));
    }

    return write(fd, src, len);
}

// return value:
//  >=0 - success; for read it's 0, for write it's number of acks received
//   <0 - error, with errno being the negative of the return value
int machine_i2c_transfer(mp_obj_base_t* self_in, uint16_t addr, size_t n, mp_machine_i2c_buf_t* bufs,
                         unsigned int flags)
{
    machine_hw_i2c_obj_t* self = MP_OBJ_TO_PTR(self_in);

    int msg_cnt   = n;
    int msg_flags = 0;
    int data_len = 0;

    uint8_t temp = 0x00;

    i2c_msg_t msgs[n];

    for (int i = 0; i < n; i++) {
        msgs[i].addr = addr;
        msgs[i].buf  = bufs->buf;
        msgs[i].len  = bufs->len;

        data_len += msgs[i].len;

        if ((flags & MP_MACHINE_I2C_FLAG_READ)) {
            msg_flags |= DRV_I2C_RD;
        } else {
            msg_flags |= DRV_I2C_WR;
        }
        if ((flags & MP_MACHINE_I2C_FLAG_STOP) != MP_MACHINE_I2C_FLAG_STOP) {
            msg_flags |= DRV_I2C_NO_STOP;
        }

        if (0x00 == msgs[i].len) {
            msgs[i].buf = &temp;
            msgs[i].len = 1;
        }

        msgs[i].flags = msg_flags;

        ++bufs;
    }

    if(0x00 == drv_i2c_transfer(self->inst, msgs, msg_cnt)) {
        return data_len;
    }

    // on error, we return -1.
    return -1;
}

/******************************************************************************/
// MicroPython bindings for machine API

STATIC void machine_i2c_print(const mp_print_t* print, mp_obj_t self_in, mp_print_kind_t kind)
{
    machine_hw_i2c_obj_t* self = MP_OBJ_TO_PTR(self_in);
    mp_printf(print, "I2C(%u, scl=%u, sda=%u, freq=%u, timeout=%u)", drv_i2c_master_get_id(self->inst),
              drv_i2c_master_get_pin_scl(self->inst), drv_i2c_master_get_pin_sda(self->inst),
              drv_i2c_master_get_timeout_ms(self->inst));
}

mp_obj_t machine_i2c_make_new(const mp_obj_type_t* type, size_t n_args, size_t n_kw, const mp_obj_t* all_args)
{
    // Parse args
    enum { ARG_id, ARG_scl, ARG_sda, ARG_freq, ARG_timeout };
    static const mp_arg_t allowed_args[] = {
        { MP_QSTR_id, MP_ARG_REQUIRED | MP_ARG_OBJ, { .u_obj = MP_OBJ_NULL } },
        { MP_QSTR_scl, MP_ARG_KW_ONLY | MP_ARG_OBJ, { .u_obj = MP_OBJ_NULL } },
        { MP_QSTR_sda, MP_ARG_KW_ONLY | MP_ARG_OBJ, { .u_obj = MP_OBJ_NULL } },
        { MP_QSTR_freq, MP_ARG_KW_ONLY | MP_ARG_INT, { .u_int = 400000 } },
        { MP_QSTR_timeout, MP_ARG_KW_ONLY | MP_ARG_INT, { .u_int = I2C_DEFAULT_TIMEOUT_US } },
    };
    mp_arg_val_t args[MP_ARRAY_SIZE(allowed_args)];
    mp_arg_parse_all_kw_array(n_args, n_kw, all_args, MP_ARRAY_SIZE(allowed_args), allowed_args, args);

    // Get I2C bus
    mp_int_t i2c_id = mp_obj_get_int(args[ARG_id].u_obj);
    if ((0 > i2c_id) || (9 < i2c_id)) {
        mp_raise_msg_varg(&mp_type_ValueError, MP_ERROR_TEXT("I2C(%d) doesn't exist"), i2c_id);
    }

    mp_int_t pin_scl = 0xFF, pin_sda = 0xFF;
    mp_int_t freq       = args[ARG_freq].u_int;
    mp_int_t timeout_ms = args[ARG_timeout].u_int / 1000;

    // Set SCL/SDA pins if given
    if (args[ARG_scl].u_obj != MP_OBJ_NULL) {
        pin_scl = mp_obj_get_int(args[ARG_scl].u_obj);
    }
    if (args[ARG_sda].u_obj != MP_OBJ_NULL) {
        pin_sda = mp_obj_get_int(args[ARG_sda].u_obj);
    }

    // Get static peripheral object
    machine_hw_i2c_obj_t* self = (machine_hw_i2c_obj_t*)&machine_hw_i2c_obj[i2c_id];

    if ((5 <= i2c_id) && (self->inst)) {
        /* soft i2c, check pin is same? */
        if ((pin_scl != drv_i2c_master_get_pin_scl(self->inst))
            || (pin_sda != drv_i2c_master_get_pin_sda(self->inst))) {
            drv_i2c_inst_destroy(&self->inst);
        }
    }

    if ((NULL == self->base.type) || (NULL == self->inst)) {
        self->base.type = &machine_i2c_type;

        self->inst = NULL;
        if (0x00 != drv_i2c_inst_create(i2c_id, freq, timeout_ms, pin_scl, pin_sda, &self->inst)) {
            mp_raise_msg(&mp_type_RuntimeError, MP_ERROR_TEXT("create i2c obj failed"));
        }
    } else {
        drv_i2c_set_freq(self->inst, freq);
        drv_i2c_set_timeout(self->inst, timeout_ms);
    }

    drv_i2c_set_7b_addr(self->inst);

    return MP_OBJ_FROM_PTR(self);
}

STATIC const mp_machine_i2c_p_t machine_i2c_p = {
#if MICROPY_PY_MACHINE_I2C_TRANSFER_WRITE1
    .transfer_supports_write1 = true,
#endif
    .init            = NULL,
    .start           = NULL,
    .stop            = NULL,
    .read            = machine_i2c_read,
    .write           = mp_machine_i2c_write,
    .transfer        = machine_i2c_transfer,
    .transfer_single = NULL,
};

/* clang-format off */
MP_DEFINE_CONST_OBJ_TYPE(
    machine_i2c_type,
    MP_QSTR_I2C,
    MP_TYPE_FLAG_NONE,
    make_new, machine_i2c_make_new,
    print, machine_i2c_print,
    protocol, &machine_i2c_p,
    locals_dict, &mp_machine_i2c_locals_dict
);
/* clang-format on */
