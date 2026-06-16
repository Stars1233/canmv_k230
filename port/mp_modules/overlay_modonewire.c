/*
 * This file is part of the MicroPython project, http://micropython.org/
 *
 * The MIT License (MIT)
 *
 * Copyright (c) 2015-2017 Damien P. George
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

#include <stdio.h>
#include <stdint.h>
#include <string.h>

#include "py/obj.h"
#include "py/mphal.h"

#include "mpprint.h"
#include "py/runtime.h"

#if MICROPY_PY_ONEWIRE

#include "onewire.h"

/******************************************************************************/
// MicroPython bindings

STATIC mp_obj_t py_onewire_reset(mp_obj_t pin_in) {
    mp_hal_pin_obj_t pin = mp_hal_get_pin_obj(pin_in);

    return mp_obj_new_bool(onewire_reset(mp_hal_pin_name(pin)));
}
STATIC MP_DEFINE_CONST_FUN_OBJ_1(onewire_reset_obj, py_onewire_reset);

STATIC mp_obj_t onewire_readbit(mp_obj_t pin_in) {
    mp_printf(&mp_plat_print, "Please do not call readbit, the timing cannot be guaranteed");

    return 0; // MP_OBJ_NEW_SMALL_INT(onewire_bus_readbit(mp_hal_get_pin_obj(pin_in)));
}
STATIC MP_DEFINE_CONST_FUN_OBJ_1(onewire_readbit_obj, onewire_readbit);

STATIC mp_obj_t py_onewire_readbyte(mp_obj_t pin_in) {
    mp_hal_pin_obj_t pin = mp_hal_get_pin_obj(pin_in);
    uint8_t value = 0;

    value = onewire_read_byte(mp_hal_pin_name(pin));

    return MP_OBJ_NEW_SMALL_INT(value);
}
STATIC MP_DEFINE_CONST_FUN_OBJ_1(onewire_readbyte_obj, py_onewire_readbyte);

STATIC mp_obj_t onewire_writebit(mp_obj_t pin_in, mp_obj_t value_in) {
    mp_printf(&mp_plat_print, "Please do not call writebit, the timing cannot be guaranteed");

    // onewire_bus_writebit(mp_hal_get_pin_obj(pin_in), mp_obj_get_int(value_in));
    return mp_const_none;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_2(onewire_writebit_obj, onewire_writebit);

STATIC mp_obj_t py_onewire_writebyte(mp_obj_t pin_in, mp_obj_t value_in) {
    mp_hal_pin_obj_t pin = mp_hal_get_pin_obj(pin_in);
    int value = mp_obj_get_int(value_in);

    onewire_write_byte(mp_hal_pin_name(pin), (uint8_t)(value & 0xFF));

    return mp_const_none;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_2(onewire_writebyte_obj, py_onewire_writebyte);

STATIC mp_obj_t py_onewire_search_rom(mp_obj_t pin_in, mp_obj_t l_rom_in, mp_obj_t diff_in)
{
    uint8_t rom[8], l_rom[8];

    mp_hal_pin_obj_t pin  = mp_hal_get_pin_obj(pin_in);
    int              diff = mp_obj_get_int(diff_in);

    if (mp_obj_equal(l_rom_in, mp_const_false)) {
        memset(l_rom, 0, 8);
    } else {
        mp_buffer_info_t bufinfo;
        mp_get_buffer_raise(l_rom_in, &bufinfo, MP_BUFFER_READ);

        if (bufinfo.len < 8) {
            mp_raise_ValueError(MP_ERROR_TEXT("buffer too small"));
        }
        memcpy(l_rom, bufinfo.buf, 8);
    }

    if (0x00 == onewire_search_rom(mp_hal_pin_name(pin), rom, l_rom, &diff)) {
        mp_obj_t ba = mp_obj_new_bytearray(8, rom);
        return mp_obj_new_tuple(2, ((mp_obj_t[]) { ba, mp_obj_new_int(diff) }));
    } else {
        return mp_obj_new_tuple(2, ((mp_obj_t[]) { mp_const_none, mp_obj_new_int(0) }));
    }
}
STATIC MP_DEFINE_CONST_FUN_OBJ_3(onewire_search_rom_obj, py_onewire_search_rom);

STATIC mp_obj_t onewire_crc8(mp_obj_t data) {
    mp_buffer_info_t bufinfo;
    mp_get_buffer_raise(data, &bufinfo, MP_BUFFER_READ);
    uint8_t crc = 0;
    for (size_t i = 0; i < bufinfo.len; ++i) {
        uint8_t byte = ((uint8_t *)bufinfo.buf)[i];
        for (int b = 0; b < 8; ++b) {
            uint8_t fb_bit = (crc ^ byte) & 0x01;
            if (fb_bit == 0x01) {
                crc = crc ^ 0x18;
            }
            crc = (crc >> 1) & 0x7f;
            if (fb_bit == 0x01) {
                crc = crc | 0x80;
            }
            byte = byte >> 1;
        }
    }
    return MP_OBJ_NEW_SMALL_INT(crc);
}
STATIC MP_DEFINE_CONST_FUN_OBJ_1(onewire_crc8_obj, onewire_crc8);
//| # Auto-generated CanMV stub docs. Edit the signatures/docstrings here.
//| module: _onewire
//| """CanMV _onewire module."""
//| def crc8(data: Any, /) -> Any:
//|     """Perform crc8 for _onewire."""
//| def readbit(pin: Any, /) -> Any:
//|     """Read data from _onewire."""
//| def readbyte(pin: Any, /) -> Any:
//|     """Read data from _onewire."""
//| def reset(pin: Any, /) -> None:
//|     """Reset _onewire."""
//| def search_rom(pin: Any, l_rom: Any, diff: Any, /) -> Any:
//|     """Perform search rom for _onewire."""
//| def writebit(pin: Any, value: Any, /) -> Any:
//|     """Write or send data using _onewire."""
//| def writebyte(pin: Any, value: Any, /) -> Any:
//|     """Write or send data using _onewire."""


STATIC const mp_rom_map_elem_t onewire_module_globals_table[] = {
    { MP_ROM_QSTR(MP_QSTR___name__), MP_ROM_QSTR(MP_QSTR_onewire) },

    { MP_ROM_QSTR(MP_QSTR_reset), MP_ROM_PTR(&onewire_reset_obj) },
    { MP_ROM_QSTR(MP_QSTR_readbit), MP_ROM_PTR(&onewire_readbit_obj) },
    { MP_ROM_QSTR(MP_QSTR_readbyte), MP_ROM_PTR(&onewire_readbyte_obj) },
    { MP_ROM_QSTR(MP_QSTR_writebit), MP_ROM_PTR(&onewire_writebit_obj) },
    { MP_ROM_QSTR(MP_QSTR_writebyte), MP_ROM_PTR(&onewire_writebyte_obj) },
    { MP_ROM_QSTR(MP_QSTR_search_rom), MP_ROM_PTR(&onewire_search_rom_obj) },
    { MP_ROM_QSTR(MP_QSTR_crc8), MP_ROM_PTR(&onewire_crc8_obj) },
};

STATIC MP_DEFINE_CONST_DICT(onewire_module_globals, onewire_module_globals_table);

const mp_obj_module_t mp_module_onewire = {
    .base = { &mp_type_module },
    .globals = (mp_obj_dict_t *)&onewire_module_globals,
};

MP_REGISTER_MODULE(MP_QSTR__onewire, mp_module_onewire);

#endif // MICROPY_PY_ONEWIRE

// dht11 helper, we make it in onewire driver.
mp_uint_t machine_time_pulse_us(mp_hal_pin_obj_t pin, int pulse_level, mp_uint_t timeout_us) {
    return pin_pulse_us(mp_hal_pin_name(pin), pulse_level, timeout_us);
}
