/* Copyright (c) 2023, Canaan Bright Sight Co., Ltd
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

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "py/obj.h"
#include "py/runtime.h"

#include "modmachine.h"

#include "drv_adc.h"

typedef struct {
    mp_obj_base_t base;

    int channel;
} machine_adc_obj_t;

STATIC void machine_adc_print(const mp_print_t* print, mp_obj_t self_in, mp_print_kind_t kind)
{
    machine_adc_obj_t* self = MP_OBJ_TO_PTR(self_in);
    mp_printf(print, "ADC: %d", self->channel);
}

STATIC mp_obj_t machine_adc_make_new(const mp_obj_type_t* type, size_t n_args, size_t n_kw, const mp_obj_t* args)
{
    mp_arg_check_num(n_args, n_kw, 1, 1, true);

    int channel = mp_obj_get_int(args[0]);
    if ((0 > channel) || (DRV_ADC_MAX_CHANNEL <= channel)) {
        mp_raise_msg_varg(&mp_type_ValueError, MP_ERROR_TEXT("invalid adc channel"));
    }

    if (0x00 != drv_adc_init()) {
        mp_raise_msg_varg(&mp_type_RuntimeError, MP_ERROR_TEXT("adc init failed"));
    }

    machine_adc_obj_t* self = m_new_obj_with_finaliser(machine_adc_obj_t);

    self->base.type = &machine_adc_type;
    self->channel   = channel;

    return MP_OBJ_FROM_PTR(self);
}

STATIC mp_obj_t machine_adc_destroy(mp_obj_t self_in)
{
    drv_adc_deinit();

    return mp_const_none;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_1(machine_adc_destroy_obj, machine_adc_destroy);

STATIC mp_obj_t machine_adc_read_u16(mp_obj_t self_in)
{
    machine_adc_obj_t* self = MP_OBJ_TO_PTR(self_in);

    uint32_t value = drv_adc_read(self->channel);

    return mp_obj_new_int_from_uint(value & 0xFFFF);
}
STATIC MP_DEFINE_CONST_FUN_OBJ_1(machine_adc_read_u16_obj, machine_adc_read_u16);

STATIC mp_obj_t machine_adc_read_uv(size_t n, const mp_obj_t* args)
{
    machine_adc_obj_t* self = MP_OBJ_TO_PTR(args[0]);

    uint32_t read, ref_uv = DRV_ADC_DEFAULT_REF_UV;

    if (0x02 <= n) {
        ref_uv = mp_obj_get_int(args[1]);
    }

    read = drv_adc_read_uv(self->channel, ref_uv);

    return mp_obj_new_int_from_uint(read);
}
STATIC MP_DEFINE_CONST_FUN_OBJ_VAR_BETWEEN(machine_adc_read_uv_obj, 1, 2, machine_adc_read_uv);

STATIC const mp_rom_map_elem_t machine_adc_locals_dict_table[] = {
    { MP_ROM_QSTR(MP_QSTR___del__), MP_ROM_PTR(&machine_adc_destroy_obj) },

    { MP_ROM_QSTR(MP_QSTR_read_u16), MP_ROM_PTR(&machine_adc_read_u16_obj) },
    { MP_ROM_QSTR(MP_QSTR_read_uv), MP_ROM_PTR(&machine_adc_read_uv_obj) },
};
STATIC MP_DEFINE_CONST_DICT(machine_adc_locals_dict, machine_adc_locals_dict_table);

/* clang-format off */
MP_DEFINE_CONST_OBJ_TYPE(
    machine_adc_type,
    MP_QSTR_ADC,
    MP_TYPE_FLAG_NONE,
    make_new, machine_adc_make_new,
    print, machine_adc_print,
    locals_dict, &machine_adc_locals_dict
);
/* clang-format on */
