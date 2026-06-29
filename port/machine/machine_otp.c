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
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
 * BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
 * NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include <stdint.h>

#include "py/runtime.h"
#include "py/obj.h"

#include "drv_pufs.h"

#include "modmachine.h"

enum {
    MACHINE_OTP_WORD_SIZE = 4,
    MACHINE_OTP_BASE_LEN = 1024,
    MACHINE_OTP_TOTAL_LEN = 4096,
    MACHINE_OTP_MAX_READ_LEN = 1024,
};

typedef struct _machine_otp_obj_t {
    mp_obj_base_t base;
} machine_otp_obj_t;

static const machine_otp_obj_t machine_otp_obj = {{&machine_otp_type}};

static mp_int_t machine_otp_get_addr(mp_obj_t addr_obj) {
    mp_int_t addr = mp_obj_get_int(addr_obj);

    if (addr < 0 || addr >= MACHINE_OTP_TOTAL_LEN) {
        mp_raise_msg_varg(&mp_type_ValueError, MP_ERROR_TEXT("addr must be 0-%d"), MACHINE_OTP_TOTAL_LEN - 1);
    }
    if ((addr % MACHINE_OTP_WORD_SIZE) != 0) {
        mp_raise_msg_varg(&mp_type_ValueError, MP_ERROR_TEXT("addr must be %d-byte aligned"), MACHINE_OTP_WORD_SIZE);
    }

    return addr;
}

static mp_obj_t machine_otp_make_new(const mp_obj_type_t *type, size_t n_args, size_t n_kw, const mp_obj_t *args) {
    (void)type;
    mp_arg_check_num(n_args, n_kw, 0, 0, false);
    return MP_OBJ_FROM_PTR(&machine_otp_obj);
}

static mp_obj_t machine_otp_read(mp_obj_t self_in, mp_obj_t addr_obj, mp_obj_t len_obj) {
    (void)self_in;
    mp_int_t addr = machine_otp_get_addr(addr_obj);
    mp_int_t len = mp_obj_get_int(len_obj);

    if (len < 1 || len > MACHINE_OTP_MAX_READ_LEN) {
        mp_raise_msg_varg(&mp_type_ValueError, MP_ERROR_TEXT("length must be 1-%d"), MACHINE_OTP_MAX_READ_LEN);
    }
    if (len > MACHINE_OTP_TOTAL_LEN - addr) {
        mp_raise_msg_varg(&mp_type_ValueError, MP_ERROR_TEXT("read exceeds OTP range"));
    }
    if (addr < MACHINE_OTP_BASE_LEN && len > MACHINE_OTP_BASE_LEN - addr) {
        mp_raise_msg_varg(&mp_type_ValueError, MP_ERROR_TEXT("read crosses OTP region boundary"));
    }

    uint8_t data[MACHINE_OTP_MAX_READ_LEN];
    drv_pufs_inst dev;
    if (drv_pufs_open(&dev) != 0) {
        mp_raise_msg(&mp_type_RuntimeError, MP_ERROR_TEXT("open PUFS failed"));
    }

    int ret = drv_pufs_otp_read(&dev, (uint16_t)addr, data, (uint32_t)len);
    drv_pufs_close(&dev);
    if (ret != 0) {
        mp_raise_msg_varg(&mp_type_RuntimeError, MP_ERROR_TEXT("OTP read failed (%d)"), ret);
    }

    return mp_obj_new_bytes(data, (size_t)len);
}
static MP_DEFINE_CONST_FUN_OBJ_3(machine_otp_read_obj, machine_otp_read);

static mp_obj_t machine_otp_get_lock(mp_obj_t self_in, mp_obj_t addr_obj) {
    (void)self_in;
    mp_int_t addr = machine_otp_get_addr(addr_obj);
    uint8_t lock = OTP_SKIP;

    drv_pufs_inst dev;
    if (drv_pufs_open(&dev) != 0) {
        mp_raise_msg(&mp_type_RuntimeError, MP_ERROR_TEXT("open PUFS failed"));
    }

    int ret = drv_pufs_otp_get_rwlck(&dev, (uint16_t)addr, &lock);
    drv_pufs_close(&dev);
    if (ret != 0) {
        mp_raise_msg_varg(&mp_type_RuntimeError, MP_ERROR_TEXT("OTP lock query failed (%d)"), ret);
    }
    if (lock != OTP_NA && lock != OTP_RO && lock != OTP_RW) {
        mp_raise_msg_varg(&mp_type_RuntimeError, MP_ERROR_TEXT("invalid OTP lock state (%u)"), (unsigned int)lock);
    }

    return mp_obj_new_int(lock);
}
static MP_DEFINE_CONST_FUN_OBJ_2(machine_otp_get_lock_obj, machine_otp_get_lock);
//| # Auto-generated CanMV stub docs. Edit the signatures/docstrings here.
//| module: machine
//| class OTP:
//|     """machine.OTP singleton object."""
//|     NA: int
//|     RO: int
//|     RW: int
//|     def __init__(self) -> None:
//|         """Return the machine.OTP singleton."""
//|     def get_lock(self, addr: int, /) -> int:
//|         """Return the OTP lock state at a 4-byte-aligned address."""
//|     def read(self, addr: int, length: int, /) -> bytes:
//|         """Read bytes from OTP at a 4-byte-aligned address."""


static const mp_rom_map_elem_t machine_otp_locals_dict_table[] = {
    { MP_ROM_QSTR(MP_QSTR_NA), MP_ROM_INT(OTP_NA) },
    { MP_ROM_QSTR(MP_QSTR_RO), MP_ROM_INT(OTP_RO) },
    { MP_ROM_QSTR(MP_QSTR_RW), MP_ROM_INT(OTP_RW) },
    { MP_ROM_QSTR(MP_QSTR_read), MP_ROM_PTR(&machine_otp_read_obj) },
    { MP_ROM_QSTR(MP_QSTR_get_lock), MP_ROM_PTR(&machine_otp_get_lock_obj) },
};
static MP_DEFINE_CONST_DICT(machine_otp_locals_dict, machine_otp_locals_dict_table);

MP_DEFINE_CONST_OBJ_TYPE(
    machine_otp_type,
    MP_QSTR_OTP,
    MP_TYPE_FLAG_NONE,
    make_new, machine_otp_make_new,
    locals_dict, &machine_otp_locals_dict
    );
