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

#include "py/mpprint.h"
#include "py/obj.h"
#include "py/runtime.h"

#include "drv_rotary_encoder.h"

#include "modmachine.h"

///////////////////////////////////////////////////////////////////////////////
// Encoder Data Wrap //////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
static const mp_obj_type_t py_encoder_data_type;

typedef struct _py_encoder_data_obj_t {
    mp_obj_base_t       base;
    struct encoder_data cobj;
} py_encoder_data_obj_t;

/* constructor from an existing C struct */
static mp_obj_t py_encoder_data_from_struct(const struct encoder_data* src)
{
    py_encoder_data_obj_t* o = m_new_obj(py_encoder_data_obj_t);
    o->base.type             = &py_encoder_data_type;
    o->cobj                  = *src;
    return MP_OBJ_FROM_PTR(o);
}

/* attribute reader */
static void py_encoder_data_attr(mp_obj_t self_in, qstr attr, mp_obj_t* dest)
{
    py_encoder_data_obj_t* self = MP_OBJ_TO_PTR(self_in);
    struct encoder_data*   e    = &self->cobj;

    if (dest[0] == MP_OBJ_NULL) { /* load attribute */
        switch (attr) {
        case MP_QSTR_delta:
            dest[0] = mp_obj_new_int(e->delta);
            break;
        case MP_QSTR_total_count:
            dest[0] = mp_obj_new_int_from_ll(e->total_count);
            break;
        case MP_QSTR_direction:
            dest[0] = MP_OBJ_NEW_SMALL_INT(e->direction);
            break;
        case MP_QSTR_button_state:
            dest[0] = MP_OBJ_NEW_SMALL_INT(e->button_state);
            break;
        case MP_QSTR_timestamp:
            dest[0] = mp_obj_new_int_from_uint(e->timestamp);
            break;
        default:
            dest[1] = MP_OBJ_SENTINEL; /* continue lookup */
        }
    }
}

/* print helper (optional) */
static void py_encoder_data_print(const mp_print_t* print, mp_obj_t self_in, mp_print_kind_t kind)
{
    const char*            dir[3] = { "none", "cw", "ccw" };
    py_encoder_data_obj_t* self   = MP_OBJ_TO_PTR(self_in);
    struct encoder_data*   e      = &self->cobj;

    mp_printf(print, "{delta:%d, total_count:%d, direction:%s, button_state:%u, timestamp:%u}", (int)e->delta,
              (long long)e->total_count, dir[e->direction % 3], (unsigned)e->button_state, (unsigned)e->timestamp);
}

/* clang-format off */
static MP_DEFINE_CONST_OBJ_TYPE(
    py_encoder_data_type,
    MP_QSTR_encoder_data,
    MP_TYPE_FLAG_NONE,
    print, py_encoder_data_print,
    attr,  py_encoder_data_attr
);
/* clang-format on */

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
typedef struct {
    mp_obj_base_t base;

    struct encoder_dev_inst_t* inst;
} machine_encoder_obj_t;

static void mp_machine_encoder_print(const mp_print_t* print, mp_obj_t self_in, mp_print_kind_t kind)
{
    int                      index;
    struct encoder_pin_cfg_t cfg;

    machine_encoder_obj_t* self = MP_OBJ_TO_PTR(self_in);

    rotary_encoder_get_index(self->inst, &index);
    rotary_encoder_get_pin_cfg(self->inst, &cfg);

    mp_printf(print, "RotaryEncoder %u: pin_clk=%d, pin_dt=%d, pin_sw=%d", index, cfg.clk_pin, cfg.dt_pin, cfg.sw_pin);
}

static mp_obj_t mp_machine_encoder_make_new(const mp_obj_type_t* type_in, size_t n_args, size_t n_kw, const mp_obj_t* all_args)
{
    enum { ARG_id, ARG_pin_clk, ARG_pin_dt, ARG_pin_sw };
    const mp_arg_t allowed_args[] = {
        { MP_QSTR_id, MP_ARG_INT | MP_ARG_REQUIRED, { .u_int = 0 } },
        { MP_QSTR_pin_clk, MP_ARG_INT | MP_ARG_REQUIRED, { .u_int = -1 } },
        { MP_QSTR_pin_dt, MP_ARG_INT | MP_ARG_REQUIRED, { .u_int = -1 } },
        { MP_QSTR_pin_sw, MP_ARG_INT, { .u_int = -1 } },
    };

    mp_arg_val_t args[MP_ARRAY_SIZE(allowed_args)];
    mp_arg_parse_all_kw_array(n_args, n_kw, all_args, MP_ARRAY_SIZE(allowed_args), allowed_args, args);

    int                        index;
    struct encoder_pin_cfg_t   cfg;
    struct encoder_dev_inst_t* inst = NULL;

    index       = args[ARG_id].u_int;
    cfg.clk_pin = args[ARG_pin_clk].u_int;
    cfg.dt_pin  = args[ARG_pin_dt].u_int;
    cfg.sw_pin  = args[ARG_pin_sw].u_int;

    if ((0 > cfg.clk_pin) || (0 > cfg.dt_pin)) {
        mp_raise_msg(&mp_type_ValueError, MP_ERROR_TEXT("Invalid clk or dt pin"));
    }

    if (0x00 != rotary_encoder_inst_create(&inst, index, &cfg)) {
        mp_raise_msg(&mp_type_RuntimeError, MP_ERROR_TEXT("Create encoder device failed"));
    }

    machine_encoder_obj_t* self = m_new_obj_with_finaliser(machine_encoder_obj_t);
    self->base.type             = &machine_encoder_type;
    self->inst                  = inst;

    return MP_OBJ_FROM_PTR(self);
}

static mp_obj_t machine_encoder_destroy(mp_obj_t self_in)
{
    machine_encoder_obj_t* self = MP_OBJ_TO_PTR(self_in);

    rotary_encoder_inst_destroy(&self->inst);

    return mp_const_none;
}
static MP_DEFINE_CONST_FUN_OBJ_1(machine_encoder_destroy_obj, machine_encoder_destroy);

static mp_obj_t machine_encoder_read(size_t n_args, const mp_obj_t* args)
{
    int timeout_ms = 0;

    struct encoder_data    data;
    machine_encoder_obj_t* self = MP_OBJ_TO_PTR(args[0]);

    if (2 <= n_args) {
        timeout_ms = mp_obj_get_int(args[1]);
    }

    if (0x00 != rotary_encoder_wait_event(self->inst, &data, timeout_ms)) {
        return mp_const_none;
    }

    return py_encoder_data_from_struct(&data);
}
MP_DEFINE_CONST_FUN_OBJ_VAR_BETWEEN(machine_encoder_read_obj, 1, 2, machine_encoder_read);

static mp_obj_t machine_encoder_reset(mp_obj_t self_in)
{
    machine_encoder_obj_t* self = MP_OBJ_TO_PTR(self_in);

    rotary_encoder_reset(self->inst);

    return mp_const_none;
}
static MP_DEFINE_CONST_FUN_OBJ_1(machine_encoder_reset_obj, machine_encoder_reset);

static const mp_rom_map_elem_t machine_encoder_locals_dict_table[] = {
    { MP_ROM_QSTR(MP_QSTR___del__), MP_ROM_PTR(&machine_encoder_destroy_obj) },
    { MP_ROM_QSTR(MP_QSTR_read), MP_ROM_PTR(&machine_encoder_read_obj) },
    { MP_ROM_QSTR(MP_QSTR_reset), MP_ROM_PTR(&machine_encoder_reset_obj) },

    { MP_ROM_QSTR(MP_QSTR_DIR_CW), MP_ROM_INT(ENCODER_DIR_CW) },
    { MP_ROM_QSTR(MP_QSTR_DIR_CCW), MP_ROM_INT(ENCODER_DIR_CCW) },
    { MP_ROM_QSTR(MP_QSTR_DIR_NONE), MP_ROM_INT(ENCODER_DIR_NONE) },
};
static MP_DEFINE_CONST_DICT(machine_encoder_locals_dict, machine_encoder_locals_dict_table);

/* clang-format off */
MP_DEFINE_CONST_OBJ_TYPE(
    machine_encoder_type,
    MP_QSTR_ENCODER,
    MP_TYPE_FLAG_NONE,
    make_new, mp_machine_encoder_make_new,
    print, mp_machine_encoder_print,
    locals_dict, &machine_encoder_locals_dict
);
/* clang-format on */
