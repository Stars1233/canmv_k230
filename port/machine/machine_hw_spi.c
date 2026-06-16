/*
 * This file is part of the MicroPython project, http://micropython.org/
 *
 * The MIT License (MIT)
 *
 * Copyright (c) 2016 Damien P. George
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

#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <sys/ioctl.h>

#include "mphal.h"
#include "mpprint.h"

#include "py/runtime.h"
#include "py/obj.h"

#include "extmod/machine_spi.h"

#include "modmachine.h"
#include "drv_spi.h"

typedef struct _machine_hw_spi_obj_t {
    mp_obj_base_t base;

    uint8_t index;
    uint8_t polarity;
    uint8_t phase;
    uint8_t bits;
    uint8_t data_line;

    int cs;

    enum {
        MACHINE_HW_SPI_STATE_NONE,
        MACHINE_HW_SPI_STATE_INIT,
        MACHINE_HW_SPI_STATE_DEINIT
    } state;

    uint32_t baudrate;
    drv_spi_inst_t spi_inst;
} machine_hw_spi_obj_t;

STATIC machine_hw_spi_obj_t machine_hw_spi_obj[3];

void machine_hw_spi_transfer(mp_obj_base_t *self_in, size_t len, const uint8_t *src, uint8_t *dest) {
    machine_hw_spi_obj_t *self = MP_OBJ_TO_PTR(self_in);

    if (MACHINE_HW_SPI_STATE_DEINIT == self->state) {
        mp_raise_msg(&mp_type_OSError, MP_ERROR_TEXT("transfer on deinitialized SPI"));
        return;
    }

    if (len != drv_spi_transfer(self->spi_inst, src, dest, len, 0)) {
        mp_printf(&mp_plat_print, "maybe spi transfer failed, %d\n", errno);
    }
}

/******************************************************************************/
// MicroPython bindings for hw_spi
STATIC void machine_hw_spi_print(const mp_print_t *print, mp_obj_t self_in, mp_print_kind_t kind) {
    machine_hw_spi_obj_t *self = MP_OBJ_TO_PTR(self_in);

    uint8_t cs_val = self->cs;

    mp_printf(print, "SPI(id=%u, baudrate=%u, polarity=%u, phase=%u, bits=%u, data_line=%u, cs=%u)",
        self->index, self->baudrate, self->polarity,
        self->phase, self->bits, self->data_line, cs_val);
}

STATIC void machine_hw_spi_deinit(mp_obj_base_t *self_in) {
    machine_hw_spi_obj_t *self = (machine_hw_spi_obj_t *)self_in;

    if (self->state == MACHINE_HW_SPI_STATE_INIT) {
        self->state = MACHINE_HW_SPI_STATE_DEINIT;
        drv_spi_inst_destroy(&self->spi_inst);
    }
}

STATIC void machine_hw_spi_init(mp_obj_base_t *self_in, size_t n_args, const mp_obj_t *pos_args, mp_map_t *kw_args) {
    machine_hw_spi_obj_t *self = (machine_hw_spi_obj_t *)self_in;
    machine_hw_spi_obj_t old_self = *self;

    enum { /* ARG_id, */ ARG_baudrate, ARG_polarity, ARG_phase, ARG_bits, ARG_cs, ARG_data_line, ARG_firstbit, ARG_sck, ARG_mosi, ARG_miso };
    static const mp_arg_t allowed_args[] = {
        // { MP_QSTR_id,           MP_ARG_REQUIRED | MP_ARG_INT, {.u_int = -1} },
        { MP_QSTR_baudrate,     MP_ARG_INT, {.u_int = 500000} },
        { MP_QSTR_polarity,     MP_ARG_KW_ONLY | MP_ARG_INT, {.u_int = 0} }, // 0 or 1
        { MP_QSTR_phase,        MP_ARG_KW_ONLY | MP_ARG_INT, {.u_int = 0} }, // 0 or 1
        { MP_QSTR_bits,         MP_ARG_KW_ONLY | MP_ARG_INT, {.u_int = 8} }, // 4 ~ 32
        // add new
        { MP_QSTR_cs,           MP_ARG_KW_ONLY | MP_ARG_INT, {.u_int = -1} }, // -1 not use driver cs control.
        { MP_QSTR_data_line,    MP_ARG_KW_ONLY | MP_ARG_INT, {.u_int = 1} }, // 1 2 4 8, 8 only valid on spi0
        // not support
        { MP_QSTR_firstbit,     MP_ARG_KW_ONLY | MP_ARG_INT, {.u_int = -1 /* MICROPY_PY_MACHINE_SPI_MSB */} },
        { MP_QSTR_sck,          MP_ARG_KW_ONLY | MP_ARG_OBJ, {.u_obj = MP_OBJ_NULL} },
        { MP_QSTR_mosi,         MP_ARG_KW_ONLY | MP_ARG_OBJ, {.u_obj = MP_OBJ_NULL} },
        { MP_QSTR_miso,         MP_ARG_KW_ONLY | MP_ARG_OBJ, {.u_obj = MP_OBJ_NULL} },
    };

    mp_arg_val_t args[MP_ARRAY_SIZE(allowed_args)];
    mp_arg_parse_all(n_args, pos_args, kw_args, MP_ARRAY_SIZE(allowed_args),
        allowed_args, args);

    bool changed = self->state != MACHINE_HW_SPI_STATE_INIT;

    // int8_t index = args[ARG_id].u_int;
    uint32_t baudrate = args[ARG_baudrate].u_int;
    int8_t polarity = args[ARG_polarity].u_int;
    int8_t phase = args[ARG_phase].u_int;
    int8_t bits = args[ARG_bits].u_int;
    int8_t cs = args[ARG_cs].u_int;
    int8_t data_line = args[ARG_data_line].u_int;

    // if((-1 == index) || (self->index != index)) {
    //     mp_raise_msg_varg(&mp_type_ValueError, MP_ERROR_TEXT("Invalid spi id(%d)"), spi_id);
    // }

    if(baudrate != self->baudrate) {
        changed = true;

        self->baudrate = baudrate;
    }

    if((polarity & 0x01) != self->polarity) {
        changed = true;

        self->polarity = polarity & 0x01;
    }

    if((phase & 0x01) != self->phase) {
        changed = true;

        self->phase = phase & 0x01;
    }

    if(bits != self->bits) {
        changed = true;

        if((4 > bits) || (32 < bits)) {
            mp_raise_msg_varg(&mp_type_ValueError, MP_ERROR_TEXT("Invalid spi bits(%d)"), bits);
        }

        self->bits = bits;
    }

    if(-1 != cs) {
        if((0 > cs) || (63 < cs)) {
            mp_raise_msg_varg(&mp_type_ValueError, MP_ERROR_TEXT("Invalid spi cs(%d)"), cs);
        }

        if(self->cs != cs) {
            changed = true;
            self->cs = cs;
        }
    }

    if(data_line != self->data_line) {
        changed = true;

#if 1
        if(0x01 == data_line) {
            // now, we support 1 line spi
        } else if((0x02 == data_line) || (0x04 == data_line) || (0x08 == data_line)) {
            mp_raise_msg_varg(&mp_type_ValueError, MP_ERROR_TEXT("Now, we only support 1line spi, not support (%d)"), data_line);
        } else {
            mp_raise_msg_varg(&mp_type_ValueError, MP_ERROR_TEXT("Invalid spi data_line(%d)"), data_line);
        }
#else
        if((0x01 == data_line) || (0x02 == data_line) || (0x04 == data_line) || ((0x00 == self->index) && (0x08 == data_line))) {
            // do nothings.
        } else {
            mp_raise_msg_varg(&mp_type_ValueError, MP_ERROR_TEXT("Invalid spi data_line(%d)"), data_line);
        }
#endif
        self->data_line = data_line;
    }

    if(changed) {
        if (self->state == MACHINE_HW_SPI_STATE_INIT) {
            self->state = MACHINE_HW_SPI_STATE_DEINIT;

            machine_hw_spi_deinit(&old_self.base);
        }
    } else {
        return;
    }

    if (0 != drv_spi_inst_create(self->index, 0, (polarity << 1) | (phase << 0), self->baudrate, bits, cs, data_line, &self->spi_inst)) {
        mp_raise_msg(&mp_type_RuntimeError, MP_ERROR_TEXT("create spi inst failed"));
    }

    self->state = MACHINE_HW_SPI_STATE_INIT;
}

mp_obj_t machine_hw_spi_make_new(const mp_obj_type_t *type, size_t n_args, size_t n_kw, const mp_obj_t *all_args) {
    mp_map_t kw_args;
    machine_hw_spi_obj_t *self;

    mp_arg_check_num(n_args, n_kw, 1, 2, true); // must set id, optional set baudrate

    mp_int_t spi_id = mp_obj_get_int(all_args[0]);

    if ((0 <= spi_id) && (2 >= spi_id)) {
        self = &machine_hw_spi_obj[spi_id];
    } else {
        mp_raise_msg_varg(&mp_type_ValueError, MP_ERROR_TEXT("SPI(%d) doesn't exist"), spi_id);
    }

    mp_map_init_fixed_table(&kw_args, n_kw, all_args + n_args);

    self->base.type = &machine_spi_type;
    self->index = spi_id;
    self->state = MACHINE_HW_SPI_STATE_NONE;

    machine_hw_spi_init(&self->base, n_args - 1, all_args + 1, &kw_args);

    return MP_OBJ_FROM_PTR(self);
}

STATIC mp_obj_t machine_spi_init(size_t n_args, const mp_obj_t *args, mp_map_t *kw_args) {
    mp_obj_base_t *s = (mp_obj_base_t *)MP_OBJ_TO_PTR(args[0]);
    mp_machine_spi_p_t *spi_p = (mp_machine_spi_p_t *)MP_OBJ_TYPE_GET_SLOT(s->type, protocol);
    spi_p->init(s, n_args - 1, args + 1, kw_args);
    return mp_const_none;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_KW(machine_spi_init_obj, 1, machine_spi_init);

STATIC mp_obj_t machine_spi_deinit(mp_obj_t self) {
    mp_obj_base_t *s = (mp_obj_base_t *)MP_OBJ_TO_PTR(self);
    mp_machine_spi_p_t *spi_p = (mp_machine_spi_p_t *)MP_OBJ_TYPE_GET_SLOT(s->type, protocol);
    if (spi_p->deinit != NULL) {
        spi_p->deinit(s);
    }
    return mp_const_none;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_1(machine_spi_deinit_obj, machine_spi_deinit);

STATIC mp_obj_t machine_spi_read(size_t n_args, const mp_obj_t *args) {
    vstr_t vstr;
    vstr_init_len(&vstr, mp_obj_get_int(args[1]));
    memset(vstr.buf, n_args == 3 ? mp_obj_get_int(args[2]) : 0, vstr.len);
    machine_hw_spi_transfer(args[0], vstr.len, NULL, (uint8_t *)vstr.buf);
    return mp_obj_new_bytes_from_vstr(&vstr);
}
MP_DEFINE_CONST_FUN_OBJ_VAR_BETWEEN(machine_spi_read_obj, 2, 3, machine_spi_read);

STATIC mp_obj_t machine_spi_readinto(size_t n_args, const mp_obj_t *args) {
    mp_buffer_info_t bufinfo;
    mp_get_buffer_raise(args[1], &bufinfo, MP_BUFFER_WRITE);
    memset(bufinfo.buf, n_args == 3 ? mp_obj_get_int(args[2]) : 0, bufinfo.len);
    machine_hw_spi_transfer(args[0], bufinfo.len, NULL, bufinfo.buf);
    return mp_const_none;
}
MP_DEFINE_CONST_FUN_OBJ_VAR_BETWEEN(machine_spi_readinto_obj, 2, 3, machine_spi_readinto);
//| # Auto-generated CanMV stub docs. Edit the signatures/docstrings here.
//| module: machine
//| class SPI:
//|     """machine.SPI object."""
//|     def __init__(self, spi_id: int, /, baudrate: int = 500000, *, polarity: int = 0, phase: int = 0, bits: int = 8, cs: int = -1, data_line: int = 1, firstbit: int = -1, sck: Any = ..., mosi: Any = ..., miso: Any = ...) -> None:
//|         """Create a machine.SPI object."""
//|     def deinit(self, /) -> None:
//|         """Release resources held by machine.SPI."""
//|     def init(self, baudrate: int = 500000, *, polarity: int = 0, phase: int = 0, bits: int = 8, cs: int = -1, data_line: int = 1, firstbit: int = -1, sck: Any = ..., mosi: Any = ..., miso: Any = ...) -> None:
//|         """Reconfigure the SPI bus."""
//|     def read(self, nbytes: int, write: int = ..., /) -> bytes:
//|         """Read data from machine.SPI."""
//|     def readinto(self, buf: Any, write: int = ..., /) -> int:
//|         """Read data from machine.SPI."""
//|     def write(self, wr_buf: Any, /) -> int:
//|         """Write or send data using machine.SPI."""
//|     def write_readinto(self, wr_buf: Any, rd_buf: Any, /) -> Any:
//|         """Write or send data using machine.SPI."""


STATIC const mp_rom_map_elem_t machine_spi_locals_dict_table_k230[] = {
    { MP_ROM_QSTR(MP_QSTR___del__), MP_ROM_PTR(&machine_spi_deinit_obj) },

    { MP_ROM_QSTR(MP_QSTR_init), MP_ROM_PTR(&machine_spi_init_obj) },
    { MP_ROM_QSTR(MP_QSTR_deinit), MP_ROM_PTR(&machine_spi_deinit_obj) },

    { MP_ROM_QSTR(MP_QSTR_read), MP_ROM_PTR(&machine_spi_read_obj) },
    { MP_ROM_QSTR(MP_QSTR_readinto), MP_ROM_PTR(&machine_spi_readinto_obj) },
    { MP_ROM_QSTR(MP_QSTR_write), MP_ROM_PTR(&mp_machine_spi_write_obj) },
    { MP_ROM_QSTR(MP_QSTR_write_readinto), MP_ROM_PTR(&mp_machine_spi_write_readinto_obj) },
};
MP_DEFINE_CONST_DICT(mp_machine_spi_locals_dict_k230, machine_spi_locals_dict_table_k230);

const mp_machine_spi_p_t machine_hw_spi_p = {
    .init = machine_hw_spi_init,
    .deinit = machine_hw_spi_deinit,
    .transfer = machine_hw_spi_transfer,
};

/* clang-format off */
MP_DEFINE_CONST_OBJ_TYPE(
    machine_spi_type,
    MP_QSTR_SPI,
    MP_TYPE_FLAG_NONE,
    make_new, machine_hw_spi_make_new,
    print, machine_hw_spi_print,
    protocol, &machine_hw_spi_p,
    locals_dict, &mp_machine_spi_locals_dict_k230
);
/* clang-format on */
