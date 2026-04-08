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

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include <errno.h>

#include "py/mphal.h"
#include "py/objstr.h"
#include "py/obj.h"
#include "py/runtime.h"

#include "drv_input.h"

#include "py_modules.h"

#define PY_USB_HID_POLL_SLICE_MS  20

typedef struct _py_usb_hid_obj_t {
    mp_obj_base_t base;
    drv_input_inst_t *inst;
    struct drv_input_info info;
    uint32_t kind;
    int timeout_ms;
    bool auto_reconnect;
    bool explicit_path;
    bool shift_pressed;
    bool ctrl_pressed;
    bool alt_pressed;
    bool meta_pressed;
    bool caps_lock_enabled;
    char path[DRV_INPUT_PATH_MAX];
} py_usb_hid_obj_t;

static int py_usb_hid_wait_reconnect(py_usb_hid_obj_t *self, int timeout_ms, mp_uint_t start_ms);

static const char *py_usb_hid_kind_name(uint32_t kind)
{
    switch (kind) {
    case DRV_INPUT_DEV_KEYBOARD:
        return "keyboard";
    case DRV_INPUT_DEV_MOUSE:
        return "mouse";
    case DRV_INPUT_DEV_TOUCH:
        return "touch";
    default:
        return "input";
    }
}

static void py_usb_hid_raise_error(const char *op, int ret)
{
    mp_raise_msg_varg(&mp_type_OSError, MP_ERROR_TEXT("%s failed, %d"), op, ret);
}

static void py_usb_hid_set_path(py_usb_hid_obj_t *self, const char *path)
{
    if (path == NULL) {
        self->path[0] = '\0';
        return;
    }

    strncpy(self->path, path, sizeof(self->path) - 1);
    self->path[sizeof(self->path) - 1] = '\0';
}

static void py_usb_hid_reset_keyboard_state(py_usb_hid_obj_t *self)
{
    if (self == NULL) {
        return;
    }

    self->shift_pressed = false;
    self->ctrl_pressed = false;
    self->alt_pressed = false;
    self->meta_pressed = false;
    self->caps_lock_enabled = false;
}

static bool py_usb_hid_is_shift_key(uint16_t code)
{
    return code == KEY_LEFTSHIFT || code == KEY_RIGHTSHIFT;
}

static bool py_usb_hid_is_ctrl_key(uint16_t code)
{
    return code == KEY_LEFTCTRL || code == KEY_RIGHTCTRL;
}

static bool py_usb_hid_is_alt_key(uint16_t code)
{
    return code == KEY_LEFTALT || code == KEY_RIGHTALT;
}

static bool py_usb_hid_is_meta_key(uint16_t code)
{
    return code == KEY_LEFTMETA || code == KEY_RIGHTMETA;
}

static int py_usb_hid_translate_alpha_key(uint16_t code, bool shifted)
{
    switch (code) {
    case KEY_A: return shifted ? 'A' : 'a';
    case KEY_B: return shifted ? 'B' : 'b';
    case KEY_C: return shifted ? 'C' : 'c';
    case KEY_D: return shifted ? 'D' : 'd';
    case KEY_E: return shifted ? 'E' : 'e';
    case KEY_F: return shifted ? 'F' : 'f';
    case KEY_G: return shifted ? 'G' : 'g';
    case KEY_H: return shifted ? 'H' : 'h';
    case KEY_I: return shifted ? 'I' : 'i';
    case KEY_J: return shifted ? 'J' : 'j';
    case KEY_K: return shifted ? 'K' : 'k';
    case KEY_L: return shifted ? 'L' : 'l';
    case KEY_M: return shifted ? 'M' : 'm';
    case KEY_N: return shifted ? 'N' : 'n';
    case KEY_O: return shifted ? 'O' : 'o';
    case KEY_P: return shifted ? 'P' : 'p';
    case KEY_Q: return shifted ? 'Q' : 'q';
    case KEY_R: return shifted ? 'R' : 'r';
    case KEY_S: return shifted ? 'S' : 's';
    case KEY_T: return shifted ? 'T' : 't';
    case KEY_U: return shifted ? 'U' : 'u';
    case KEY_V: return shifted ? 'V' : 'v';
    case KEY_W: return shifted ? 'W' : 'w';
    case KEY_X: return shifted ? 'X' : 'x';
    case KEY_Y: return shifted ? 'Y' : 'y';
    case KEY_Z: return shifted ? 'Z' : 'z';
    default:
        return -1;
    }
}

static int py_usb_hid_translate_ctrl_char(uint16_t code)
{
    switch (code) {
    case KEY_A:
        return 1;
    case KEY_B:
        return 2;
    case KEY_C:
        return 3;
    case KEY_D:
        return 4;
    case KEY_E:
        return 5;
    case KEY_F:
        return 6;
    case KEY_G:
        return 7;
    case KEY_H:
        return 8;
    case KEY_I:
        return 9;
    case KEY_J:
        return 10;
    case KEY_K:
        return 11;
    case KEY_L:
        return 12;
    case KEY_M:
        return 13;
    case KEY_N:
        return 14;
    case KEY_O:
        return 15;
    case KEY_P:
        return 16;
    case KEY_Q:
        return 17;
    case KEY_R:
        return 18;
    case KEY_S:
        return 19;
    case KEY_T:
        return 20;
    case KEY_U:
        return 21;
    case KEY_V:
        return 22;
    case KEY_W:
        return 23;
    case KEY_X:
        return 24;
    case KEY_Y:
        return 25;
    case KEY_Z:
        return 26;
    case KEY_LEFTBRACE:
        return 27;
    case KEY_BACKSLASH:
        return 28;
    case KEY_RIGHTBRACE:
        return 29;
    case KEY_6:
    case KEY_MINUS:
        return 30;
    case KEY_SLASH:
        return 31;
    case KEY_SPACE:
    case KEY_2:
        return 0;
    default:
        return -1;
    }
}

static int py_usb_hid_translate_ascii_key(const py_usb_hid_obj_t *self, uint16_t code)
{
    int alpha_ch;
    bool shifted;

    if (self == NULL) {
        return -1;
    }

    if (self->ctrl_pressed) {
        return py_usb_hid_translate_ctrl_char(code);
    }

    shifted = (self->shift_pressed != self->caps_lock_enabled);

    alpha_ch = py_usb_hid_translate_alpha_key(code, shifted);
    if (alpha_ch >= 0) {
        return alpha_ch;
    }

    switch (code) {
    case KEY_1: return shifted ? '!' : '1';
    case KEY_2: return shifted ? '@' : '2';
    case KEY_3: return shifted ? '#' : '3';
    case KEY_4: return shifted ? '$' : '4';
    case KEY_5: return shifted ? '%' : '5';
    case KEY_6: return shifted ? '^' : '6';
    case KEY_7: return shifted ? '&' : '7';
    case KEY_8: return shifted ? '*' : '8';
    case KEY_9: return shifted ? '(' : '9';
    case KEY_0: return shifted ? ')' : '0';
    case KEY_SPACE: return ' ';
    case KEY_ENTER:
    case KEY_KPENTER:
        return '\n';
    case KEY_TAB:
        return '\t';
    case KEY_MINUS: return shifted ? '_' : '-';
    case KEY_EQUAL: return shifted ? '+' : '=';
    case KEY_LEFTBRACE: return shifted ? '{' : '[';
    case KEY_RIGHTBRACE: return shifted ? '}' : ']';
    case KEY_BACKSLASH: return shifted ? '|' : '\\';
    case KEY_SEMICOLON: return shifted ? ':' : ';';
    case KEY_APOSTROPHE: return shifted ? '"' : '\'';
    case KEY_GRAVE: return shifted ? '~' : '`';
    case KEY_COMMA: return shifted ? '<' : ',';
    case KEY_DOT: return shifted ? '>' : '.';
    case KEY_SLASH: return shifted ? '?' : '/';
    case KEY_KP0: return '0';
    case KEY_KP1: return '1';
    case KEY_KP2: return '2';
    case KEY_KP3: return '3';
    case KEY_KP4: return '4';
    case KEY_KP5: return '5';
    case KEY_KP6: return '6';
    case KEY_KP7: return '7';
    case KEY_KP8: return '8';
    case KEY_KP9: return '9';
    case KEY_KPDOT: return '.';
    case KEY_KPPLUS: return '+';
    case KEY_KPMINUS: return '-';
    case KEY_KPSLASH: return '/';
    case KEY_KPASTERISK: return '*';
    default:
        return -1;
    }
}

static void py_usb_hid_update_modifier_state(py_usb_hid_obj_t *self, uint16_t code, int32_t value)
{
    bool pressed;

    if (self == NULL) {
        return;
    }

    pressed = value != KEY_RELEASED;

    if (py_usb_hid_is_shift_key(code)) {
        self->shift_pressed = pressed;
        return;
    }

    if (py_usb_hid_is_ctrl_key(code)) {
        self->ctrl_pressed = pressed;
        return;
    }

    if (py_usb_hid_is_alt_key(code)) {
        self->alt_pressed = pressed;
        return;
    }

    if (py_usb_hid_is_meta_key(code)) {
        self->meta_pressed = pressed;
        return;
    }

    if (code == KEY_CAPSLOCK && value == KEY_PRESSED) {
        self->caps_lock_enabled = !self->caps_lock_enabled;
    }
}

static bool py_usb_hid_kind_matches(const py_usb_hid_obj_t *self, uint32_t kind)
{
    return self != NULL && self->kind == kind;
}

static void py_usb_hid_mark_disconnected(py_usb_hid_obj_t *self)
{
    if (self == NULL || self->inst == NULL) {
        return;
    }

    drv_input_inst_mark_disconnected(self->inst);
    memset(&self->info, 0, sizeof(self->info));
    py_usb_hid_reset_keyboard_state(self);
}

static int py_usb_hid_reconnect_after_disconnect(py_usb_hid_obj_t *self,
                                                 int timeout_ms,
                                                 mp_uint_t start_ms)
{
    py_usb_hid_mark_disconnected(self);
    return py_usb_hid_wait_reconnect(self, timeout_ms, start_ms);
}

static int py_usb_hid_refresh_info(py_usb_hid_obj_t *self)
{
    int ret;

    if (self == NULL || self->inst == NULL) {
        return -ENODEV;
    }

    ret = drv_input_get_info(self->inst, &self->info);
    if (ret != 0) {
        return ret;
    }

    if (!py_usb_hid_kind_matches(self, self->info.kind)) {
        drv_input_inst_destroy(&self->inst);
        return -EINVAL;
    }

    if (self->inst->path[0] != '\0') {
        py_usb_hid_set_path(self, self->inst->path);
    }

    return 0;
}

static int py_usb_hid_open_internal(py_usb_hid_obj_t *self)
{
    int ret;

    if (self == NULL) {
        return -EINVAL;
    }

    drv_input_inst_destroy(&self->inst);
    memset(&self->info, 0, sizeof(self->info));

    if (self->explicit_path && self->path[0] != '\0') {
        ret = drv_input_inst_create_path(self->path, &self->inst);
        if (ret != 0) {
            return ret;
        }
    } else {
        char path[DRV_INPUT_PATH_MAX];
        struct drv_input_info info;

        ret = drv_input_find_first_by_type(self->kind, path, sizeof(path), &info);
        if (ret != 0) {
            return ret;
        }

        ret = drv_input_inst_create_path(path, &self->inst);
        if (ret != 0) {
            return ret;
        }
    }

    ret = py_usb_hid_refresh_info(self);
    if (ret != 0) {
        drv_input_inst_destroy(&self->inst);
        return ret;
    }

    if (self->auto_reconnect) {
        drv_input_inst_set_auto_reconnect(self->inst, self->kind);
    }

    py_usb_hid_reset_keyboard_state(self);

    return 0;
}

static int py_usb_hid_reconnect_internal(py_usb_hid_obj_t *self)
{
    int ret;

    if (self == NULL) {
        return -EINVAL;
    }

    if (self->inst == NULL) {
        return py_usb_hid_open_internal(self);
    }

    ret = drv_input_inst_try_reconnect(self->inst);
    if (ret != 0) {
        return ret;
    }

    self->info = self->inst->info;
    if (self->inst->path[0] != '\0') {
        py_usb_hid_set_path(self, self->inst->path);
    }

    py_usb_hid_reset_keyboard_state(self);

    return 0;
}

static int py_usb_hid_wait_reconnect(py_usb_hid_obj_t *self, int timeout_ms, mp_uint_t start_ms)
{
    for (;;) {
        int ret;

        if (self == NULL || !self->auto_reconnect) {
            return -ENODEV;
        }

        ret = py_usb_hid_reconnect_internal(self);
        if (ret == 0) {
            return 0;
        }

        if (timeout_ms >= 0 && (mp_hal_ticks_ms() - start_ms) >= (mp_uint_t)timeout_ms) {
            return 1;
        }

        mp_hal_delay_ms(PY_USB_HID_POLL_SLICE_MS);
        MICROPY_EVENT_POLL_HOOK;
    }
}

static int py_usb_hid_wait_ready(py_usb_hid_obj_t *self, int timeout_ms)
{
    mp_uint_t start_ms;

    if (self == NULL) {
        return -ENODEV;
    }

    if (self->inst == NULL) {
        int ret = py_usb_hid_open_internal(self);
        if (ret != 0) {
            return ret;
        }
    }

    start_ms = mp_hal_ticks_ms();
    for (;;) {
        int wait_ms = PY_USB_HID_POLL_SLICE_MS;
        int ret;

        if (timeout_ms >= 0) {
            mp_uint_t elapsed_ms = mp_hal_ticks_ms() - start_ms;

            if (elapsed_ms >= (mp_uint_t)timeout_ms) {
                return 0;
            }

            if ((mp_uint_t)wait_ms > (mp_uint_t)timeout_ms - elapsed_ms) {
                wait_ms = (int)((mp_uint_t)timeout_ms - elapsed_ms);
            }
        }

        ret = drv_input_poll(self->inst, wait_ms);
        if (ret > 0) {
            return 1;
        }
        if (ret == 0) {
            MICROPY_EVENT_POLL_HOOK;
            continue;
        }

        if (ret == -ENODEV && self->auto_reconnect) {
            ret = py_usb_hid_reconnect_after_disconnect(self, timeout_ms, start_ms);
            if (ret == 0) {
                continue;
            }
            if (ret > 0) {
                return 0;
            }
            return ret;
        }

        return ret;
    }
}

static mp_obj_t py_usb_hid_build_info(const py_usb_hid_obj_t *self)
{
    mp_obj_t dict = mp_obj_new_dict(8);

    mp_obj_dict_store(dict, MP_OBJ_NEW_QSTR(MP_QSTR_kind), mp_obj_new_str(py_usb_hid_kind_name(self->kind), strlen(py_usb_hid_kind_name(self->kind))));
    mp_obj_dict_store(dict, MP_OBJ_NEW_QSTR(MP_QSTR_name), mp_obj_new_str(self->info.name, strlen(self->info.name)));
    mp_obj_dict_store(dict, MP_OBJ_NEW_QSTR(MP_QSTR_path), mp_obj_new_str(self->path, strlen(self->path)));
    mp_obj_dict_store(dict, MP_OBJ_NEW_QSTR(MP_QSTR_ev_bits), mp_obj_new_int_from_uint(self->info.ev_bits));
    mp_obj_dict_store(dict, MP_OBJ_NEW_QSTR(MP_QSTR_key_bits), mp_obj_new_int_from_uint(self->info.key_bits));
    mp_obj_dict_store(dict, MP_OBJ_NEW_QSTR(MP_QSTR_rel_bits), mp_obj_new_int_from_uint(self->info.rel_bits));
    mp_obj_dict_store(dict, MP_OBJ_NEW_QSTR(MP_QSTR_abs_bits), mp_obj_new_int_from_uint(self->info.abs_bits));
    mp_obj_dict_store(dict, MP_OBJ_NEW_QSTR(MP_QSTR_auto_reconnect), mp_obj_new_bool(self->auto_reconnect));
    return dict;
}

static mp_obj_t py_usb_hid_build_keyboard_frame(py_usb_hid_obj_t *self, const struct drv_keyboard_frame *frame)
{
    mp_obj_t dict = mp_obj_new_dict(10);
    mp_obj_t items[DRV_INPUT_MAX_FRAME_EVENTS];
    mp_obj_t char_items[DRV_INPUT_MAX_FRAME_EVENTS];
    mp_obj_t text_bytes_obj;
    vstr_t text;
    size_t char_count = 0;
    size_t index;

    for (index = 0; index < frame->count; index++) {
        items[index] = mp_obj_new_tuple(2,
            (mp_obj_t[]) {
                mp_obj_new_int(frame->keycodes[index]),
                mp_obj_new_int(frame->values[index]),
            });
    }

    vstr_init(&text, DRV_INPUT_MAX_FRAME_EVENTS);

    for (index = 0; index < frame->count; index++) {
        int ch = -1;

        py_usb_hid_update_modifier_state(self, frame->keycodes[index], frame->values[index]);
        if (frame->values[index] == KEY_RELEASED) {
            continue;
        }
        if (py_usb_hid_is_shift_key(frame->keycodes[index])
            || py_usb_hid_is_ctrl_key(frame->keycodes[index])
            || py_usb_hid_is_alt_key(frame->keycodes[index])
            || py_usb_hid_is_meta_key(frame->keycodes[index])
            || frame->keycodes[index] == KEY_CAPSLOCK) {
            continue;
        }

        ch = py_usb_hid_translate_ascii_key(self, frame->keycodes[index]);
        if (ch < 0) {
            continue;
        }

        vstr_add_char(&text, (char)ch);
        char_items[char_count++] = mp_obj_new_int(ch);
    }

    text_bytes_obj = mp_obj_new_bytes((const byte *)text.buf, text.len);

    mp_obj_dict_store(dict, MP_OBJ_NEW_QSTR(MP_QSTR_events), mp_obj_new_tuple(frame->count, items));
    mp_obj_dict_store(dict, MP_OBJ_NEW_QSTR(MP_QSTR_count), mp_obj_new_int_from_uint(frame->count));
    mp_obj_dict_store(dict, MP_OBJ_NEW_QSTR(MP_QSTR_complete), mp_obj_new_bool(frame->complete));
    mp_obj_dict_store(dict, MP_OBJ_NEW_QSTR(MP_QSTR_chars), mp_obj_new_tuple(char_count, char_items));
    mp_obj_dict_store(dict, MP_OBJ_NEW_QSTR(MP_QSTR_text), text_bytes_obj);
    mp_obj_dict_store(dict, MP_OBJ_NEW_QSTR(MP_QSTR_shift), mp_obj_new_bool(self->shift_pressed));
    mp_obj_dict_store(dict, MP_OBJ_NEW_QSTR(MP_QSTR_ctrl), mp_obj_new_bool(self->ctrl_pressed));
    mp_obj_dict_store(dict, MP_OBJ_NEW_QSTR(MP_QSTR_alt), mp_obj_new_bool(self->alt_pressed));
    mp_obj_dict_store(dict, MP_OBJ_NEW_QSTR(MP_QSTR_meta), mp_obj_new_bool(self->meta_pressed));
    mp_obj_dict_store(dict, MP_OBJ_NEW_QSTR(MP_QSTR_caps_lock), mp_obj_new_bool(self->caps_lock_enabled));
    return dict;
}

static mp_obj_t py_usb_hid_build_pointer_frame(const struct drv_pointer_frame *frame, uint32_t kind)
{
    mp_obj_t dict = mp_obj_new_dict(14);

    mp_obj_dict_store(dict, MP_OBJ_NEW_QSTR(MP_QSTR_kind), mp_obj_new_str(py_usb_hid_kind_name(kind), strlen(py_usb_hid_kind_name(kind))));
    mp_obj_dict_store(dict, MP_OBJ_NEW_QSTR(MP_QSTR_complete), mp_obj_new_bool(frame->complete));
    mp_obj_dict_store(dict, MP_OBJ_NEW_QSTR(MP_QSTR_has_rel), mp_obj_new_bool(frame->has_rel));
    mp_obj_dict_store(dict, MP_OBJ_NEW_QSTR(MP_QSTR_has_abs), mp_obj_new_bool(frame->has_abs));
    mp_obj_dict_store(dict, MP_OBJ_NEW_QSTR(MP_QSTR_touch_seen), mp_obj_new_bool(frame->touch_seen));
    mp_obj_dict_store(dict, MP_OBJ_NEW_QSTR(MP_QSTR_touch_down), mp_obj_new_bool(frame->touch_down));
    mp_obj_dict_store(dict, MP_OBJ_NEW_QSTR(MP_QSTR_rel_x), mp_obj_new_int(frame->rel_x));
    mp_obj_dict_store(dict, MP_OBJ_NEW_QSTR(MP_QSTR_rel_y), mp_obj_new_int(frame->rel_y));
    mp_obj_dict_store(dict, MP_OBJ_NEW_QSTR(MP_QSTR_wheel), mp_obj_new_int(frame->wheel));
    mp_obj_dict_store(dict, MP_OBJ_NEW_QSTR(MP_QSTR_hwheel), mp_obj_new_int(frame->hwheel));
    mp_obj_dict_store(dict, MP_OBJ_NEW_QSTR(MP_QSTR_abs_x), mp_obj_new_int(frame->abs_x));
    mp_obj_dict_store(dict, MP_OBJ_NEW_QSTR(MP_QSTR_abs_y), mp_obj_new_int(frame->abs_y));
    mp_obj_dict_store(dict, MP_OBJ_NEW_QSTR(MP_QSTR_pressure), mp_obj_new_int(frame->pressure));
    mp_obj_dict_store(dict, MP_OBJ_NEW_QSTR(MP_QSTR_buttons), mp_obj_new_int_from_uint(frame->buttons));
    mp_obj_dict_store(dict, MP_OBJ_NEW_QSTR(MP_QSTR_pressed_mask), mp_obj_new_int_from_uint(frame->pressed_mask));
    mp_obj_dict_store(dict, MP_OBJ_NEW_QSTR(MP_QSTR_released_mask), mp_obj_new_int_from_uint(frame->released_mask));
    return dict;
}

static mp_obj_t py_usb_hid_from_kind(const mp_obj_type_t *type, uint32_t kind, size_t n_args, size_t n_kw, const mp_obj_t *args)
{
    enum { ARG_path, ARG_timeout_ms, ARG_auto_reconnect };
    static const mp_arg_t allowed_args[] = {
        { MP_QSTR_path, MP_ARG_OBJ, { .u_obj = mp_const_none } },
        { MP_QSTR_timeout_ms, MP_ARG_INT, { .u_int = 300 } },
        { MP_QSTR_auto_reconnect, MP_ARG_BOOL, { .u_bool = true } },
    };
    mp_map_t kw_args;
    mp_arg_val_t parsed_args[MP_ARRAY_SIZE(allowed_args)];
    py_usb_hid_obj_t *self;

    mp_arg_check_num(n_args, n_kw, 0, 3, true);
    mp_map_init_fixed_table(&kw_args, n_kw, args + n_args);
    mp_arg_parse_all(n_args, args, &kw_args, MP_ARRAY_SIZE(allowed_args), allowed_args, parsed_args);

    self = m_new_obj_with_finaliser(py_usb_hid_obj_t);
    memset(self, 0, sizeof(*self));
    self->base.type = type;
    self->kind = kind;
    self->timeout_ms = parsed_args[ARG_timeout_ms].u_int;
    self->auto_reconnect = parsed_args[ARG_auto_reconnect].u_bool;
    self->explicit_path = parsed_args[ARG_path].u_obj != mp_const_none;

    if (parsed_args[ARG_path].u_obj != mp_const_none) {
        py_usb_hid_set_path(self, mp_obj_str_get_str(parsed_args[ARG_path].u_obj));
    }

    return MP_OBJ_FROM_PTR(self);
}

static void py_usb_hid_print(const mp_print_t *print, mp_obj_t self_in, mp_print_kind_t kind)
{
    py_usb_hid_obj_t *self = MP_OBJ_TO_PTR(self_in);

    mp_printf(print,
              "%s(path=%s, timeout_ms=%d, auto_reconnect=%d, opened=%d)",
              py_usb_hid_kind_name(self->kind),
              self->path[0] != '\0' ? self->path : "auto",
              self->timeout_ms,
              self->auto_reconnect,
              self->inst != NULL);
}

static void py_usb_hid_attr(mp_obj_t self_in, qstr attr, mp_obj_t *dest)
{
    py_usb_hid_obj_t *self = MP_OBJ_TO_PTR(self_in);

    if (dest[0] == MP_OBJ_NULL) {
        switch (attr) {
        case MP_QSTR_path:
            dest[0] = mp_obj_new_str(self->path, strlen(self->path));
            break;
        case MP_QSTR_timeout_ms:
            dest[0] = mp_obj_new_int(self->timeout_ms);
            break;
        case MP_QSTR_auto_reconnect:
            dest[0] = mp_obj_new_bool(self->auto_reconnect);
            break;
        default:
            dest[1] = MP_OBJ_SENTINEL;
            break;
        }
    } else if (dest[0] == MP_OBJ_SENTINEL && dest[1] != MP_OBJ_NULL) {
        switch (attr) {
        case MP_QSTR_path:
            py_usb_hid_set_path(self, mp_obj_str_get_str(dest[1]));
            self->explicit_path = true;
            dest[0] = MP_OBJ_NULL;
            break;
        case MP_QSTR_timeout_ms:
            self->timeout_ms = mp_obj_get_int(dest[1]);
            dest[0] = MP_OBJ_NULL;
            break;
        case MP_QSTR_auto_reconnect:
            self->auto_reconnect = mp_obj_is_true(dest[1]);
            dest[0] = MP_OBJ_NULL;
            break;
        }
    }
}

static mp_obj_t py_usb_hid_open(size_t n_args, const mp_obj_t *args)
{
    py_usb_hid_obj_t *self = MP_OBJ_TO_PTR(args[0]);
    int ret;

    if (n_args == 2) {
        self->explicit_path = args[1] != mp_const_none;
        py_usb_hid_set_path(self, args[1] == mp_const_none ? NULL : mp_obj_str_get_str(args[1]));
    }

    ret = py_usb_hid_open_internal(self);
    if (ret != 0) {
        py_usb_hid_raise_error("open", ret);
    }

    return mp_const_true;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_VAR_BETWEEN(py_usb_hid_open_obj, 1, 2, py_usb_hid_open);

static mp_obj_t py_usb_hid_close(mp_obj_t self_in)
{
    py_usb_hid_obj_t *self = MP_OBJ_TO_PTR(self_in);

    drv_input_inst_destroy(&self->inst);
    memset(&self->info, 0, sizeof(self->info));
    py_usb_hid_reset_keyboard_state(self);
    return mp_const_none;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_1(py_usb_hid_close_obj, py_usb_hid_close);

static mp_obj_t py_usb_hid_is_open(mp_obj_t self_in)
{
    py_usb_hid_obj_t *self = MP_OBJ_TO_PTR(self_in);
    return mp_obj_new_bool(drv_input_inst_is_connected(self->inst));
}
STATIC MP_DEFINE_CONST_FUN_OBJ_1(py_usb_hid_is_open_obj, py_usb_hid_is_open);

static int py_usb_hid_get_timeout_arg(py_usb_hid_obj_t *self, size_t n_args, const mp_obj_t *args)
{
    if (n_args > 1) {
        return mp_obj_get_int(args[1]);
    }
    return self->timeout_ms;
}

static mp_obj_t py_usb_hid_poll(size_t n_args, const mp_obj_t *args)
{
    py_usb_hid_obj_t *self = MP_OBJ_TO_PTR(args[0]);
    int timeout_ms = py_usb_hid_get_timeout_arg(self, n_args, args);
    int ret;

    if (!drv_input_inst_is_connected(self->inst) && !self->auto_reconnect) {
        py_usb_hid_raise_error("poll", -ENODEV);
    }

    ret = py_usb_hid_wait_ready(self, timeout_ms);
    if (ret < 0) {
        py_usb_hid_raise_error("poll", ret);
    }

    return mp_obj_new_bool(ret > 0);
}
STATIC MP_DEFINE_CONST_FUN_OBJ_VAR_BETWEEN(py_usb_hid_poll_obj, 1, 2, py_usb_hid_poll);

static mp_obj_t py_usb_hid_info(mp_obj_t self_in)
{
    py_usb_hid_obj_t *self = MP_OBJ_TO_PTR(self_in);
    int ret;

    if (self->inst == NULL) {
        ret = py_usb_hid_open_internal(self);
        if (ret != 0) {
            py_usb_hid_raise_error("info", ret);
        }
    }

    if (!drv_input_inst_is_connected(self->inst)) {
        if (self->auto_reconnect) {
            ret = py_usb_hid_reconnect_internal(self);
            if (ret != 0) {
                py_usb_hid_raise_error("info", -ENODEV);
            }
        } else {
            py_usb_hid_raise_error("info", -ENODEV);
        }
    }

    ret = py_usb_hid_refresh_info(self);
    if (ret != 0) {
        py_usb_hid_raise_error("info", ret);
    }

    return py_usb_hid_build_info(self);
}
STATIC MP_DEFINE_CONST_FUN_OBJ_1(py_usb_hid_info_obj, py_usb_hid_info);

static mp_obj_t py_usb_hid_reconnect(mp_obj_t self_in)
{
    py_usb_hid_obj_t *self = MP_OBJ_TO_PTR(self_in);
    int ret = py_usb_hid_reconnect_internal(self);

    if (ret != 0) {
        py_usb_hid_raise_error("reconnect", ret);
    }

    return mp_const_true;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_1(py_usb_hid_reconnect_obj, py_usb_hid_reconnect);

static mp_obj_t py_usb_keyboard_make_new(const mp_obj_type_t *type, size_t n_args, size_t n_kw, const mp_obj_t *args)
{
    return py_usb_hid_from_kind(type, DRV_INPUT_DEV_KEYBOARD, n_args, n_kw, args);
}

static mp_obj_t py_usb_mouse_make_new(const mp_obj_type_t *type, size_t n_args, size_t n_kw, const mp_obj_t *args)
{
    return py_usb_hid_from_kind(type, DRV_INPUT_DEV_MOUSE, n_args, n_kw, args);
}

static mp_obj_t py_usb_touch_make_new(const mp_obj_type_t *type, size_t n_args, size_t n_kw, const mp_obj_t *args)
{
    return py_usb_hid_from_kind(type, DRV_INPUT_DEV_TOUCH, n_args, n_kw, args);
}

static mp_obj_t py_usb_keyboard_read(size_t n_args, const mp_obj_t *args)
{
    py_usb_hid_obj_t *self = MP_OBJ_TO_PTR(args[0]);
    int timeout_ms = py_usb_hid_get_timeout_arg(self, n_args, args);
    mp_uint_t start_ms = mp_hal_ticks_ms();

    for (;;) {
        struct drv_keyboard_frame frame;
        int remaining_ms = timeout_ms;
        int ret;

        if (timeout_ms >= 0) {
            mp_uint_t elapsed_ms = mp_hal_ticks_ms() - start_ms;

            if (elapsed_ms >= (mp_uint_t)timeout_ms) {
                return mp_const_none;
            }

            remaining_ms = (int)((mp_uint_t)timeout_ms - elapsed_ms);
        }

        ret = py_usb_hid_wait_ready(self, remaining_ms);
        if (ret < 0) {
            py_usb_hid_raise_error("read", ret);
        }
        if (ret == 0) {
            return mp_const_none;
        }

        ret = drv_input_read_keyboard_frame(self->inst, &frame);
        if (ret > 0) {
            if (frame.count == 0 && !frame.complete) {
                MICROPY_EVENT_POLL_HOOK;
                continue;
            }
            return py_usb_hid_build_keyboard_frame(self, &frame);
        }

        if (ret == 0) {
            MICROPY_EVENT_POLL_HOOK;
            continue;
        }

        py_usb_hid_raise_error("read", ret);
    }
}
STATIC MP_DEFINE_CONST_FUN_OBJ_VAR_BETWEEN(py_usb_keyboard_read_obj, 1, 2, py_usb_keyboard_read);

static mp_obj_t py_usb_pointer_read(size_t n_args, const mp_obj_t *args)
{
    py_usb_hid_obj_t *self = MP_OBJ_TO_PTR(args[0]);
    int timeout_ms = py_usb_hid_get_timeout_arg(self, n_args, args);
    mp_uint_t start_ms = mp_hal_ticks_ms();

    for (;;) {
        struct drv_pointer_frame frame;
        int remaining_ms = timeout_ms;
        int ret;

        if (timeout_ms >= 0) {
            mp_uint_t elapsed_ms = mp_hal_ticks_ms() - start_ms;

            if (elapsed_ms >= (mp_uint_t)timeout_ms) {
                return mp_const_none;
            }

            remaining_ms = (int)((mp_uint_t)timeout_ms - elapsed_ms);
        }

        ret = py_usb_hid_wait_ready(self, remaining_ms);
        if (ret < 0) {
            py_usb_hid_raise_error("read", ret);
        }
        if (ret == 0) {
            return mp_const_none;
        }

        ret = drv_input_read_pointer_frame(self->inst, &frame);
        if (ret > 0) {
            if (!frame.complete && !frame.has_rel && !frame.has_abs && frame.pressed_mask == 0 && frame.released_mask == 0) {
                MICROPY_EVENT_POLL_HOOK;
                continue;
            }
            return py_usb_hid_build_pointer_frame(&frame, self->kind);
        }

        if (ret == 0) {
            MICROPY_EVENT_POLL_HOOK;
            continue;
        }

        py_usb_hid_raise_error("read", ret);
    }
}
STATIC MP_DEFINE_CONST_FUN_OBJ_VAR_BETWEEN(py_usb_pointer_read_obj, 1, 2, py_usb_pointer_read);

STATIC const mp_rom_map_elem_t py_usb_keyboard_locals_dict_table[] = {
    { MP_ROM_QSTR(MP_QSTR___del__), MP_ROM_PTR(&py_usb_hid_close_obj) },
    { MP_ROM_QSTR(MP_QSTR_close), MP_ROM_PTR(&py_usb_hid_close_obj) },
    { MP_ROM_QSTR(MP_QSTR_info), MP_ROM_PTR(&py_usb_hid_info_obj) },
    { MP_ROM_QSTR(MP_QSTR_is_open), MP_ROM_PTR(&py_usb_hid_is_open_obj) },
    { MP_ROM_QSTR(MP_QSTR_open), MP_ROM_PTR(&py_usb_hid_open_obj) },
    { MP_ROM_QSTR(MP_QSTR_poll), MP_ROM_PTR(&py_usb_hid_poll_obj) },
    { MP_ROM_QSTR(MP_QSTR_read), MP_ROM_PTR(&py_usb_keyboard_read_obj) },
    { MP_ROM_QSTR(MP_QSTR_reconnect), MP_ROM_PTR(&py_usb_hid_reconnect_obj) },

    { MP_ROM_QSTR(MP_QSTR_VALUE_RELEASED), MP_ROM_INT(KEY_RELEASED) },
    { MP_ROM_QSTR(MP_QSTR_VALUE_PRESSED), MP_ROM_INT(KEY_PRESSED) },
    { MP_ROM_QSTR(MP_QSTR_VALUE_REPEAT), MP_ROM_INT(KEY_REPEAT) },
};
STATIC MP_DEFINE_CONST_DICT(py_usb_keyboard_locals_dict, py_usb_keyboard_locals_dict_table);

STATIC const mp_rom_map_elem_t py_usb_pointer_locals_dict_table[] = {
    { MP_ROM_QSTR(MP_QSTR___del__), MP_ROM_PTR(&py_usb_hid_close_obj) },
    { MP_ROM_QSTR(MP_QSTR_BTN_LEFT_MASK), MP_ROM_INT(1u << 0) },
    { MP_ROM_QSTR(MP_QSTR_BTN_RIGHT_MASK), MP_ROM_INT(1u << 1) },
    { MP_ROM_QSTR(MP_QSTR_BTN_MIDDLE_MASK), MP_ROM_INT(1u << 2) },
    { MP_ROM_QSTR(MP_QSTR_BTN_TOUCH_MASK), MP_ROM_INT(1u << 7) },
    { MP_ROM_QSTR(MP_QSTR_close), MP_ROM_PTR(&py_usb_hid_close_obj) },
    { MP_ROM_QSTR(MP_QSTR_info), MP_ROM_PTR(&py_usb_hid_info_obj) },
    { MP_ROM_QSTR(MP_QSTR_is_open), MP_ROM_PTR(&py_usb_hid_is_open_obj) },
    { MP_ROM_QSTR(MP_QSTR_open), MP_ROM_PTR(&py_usb_hid_open_obj) },
    { MP_ROM_QSTR(MP_QSTR_poll), MP_ROM_PTR(&py_usb_hid_poll_obj) },
    { MP_ROM_QSTR(MP_QSTR_read), MP_ROM_PTR(&py_usb_pointer_read_obj) },
    { MP_ROM_QSTR(MP_QSTR_reconnect), MP_ROM_PTR(&py_usb_hid_reconnect_obj) },
};
STATIC MP_DEFINE_CONST_DICT(py_usb_pointer_locals_dict, py_usb_pointer_locals_dict_table);

/* clang-format off */
MP_DEFINE_CONST_OBJ_TYPE(
    py_usb_keyboard_type,
    MP_QSTR_Keyboard,
    MP_TYPE_FLAG_NONE,
    make_new, py_usb_keyboard_make_new,
    print, py_usb_hid_print,
    attr, py_usb_hid_attr,
    locals_dict, &py_usb_keyboard_locals_dict
    );

MP_DEFINE_CONST_OBJ_TYPE(
    py_usb_mouse_type,
    MP_QSTR_Mouse,
    MP_TYPE_FLAG_NONE,
    make_new, py_usb_mouse_make_new,
    print, py_usb_hid_print,
    attr, py_usb_hid_attr,
    locals_dict, &py_usb_pointer_locals_dict
    );

MP_DEFINE_CONST_OBJ_TYPE(
    py_usb_touch_type,
    MP_QSTR_Touch,
    MP_TYPE_FLAG_NONE,
    make_new, py_usb_touch_make_new,
    print, py_usb_hid_print,
    attr, py_usb_hid_attr,
    locals_dict, &py_usb_pointer_locals_dict
    );

/* clang-format on */
