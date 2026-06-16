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

#include <stdbool.h>
#include <stdint.h>

#include "py/obj.h"
#include "py/runtime.h"

#include "modmachine.h"

#include "drv_wdt.h"

typedef struct _machine_wdt_obj_t {
    mp_obj_base_t base;
    uint32_t      timeout;
    int volatile active;
} machine_wdt_obj_t;

static bool volatile machine_wdt_in_use = false;

static void machine_wdt_release(machine_wdt_obj_t* self)
{
    if (!self->active) {
        return;
    }

    wdt_close();
    self->active       = 0;
    machine_wdt_in_use = false;
}

static void machine_wdt_ensure_active(machine_wdt_obj_t* self)
{
    if (!self->active) {
        mp_raise_msg(&mp_type_OSError, MP_ERROR_TEXT("watchdog is closed"));
    }
}

static void mp_machine_wdt_print(const mp_print_t* print, mp_obj_t self_in, mp_print_kind_t kind)
{
    machine_wdt_obj_t* self = self_in;
    mp_printf(print, "WDT: timeout=%ds", self->timeout);
}

static mp_obj_t mp_machine_wdt_make_new(const mp_obj_type_t* type_in, size_t n_args, size_t n_kw, const mp_obj_t* all_args)
{
    enum { ARG_id, ARG_timeout, ARG_auto_close };
    const mp_arg_t allowed_args[] = {
        { MP_QSTR_id, MP_ARG_INT, { .u_int = 1 } },
        { MP_QSTR_timeout, MP_ARG_INT, { .u_int = 5 } },
        { MP_QSTR_auto_close, MP_ARG_BOOL | MP_ARG_KW_ONLY, { .u_bool = true } },
    };

    mp_arg_val_t args[MP_ARRAY_SIZE(allowed_args)];
    mp_arg_parse_all_kw_array(n_args, n_kw, all_args, MP_ARRAY_SIZE(allowed_args), allowed_args, args);

    (void)args[ARG_auto_close];

    if (args[ARG_timeout].u_int <= 0) {
        mp_raise_ValueError(MP_ERROR_TEXT("WDT timeout too short"));
    }

    if (machine_wdt_in_use) {
        mp_raise_ValueError(MP_ERROR_TEXT("WDT is already in use"));
    }

    machine_wdt_obj_t* self = m_new_obj_with_finaliser(machine_wdt_obj_t);

    self->base.type = &machine_wdt_type;
    self->timeout   = args[ARG_timeout].u_int;
    self->active    = 0;

    if (wdt_set_timeout(self->timeout) != 0) {
        goto err;
    }

    if (wdt_start() != 0) {
        goto err;
    }

    {
        uint32_t actual_timeout = wdt_get_timeout();

        if (actual_timeout != (uint32_t)-1) {
            self->timeout = actual_timeout;
        }
    }

    self->active       = 1;
    machine_wdt_in_use = true;

    return self;

err:
    machine_wdt_in_use = false;
    wdt_close();

    mp_raise_msg_varg(&mp_type_OSError, MP_ERROR_TEXT("wdt init error"));

    return mp_const_none;
}

static mp_obj_t machine_wdt_feed(mp_obj_t self_in)
{
    machine_wdt_obj_t* self = MP_OBJ_TO_PTR(self_in);

    machine_wdt_ensure_active(self);

    if (wdt_feed() != 0) {
        mp_raise_msg_varg(&mp_type_OSError, MP_ERROR_TEXT("feed watchdog error"));
    }

    return mp_const_none;
}
static MP_DEFINE_CONST_FUN_OBJ_1(machine_wdt_feed_obj, machine_wdt_feed);

static mp_obj_t machine_wdt_close(mp_obj_t self_in)
{
    machine_wdt_obj_t* self = MP_OBJ_TO_PTR(self_in);

    machine_wdt_release(self);

    return mp_const_none;
}
static MP_DEFINE_CONST_FUN_OBJ_1(machine_wdt_close_obj, machine_wdt_close);
//| # Auto-generated CanMV stub docs. Edit the signatures/docstrings here.
//| module: machine
//| class WDT:
//|     """machine.WDT object."""
//|     def __init__(self, id: int = 1, timeout: int = 5, *, auto_close: bool = True) -> None:
//|         """Create a machine.WDT object."""
//|     def close(self, /) -> None:
//|         """Release resources held by machine.WDT."""
//|     def feed(self, /) -> Any:
//|         """Feed machine.WDT."""
//|     def stop(self, /) -> Any:
//|         """Stop machine.WDT."""


static const mp_rom_map_elem_t machine_wdt_locals_dict_table[] = {
    { MP_ROM_QSTR(MP_QSTR_feed), MP_ROM_PTR(&machine_wdt_feed_obj) },

    { MP_ROM_QSTR(MP_QSTR___del__), MP_ROM_PTR(&machine_wdt_close_obj) },
    { MP_ROM_QSTR(MP_QSTR_close), MP_ROM_PTR(&machine_wdt_close_obj) },
    { MP_ROM_QSTR(MP_QSTR_stop), MP_ROM_PTR(&machine_wdt_close_obj) },
};
static MP_DEFINE_CONST_DICT(machine_wdt_locals_dict, machine_wdt_locals_dict_table);

/* clang-format off */
MP_DEFINE_CONST_OBJ_TYPE(
    machine_wdt_type,
    MP_QSTR_WDT,
    MP_TYPE_FLAG_NONE,
    make_new, mp_machine_wdt_make_new,
    print, mp_machine_wdt_print,
    locals_dict, &machine_wdt_locals_dict
);
/* clang-format on */
