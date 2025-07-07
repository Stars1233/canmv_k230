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

/******************************************************************************/
// Low-level 1-Wire routines

#define TIMING_RESET1 (480)
#define TIMING_RESET2 (70)
#define TIMING_RESET3 (410)
#define TIMING_READ1  (5)
#define TIMING_READ2  (5)
#define TIMING_READ3  (40)
#define TIMING_WRITE1 (10)
#define TIMING_WRITE2 (50)
#define TIMING_WRITE3 (10)

STATIC int onewire_bus_reset(mp_hal_pin_obj_t pin) {
    mp_hal_pin_open_drain(pin);

    mp_hal_pin_od_low(pin);
    mp_hal_delay_us_fast(TIMING_RESET1);
    uint32_t i = mp_hal_quiet_timing_enter();
    mp_hal_pin_od_high(pin);
    mp_hal_delay_us_fast(TIMING_RESET2);
    int status = !mp_hal_pin_read(pin);
    mp_hal_quiet_timing_exit(i);
    mp_hal_delay_us_fast(TIMING_RESET3);
    return status;
}

STATIC int onewire_bus_readbit(mp_hal_pin_obj_t pin) {
    mp_hal_pin_od_high(pin);
    uint32_t i = mp_hal_quiet_timing_enter();
    mp_hal_pin_od_low(pin);
    mp_hal_delay_us_fast(TIMING_READ1);
    mp_hal_pin_od_high(pin);
    mp_hal_delay_us_fast(TIMING_READ2);
    int value = mp_hal_pin_read(pin);
    mp_hal_quiet_timing_exit(i);
    mp_hal_delay_us_fast(TIMING_READ3);
    return value;
}

STATIC void onewire_bus_writebit(mp_hal_pin_obj_t pin, int value) {
    uint32_t i = mp_hal_quiet_timing_enter();
    mp_hal_pin_od_low(pin);
    mp_hal_delay_us_fast(TIMING_WRITE1);
    if (value) {
        mp_hal_pin_od_high(pin);
    }
    mp_hal_delay_us_fast(TIMING_WRITE2);
    mp_hal_pin_od_high(pin);
    mp_hal_delay_us_fast(TIMING_WRITE3);
    mp_hal_quiet_timing_exit(i);
}

static int onewire_bus_search_rom(mp_hal_pin_obj_t pin, uint8_t rom[8], uint8_t l_rom[8], int* diff_in)
{
    if (0x00 == onewire_bus_reset(pin)) {
        return -1;
    }
    mp_hal_delay_us_fast(1);

    /* write byte 0xF0 */
    mp_hal_pin_od_high(pin);
    for (int i = 0; i < 8; ++i) {
        onewire_bus_writebit(pin, (0xF0 >> i) & 1); // SEARCH_ROM
    }

    int i         = 64;
    int next_diff = 0;
    int diff      = *diff_in;

    for (int byte = 0; byte < 8; byte++) {
        uint8_t r_b = 0;

        for (int bit = 0; bit < 8; bit++) {
            int b1 = onewire_bus_readbit(pin);
            int b2 = onewire_bus_readbit(pin);

            if (b2) {
                if (b1) {
                    // there are no devices or there is an error on the bus
                    return -1;
                }
            } else {
                if (0x00 == b1) {
                    // collision, two devices with different bit meaning
                    if (diff > i || ((l_rom[byte] & (1 << bit)) && (diff != i))) {
                        b1        = 1;
                        next_diff = i;
                    }
                }
            }

            mp_hal_pin_od_high(pin);
            onewire_bus_writebit(pin, b1);

            if (b1) {
                r_b |= (1 << bit);
            }
            i -= 1;
        }
        rom[byte] = r_b;
    }

    *diff_in = next_diff;

    return 0;
}

STATIC int onewire_bus_readbit_without_lock(mp_hal_pin_obj_t pin) {
    mp_hal_pin_od_high(pin);
    mp_hal_delay_us_fast(1);

    mp_hal_pin_od_low(pin);
    mp_hal_delay_us_fast(TIMING_READ1);
    mp_hal_pin_od_high(pin);
    mp_hal_delay_us_fast(TIMING_READ2);
    int value = mp_hal_pin_read(pin);
    mp_hal_delay_us_fast(TIMING_READ3);

    return value;
}

/******************************************************************************/
// MicroPython bindings

STATIC mp_obj_t onewire_reset(mp_obj_t pin_in) {
    return mp_obj_new_bool(onewire_bus_reset(mp_hal_get_pin_obj(pin_in)));
}
STATIC MP_DEFINE_CONST_FUN_OBJ_1(onewire_reset_obj, onewire_reset);

STATIC mp_obj_t onewire_readbit(mp_obj_t pin_in) {
    mp_printf(&mp_plat_print, "Please do not call readbit, the timing cannot be guaranteed");

    return MP_OBJ_NEW_SMALL_INT(onewire_bus_readbit(mp_hal_get_pin_obj(pin_in)));
}
STATIC MP_DEFINE_CONST_FUN_OBJ_1(onewire_readbit_obj, onewire_readbit);

STATIC mp_obj_t onewire_readbyte(mp_obj_t pin_in) {
    mp_hal_pin_obj_t pin = mp_hal_get_pin_obj(pin_in);
    uint8_t value = 0;

    uint32_t i = mp_hal_quiet_timing_enter();
    for (int i = 0; i < 8; ++i) {
        value |= onewire_bus_readbit_without_lock(pin) << i;
    }
    mp_hal_quiet_timing_exit(i);

    return MP_OBJ_NEW_SMALL_INT(value);
}
STATIC MP_DEFINE_CONST_FUN_OBJ_1(onewire_readbyte_obj, onewire_readbyte);

STATIC mp_obj_t onewire_writebit(mp_obj_t pin_in, mp_obj_t value_in) {
    mp_printf(&mp_plat_print, "Please do not call writebit, the timing cannot be guaranteed");

    onewire_bus_writebit(mp_hal_get_pin_obj(pin_in), mp_obj_get_int(value_in));
    return mp_const_none;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_2(onewire_writebit_obj, onewire_writebit);

STATIC mp_obj_t onewire_writebyte(mp_obj_t pin_in, mp_obj_t value_in) {
    mp_hal_pin_obj_t pin = mp_hal_get_pin_obj(pin_in);
    int value = mp_obj_get_int(value_in);

    mp_hal_pin_od_high(pin);

    for (int i = 0; i < 8; ++i) {
        onewire_bus_writebit(pin, value & 1);
        value >>= 1;
    }
    return mp_const_none;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_2(onewire_writebyte_obj, onewire_writebyte);

STATIC mp_obj_t onewire_search_rom(mp_obj_t pin_in, mp_obj_t l_rom_in, mp_obj_t diff_in)
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

    if (0x00 == onewire_bus_search_rom(pin, rom, l_rom, &diff)) {
        mp_obj_t ba = mp_obj_new_bytearray(8, rom);
        return mp_obj_new_tuple(2, ((mp_obj_t[]) { ba, mp_obj_new_int(diff) }));
    } else {
        return mp_obj_new_tuple(2, ((mp_obj_t[]) { mp_const_none, mp_obj_new_int(0) }));
    }
}
STATIC MP_DEFINE_CONST_FUN_OBJ_3(onewire_search_rom_obj, onewire_search_rom);

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
