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

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <errno.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <unistd.h>

#include "k_type.h"

#include "py/obj.h"
#include "py/runtime.h"
#include "py/stream.h"

#include "py_assert.h" // use openmv marco, PY_ASSERT_TYPE
#include "py_image.h"

#include "mpprint.h"

#include "py_modules.h"

typedef struct _py_usb_serial {
    int  fd;
    int  timeout_ms;
    char path[32];
} py_usb_seria_t;

typedef struct _py_usb_serial_obj {
    mp_obj_base_t  base;
    py_usb_seria_t _cobj;
} py_usb_serial_obj_t;

static void py_usb_serial_set_path(char *dst, size_t dst_size, const char *src)
{
    if ((dst_size == 0) || (dst == NULL)) {
        return;
    }

    if (src == NULL) {
        dst[0] = '\0';
        return;
    }

    strncpy(dst, src, dst_size - 1);
    dst[dst_size - 1] = '\0';
}

static __inline __attribute__((__always_inline__)) void py_usb_serial_is_opened(py_usb_seria_t* serial)
{
    if (0 > serial->fd) {
        mp_raise_msg_varg(&mp_type_OSError, MP_ERROR_TEXT("USB Serial not opened"));
    }
}

STATIC mp_obj_t py_usb_serial_from_struct(py_usb_seria_t* serial)
{
    py_usb_serial_obj_t* o = m_new_obj_with_finaliser(py_usb_serial_obj_t);

    o->base.type = &py_usb_serial_type;

    if (serial) {
        memcpy(&o->_cobj, serial, sizeof(*serial));
    } else {
        memset(&o->_cobj, 0x00, sizeof(*serial));
    }

    return MP_OBJ_FROM_PTR(o);
}

STATIC void* py_usb_serial_cobj(mp_obj_t self)
{
    PY_ASSERT_TYPE(self, &py_usb_serial_type);
    return &((py_usb_serial_obj_t*)self)->_cobj;
}

STATIC mp_obj_t py_usb_serial_make_new(const mp_obj_type_t* type, size_t n_args, size_t n_kw, const mp_obj_t* args)
{
    /* clang-format off */
    enum {ARG_path, ARG_timeout_ms, };
    static const mp_arg_t allowed_args[] = {
        { MP_QSTR_path, MP_ARG_OBJ | MP_ARG_REQUIRED, { .u_obj = mp_const_none  } },
        { MP_QSTR_timeout_ms, MP_ARG_INT, { .u_int = 300 } },
    };
    /* clang-format on */
    mp_map_t     kw_args;
    mp_arg_val_t parsed_args[MP_ARRAY_SIZE(allowed_args)];

    mp_arg_check_num(n_args, n_kw, 1, 2, true);
    mp_map_init_fixed_table(&kw_args, n_kw, args + n_args);

    mp_arg_parse_all(n_args, args, &kw_args, MP_ARRAY_SIZE(allowed_args), allowed_args, parsed_args);

    py_usb_seria_t serial;
    const char*    dev_path = "/dev/ttyUSB1";

    serial.fd         = -1;
    serial.timeout_ms = parsed_args[ARG_timeout_ms].u_int;

    if (mp_const_none != parsed_args[ARG_path].u_obj) {
        dev_path = mp_obj_str_get_str(parsed_args[ARG_path].u_obj);
    }
    py_usb_serial_set_path(serial.path, sizeof(serial.path), dev_path);

    return py_usb_serial_from_struct(&serial);
}

STATIC void py_usb_serial_print(const mp_print_t* print, mp_obj_t self_in, mp_print_kind_t kind)
{
    py_usb_seria_t* serial = py_usb_serial_cobj(MP_OBJ_TO_PTR(self_in));

    mp_printf(print, "{\"path\":%s, \"timeout_ms\":%d}", serial->path, serial->timeout_ms);
}

STATIC void py_usb_serial_attr(mp_obj_t self_in, qstr attr, mp_obj_t* dest)
{
    py_usb_seria_t* self = py_usb_serial_cobj(MP_OBJ_TO_PTR(self_in));

    if (MP_OBJ_NULL == dest[0]) {
        // load attribute
        switch (attr) {
        case MP_QSTR_path:
            dest[0] = mp_obj_new_str(self->path, strlen(self->path));
            break;
        case MP_QSTR_timeout_ms:
            dest[0] = mp_obj_new_int(self->timeout_ms);
            break;
        default:
            dest[1] = MP_OBJ_SENTINEL; // continue lookup in locals_dict
            break;
        }
    } else if ((MP_OBJ_SENTINEL == dest[0]) && (MP_OBJ_NULL == dest[1])) {
        // delete attr
    } else if ((MP_OBJ_SENTINEL == dest[0]) && (MP_OBJ_NULL != dest[1])) {
        // store attr
        switch (attr) {
        case MP_QSTR_path: {
            const char* path = mp_obj_str_get_str(dest[1]);
            if (strcmp(path, self->path) != 0) {
                py_usb_serial_set_path(self->path, sizeof(self->path), path);
            }

            dest[0] = MP_OBJ_NULL;
        } break;
        case MP_QSTR_timeout_ms: {
            self->timeout_ms = mp_obj_get_int(dest[1]);

            dest[0] = MP_OBJ_NULL;
        } break;
        }
    }
}

STATIC mp_obj_t py_usb_serial_open(size_t n_args, const mp_obj_t* args)
{
    int fd;

    py_usb_seria_t* self = py_usb_serial_cobj(MP_OBJ_TO_PTR(args[0]));

    if (0x02 == n_args) {
        const char* path = mp_obj_str_get_str(args[1]);

        if (strcmp(path, self->path) != 0) {
            py_usb_serial_set_path(self->path, sizeof(self->path), path);
        }
    }

    if (0 <= self->fd) {
        close(self->fd);
        self->fd = -1;
    }

    if (0 > (fd = open(self->path, O_RDWR | O_NONBLOCK))) {
        mp_printf(&mp_plat_print, "USB Serial: Open %s failed\n", self->path);
        return mp_const_false;
    }

    self->fd = fd;

    return mp_const_true;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_VAR_BETWEEN(py_usb_serial_open_obj, 1, 2, py_usb_serial_open);

STATIC mp_obj_t py_usb_serial_close(mp_obj_t self_in)
{
    py_usb_seria_t* self = py_usb_serial_cobj(MP_OBJ_TO_PTR(self_in));

    if (0 <= self->fd) {
        close(self->fd);
        self->fd = -1;
    }

    return mp_const_none;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_1(py_usb_serial_close_obj, py_usb_serial_close);

STATIC const mp_rom_map_elem_t py_usb_serial_locals_dict_table[] = {
    { MP_ROM_QSTR(MP_QSTR___del__), MP_ROM_PTR(&py_usb_serial_close_obj) },

    { MP_ROM_QSTR(MP_QSTR_open), MP_ROM_PTR(&py_usb_serial_open_obj) },
    { MP_ROM_QSTR(MP_QSTR_close), MP_ROM_PTR(&py_usb_serial_close_obj) },

    { MP_ROM_QSTR(MP_QSTR_readline), MP_ROM_PTR(&mp_stream_unbuffered_readline_obj) },
    { MP_ROM_QSTR(MP_QSTR_readinto), MP_ROM_PTR(&mp_stream_readinto_obj) },
    { MP_ROM_QSTR(MP_QSTR_write), MP_ROM_PTR(&mp_stream_write_obj) },
    { MP_ROM_QSTR(MP_QSTR_read), MP_ROM_PTR(&mp_stream_read_obj) },
};
STATIC MP_DEFINE_CONST_DICT(py_usb_serial_locals_dict, py_usb_serial_locals_dict_table);

/** stream protocol **********************************************************/
STATIC mp_uint_t py_usb_serial_read(mp_obj_t self_in, void* buf_in, mp_uint_t size, int* errcode)
{
    mp_uint_t bytes_read = 0;
    mp_uint_t curr_time_ms, time_out_ms;
    uint8_t*  buf = buf_in;

    py_usb_seria_t* self = py_usb_serial_cobj(MP_OBJ_TO_PTR(self_in));
    py_usb_serial_is_opened(self);

    if (size == 0) {
        return 0;
    }

    time_out_ms = mp_hal_ticks_ms() + self->timeout_ms;

    do {
        ssize_t rv = read(self->fd, buf + bytes_read, size - bytes_read);

        if (rv > 0) {
            bytes_read += (mp_uint_t)rv;
        } else if (rv == 0) {
            break;
        } else if ((errno != EAGAIN) && (errno != EWOULDBLOCK) && (errno != EINTR)) {
            *errcode = MP_EIO;
            return MP_STREAM_ERROR;
        }

        MICROPY_EVENT_POLL_HOOK

        curr_time_ms = mp_hal_ticks_ms();
    } while ((bytes_read != size) && (curr_time_ms < time_out_ms));

    if (bytes_read <= 0) {
        *errcode = MP_EAGAIN;
        return MP_STREAM_ERROR;
    }

    return bytes_read;
}

STATIC mp_uint_t py_usb_serial_write(mp_obj_t self_in, const void* buf_in, mp_uint_t size, int* errcode)
{
    py_usb_seria_t* self = py_usb_serial_cobj(MP_OBJ_TO_PTR(self_in));
    py_usb_serial_is_opened(self);

    int bytes_written = write(self->fd, buf_in, size);

    if (bytes_written < 0) {
        *errcode = MP_EAGAIN;
        return MP_STREAM_ERROR;
    }

    return bytes_written;
}

static size_t py_usb_serial_recv_available(mp_obj_t self_in)
{
    py_usb_seria_t* self = py_usb_serial_cobj(MP_OBJ_TO_PTR(self_in));
    py_usb_serial_is_opened(self);

    size_t bytes_available;
    if (ioctl(self->fd, FIONREAD, &bytes_available) < 0) {
        return 0;
    }

    return bytes_available;
}

STATIC mp_uint_t py_usb_serial_ioctl(mp_obj_t self_in, mp_uint_t request, uintptr_t arg, int* errcode)
{
    mp_uint_t ret = 0;

    *errcode      = 0;

    switch (request) {
    case MP_STREAM_POLL: {
        mp_uint_t flags = arg;
        ret             = 0;
        if ((flags & MP_STREAM_POLL_RD) && py_usb_serial_recv_available(self_in) > 0) {
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

STATIC const mp_stream_p_t py_usb_serial_protocol = {
    .read    = py_usb_serial_read,
    .write   = py_usb_serial_write,
    .ioctl   = py_usb_serial_ioctl,
    .is_text = false,
};

/* clang-format off */
MP_DEFINE_CONST_OBJ_TYPE(
    py_usb_serial_type,
    MP_QSTR_py_usb_serial,
    MP_TYPE_FLAG_NONE,
    make_new, py_usb_serial_make_new,
    print, py_usb_serial_print,
    attr, py_usb_serial_attr,
    protocol, &py_usb_serial_protocol,
    locals_dict, &py_usb_serial_locals_dict
);
/* clang-format on */
