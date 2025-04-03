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

#include "generated/autoconf.h"

#if defined(CONFIG_ENABLE_UVC_CAMERA)

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <errno.h>
#include <fcntl.h>
#include <signal.h>
#include <sys/ioctl.h>
#include <unistd.h>

#include "py/obj.h"
#include "py/runtime.h"

#include "py_assert.h" // use openmv marco, PY_ASSERT_TYPE
#include "py_image.h"

#include "mpprint.h"

#include "mpi_uvc_api.h"

static struct uvc_format curr_format = { 0 };
static struct uvc_frame  curr_frame  = { .index = INVALID_FRAME_INDEX, 0 };

/* uvc_video_mode ************************************************************/
STATIC const mp_obj_type_t py_uvc_video_mode_type;

typedef struct _py_uvc_vicdeo_mode_obj_t {
    mp_obj_base_t     base;
    struct uvc_format _cobj;
} py_uvc_vicdeo_mode_obj_t;

STATIC mp_obj_t py_uvc_video_mode_from_struct(struct uvc_format* mode)
{
    py_uvc_vicdeo_mode_obj_t* o = m_new_obj_with_finaliser(py_uvc_vicdeo_mode_obj_t);
    o->base.type                = &py_uvc_video_mode_type;
    o->_cobj                    = *mode;
    return o;
}

STATIC void* py_uvc_video_mode_cobj(mp_obj_t uvc_video_mode)
{
    PY_ASSERT_TYPE(uvc_video_mode, &py_uvc_video_mode_type);
    return &((py_uvc_vicdeo_mode_obj_t*)uvc_video_mode)->_cobj;
}

STATIC void py_uvc_video_mode_print(const mp_print_t* print, mp_obj_t self_in, mp_print_kind_t kind)
{
    py_uvc_vicdeo_mode_obj_t* self = MP_OBJ_TO_PTR(self_in);
    struct uvc_format*        mode = py_uvc_video_mode_cobj(self);

    mp_printf(print, "{\"width\":%u, \"height\":%u, \"format\":%s, \"fps\":%.2f}", mode->width, mode->height,
              (USBH_VIDEO_FORMAT_UNCOMPRESSED == mode->format_type)
                  ? "uncompress"
                  : (USBH_VIDEO_FORMAT_MJPEG == mode->format_type) ? "mjpeg" : "unknown",
              10000000.0f / mode->frameinterval);
}

STATIC mp_obj_t py_uvc_video_mode_attr(mp_obj_t self_in, qstr attr, mp_obj_t* dest)
{
    py_uvc_vicdeo_mode_obj_t* self = MP_OBJ_TO_PTR(self_in);
    struct uvc_format*        mode = py_uvc_video_mode_cobj(self);

    if (MP_OBJ_NULL == dest[0]) {
        // load attribute
        switch (attr) {
        case MP_QSTR_width:
            return mp_obj_new_int(mode->width);
        case MP_QSTR_height:
            return mp_obj_new_int(mode->height);
        case MP_QSTR_format:
            return mp_obj_new_int(mode->format_type);
        case MP_QSTR_fps:
            return mp_obj_new_float(10000000.0f / mode->frameinterval);
        case MP_QSTR_frameinterval:
            return mp_obj_new_int(mode->frameinterval);
        default:
            return MP_OBJ_NULL;
        }
    } else if (MP_OBJ_SENTINEL == dest[0]) {
        // store attribute
    }

    return MP_OBJ_NULL;
}

STATIC MP_DEFINE_CONST_OBJ_TYPE(py_uvc_video_mode_type, MP_QSTR_uvc_video_mode, MP_TYPE_FLAG_NONE, print,
                                py_uvc_video_mode_print, attr, py_uvc_video_mode_attr);

STATIC mp_obj_t uvc_video_mode(size_t n, const mp_obj_t* objs)
{
    if (0x00 == n) {
        // get current mode
        return py_uvc_video_mode_from_struct(&curr_format);
    } else {
        struct uvc_format format
            = { .width = 0, .height = 0, .format_type = USBH_VIDEO_FORMAT_MJPEG, .frameinterval = 0 };

        if (0x02 > n) {
            mp_raise_msg(&mp_type_ValueError, MP_ERROR_TEXT("must set width and height"));
        }
        format.width  = mp_obj_get_int(objs[0]);
        format.height = mp_obj_get_int(objs[1]);

        if (0x03 <= n) {
            format.format_type = mp_obj_get_int(objs[2]);
        }

        /* TODO: support more format */
        if (USBH_VIDEO_FORMAT_UNCOMPRESSED != format.format_type) {
            format.format_type = USBH_VIDEO_FORMAT_MJPEG;
        }

        if (0x04 <= n) {
            format.frameinterval = 10000000 / mp_obj_get_int(objs[3]);
        }

        return py_uvc_video_mode_from_struct(&format);
    }
}
STATIC MP_DEFINE_CONST_FUN_OBJ_VAR_BETWEEN(uvc_video_mode_obj, 0, 4, uvc_video_mode);
STATIC MP_DEFINE_CONST_STATICMETHOD_OBJ(uvc_video_mode_method, MP_ROM_PTR(&uvc_video_mode_obj));

STATIC mp_obj_t uvc_probe(void)
{
    int  find = 0;
    char info[256];

    mp_obj_t info_obj = mp_const_none;

    memset(info, 0, sizeof(info));
    if (0x00 == uvc_get_devinfo(&info[0], sizeof(info))) {
        find = 1;

        info_obj = mp_obj_new_str(info, strlen(info));
    }

    return mp_obj_new_tuple(2, ((mp_obj_t[]) { mp_obj_new_bool(find), info_obj }));
}
STATIC MP_DEFINE_CONST_FUN_OBJ_0(uvc_probe_obj, uvc_probe);
STATIC MP_DEFINE_CONST_STATICMETHOD_OBJ(uvc_probe_method, MP_ROM_PTR(&uvc_probe_obj));

STATIC mp_obj_t uvc_list_video_mode(void)
{
    int                fmts_count = 0;
    struct uvc_format* fmts       = NULL;
    mp_obj_t           mode_list  = mp_obj_new_list(0, NULL);

    fmts_count = uvc_get_formats(&fmts);
    if (fmts_count && fmts) {
        for (int i = 0; i < fmts_count; i++) {
            mp_obj_t mode = py_uvc_video_mode_from_struct(&fmts[i]);
            mp_obj_list_append(mode_list, mode);
        }
    }
    uvc_free_formats(&fmts);

    return MP_OBJ_FROM_PTR(mode_list);
}
STATIC MP_DEFINE_CONST_FUN_OBJ_0(uvc_list_video_mode_obj, uvc_list_video_mode);
STATIC MP_DEFINE_CONST_STATICMETHOD_OBJ(uvc_list_video_mode_method, MP_ROM_PTR(&uvc_list_video_mode_obj));

STATIC mp_obj_t uvc_select_video_mode(mp_obj_t mode_in)
{
    py_uvc_vicdeo_mode_obj_t* self = MP_OBJ_TO_PTR(mode_in);
    struct uvc_format*        mode = py_uvc_video_mode_cobj(self);

    int succ = uvc_init(mode);

    if (0x00 == succ) {
        memcpy(&curr_format, mode, sizeof(*mode));
    }

    return mp_obj_new_tuple(2, ((mp_obj_t[]) { mp_obj_new_bool(0x00 == succ), mode_in }));
}
STATIC MP_DEFINE_CONST_FUN_OBJ_1(uvc_select_video_mode_obj, uvc_select_video_mode);
STATIC MP_DEFINE_CONST_STATICMETHOD_OBJ(uvc_select_video_mode_method, MP_ROM_PTR(&uvc_select_video_mode_obj));

STATIC mp_obj_t py_uvc_start(void)
{
    int succ = uvc_start_stream();
    return mp_obj_new_bool(succ);
}
STATIC MP_DEFINE_CONST_FUN_OBJ_0(uvc_start_obj, py_uvc_start);
STATIC MP_DEFINE_CONST_STATICMETHOD_OBJ(uvc_start_method, MP_ROM_PTR(&uvc_start_obj));

void mod_uvc_exit()
{
    if (INVALID_FRAME_INDEX != curr_frame.index) {
        uvc_put_frame(&curr_frame);
        memset(&curr_frame, 0, sizeof(curr_frame));
        curr_frame.index = INVALID_FRAME_INDEX;
    }
    memset(&curr_format, 0, sizeof(curr_format));

    uvc_exit();
}

STATIC mp_obj_t uvc_stop(void)
{
    mod_uvc_exit();

    return mp_const_none;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_0(uvc_stop_obj, uvc_stop);
STATIC MP_DEFINE_CONST_STATICMETHOD_OBJ(uvc_stop_method, MP_ROM_PTR(&uvc_stop_obj));

STATIC mp_obj_t uvc_snapshot(size_t n, const mp_obj_t* objs)
{
    int              timeout_ms = 1000;
    struct uvc_frame req_frame  = { 0 };

    if (0x01 <= n) {
        timeout_ms = mp_obj_get_int(objs[0]);
    }

    if (INVALID_FRAME_INDEX != curr_frame.index) {
        uvc_put_frame(&curr_frame);
        memset(&curr_frame, 0, sizeof(curr_frame));
        curr_frame.index = INVALID_FRAME_INDEX;
    }

    if (0x00 == uvc_get_frame(&req_frame, timeout_ms)) {
        memcpy(&curr_frame, &req_frame, sizeof(req_frame));

        if (USBH_VIDEO_FORMAT_UNCOMPRESSED == curr_format.format_type) {
            return py_image(curr_format.width, curr_format.height, PIXFORMAT_YUV422,
                            curr_format.width * curr_format.height * 2, req_frame.userptr);
        } else if (USBH_VIDEO_FORMAT_MJPEG == curr_format.format_type) {
            return py_image(curr_format.width, curr_format.height, PIXFORMAT_JPEG, req_frame.v_stream.len,
                            req_frame.userptr);
        } else {
            mp_printf(&mp_plat_print, "unsupport format %d\n", curr_format.format_type);
            return mp_const_none;
        }
    }

    return mp_const_none;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_VAR_BETWEEN(uvc_snapsho_obj, 0, 1, uvc_snapshot);
STATIC MP_DEFINE_CONST_STATICMETHOD_OBJ(uvc_snapsho_method, MP_ROM_PTR(&uvc_snapsho_obj));

STATIC const mp_rom_map_elem_t uvc_locals_dict_table[] = {
    { MP_ROM_QSTR(MP_QSTR_probe), MP_ROM_PTR(&uvc_probe_method) },
    { MP_ROM_QSTR(MP_QSTR_video_mode), MP_ROM_PTR(&uvc_video_mode_method) },
    { MP_ROM_QSTR(MP_QSTR_list_video_mode), MP_ROM_PTR(&uvc_list_video_mode_method) },
    { MP_ROM_QSTR(MP_QSTR_select_video_mode), MP_ROM_PTR(&uvc_select_video_mode_method) },
    { MP_ROM_QSTR(MP_QSTR_start), MP_ROM_PTR(&uvc_start_method) },
    { MP_ROM_QSTR(MP_QSTR_stop), MP_ROM_PTR(&uvc_stop_method) },
    { MP_ROM_QSTR(MP_QSTR_snapshot), MP_ROM_PTR(&uvc_snapsho_method) },

    { MP_ROM_QSTR(MP_QSTR_FORMAT_MJPEG), MP_ROM_INT(USBH_VIDEO_FORMAT_MJPEG) },
    { MP_ROM_QSTR(MP_QSTR_FORMAT_UNCOMPRESS), MP_ROM_INT(USBH_VIDEO_FORMAT_UNCOMPRESSED) },
};
STATIC MP_DEFINE_CONST_DICT(uvc_locals_dict, uvc_locals_dict_table);

MP_DEFINE_CONST_OBJ_TYPE(media_uvc_type, MP_QSTR_UVC, MP_TYPE_FLAG_NONE, locals_dict, &uvc_locals_dict);

#endif // CONFIG_ENABLE_UVC_CAMERA
