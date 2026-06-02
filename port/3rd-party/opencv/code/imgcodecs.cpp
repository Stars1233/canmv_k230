#include "imgcodecs.h"
#include "convert.h"
#include "numpy.h"
extern "C" {
#include "ndarray.h"
#include "ulab.h"
#include "py/runtime.h"
#include "py/stream.h"
}
#include "cv_upy_macros.h"
#include <opencv2/core.hpp>
#include <opencv2/imgcodecs.hpp>
#include <vector>

using namespace cv;

STATIC mp_obj_t cv2_imgcodecs_imread_fun(size_t n_args, const mp_obj_t *pos_args, mp_map_t *kw_args)
{
    enum { ARG_filename, ARG_flags };
    static const mp_arg_t allowed_args[] = {
        { MP_QSTR_filename, MP_ARG_REQUIRED | MP_ARG_OBJ, {.u_obj = MP_OBJ_NULL} },
        { MP_QSTR_flags,    MP_ARG_INT, {.u_int = IMREAD_COLOR} },
    };
    mp_arg_val_t args[MP_ARRAY_SIZE(allowed_args)];
    mp_arg_parse_all(n_args, pos_args, kw_args, MP_ARRAY_SIZE(allowed_args), allowed_args, args);

    int flags = args[ARG_flags].u_int;

    mp_obj_t filename_obj = args[ARG_filename].u_obj;
    const char *filename = mp_obj_str_get_str(filename_obj);

    Mat img;
    try {
        img = imread(filename, flags);
    } catch(Exception &e) {
        mp_raise_msg(&mp_type_Exception, MP_ERROR_TEXT(e.what()));
    }

    if(img.empty())
        return mp_const_none;

    return mat_to_mp_obj(img);
}
MP_DEFINE_CONST_FUN_OBJ_KW(cv2_imgcodecs_imread_obj, 1, cv2_imgcodecs_imread_fun);

STATIC mp_obj_t cv2_imgcodecs_imwrite_fun(size_t n_args, const mp_obj_t *pos_args, mp_map_t *kw_args)
{
    enum { ARG_filename, ARG_img };
    static const mp_arg_t allowed_args[] = {
        { MP_QSTR_filename, MP_ARG_REQUIRED | MP_ARG_OBJ, {.u_obj = MP_OBJ_NULL} },
        { MP_QSTR_img,      MP_ARG_REQUIRED | MP_ARG_OBJ, {.u_obj = MP_OBJ_NULL} },
    };
    mp_arg_val_t args[MP_ARRAY_SIZE(allowed_args)];
    mp_arg_parse_all(n_args, pos_args, kw_args, MP_ARRAY_SIZE(allowed_args), allowed_args, args);

    const char *filename = mp_obj_str_get_str(args[ARG_filename].u_obj);
    Mat img = mp_obj_to_mat(args[ARG_img].u_obj);

    bool success;
    try {
        success = imwrite(filename, img);
    } catch(Exception &e) {
        mp_raise_msg(&mp_type_Exception, MP_ERROR_TEXT(e.what()));
    }
    return mp_obj_new_bool(success);
}
MP_DEFINE_CONST_FUN_OBJ_KW(cv2_imgcodecs_imwrite_obj, 1, cv2_imgcodecs_imwrite_fun);

STATIC mp_obj_t cv2_imgcodecs_imdecode_fun(size_t n_args, const mp_obj_t *pos_args, mp_map_t *kw_args)
{
    enum { ARG_buf, ARG_flags };
    static const mp_arg_t allowed_args[] = {
        { MP_QSTR_buf,   MP_ARG_REQUIRED | MP_ARG_OBJ, {.u_obj = MP_OBJ_NULL} },
        { MP_QSTR_flags, MP_ARG_INT, {.u_int = IMREAD_COLOR} },
    };
    mp_arg_val_t args[MP_ARRAY_SIZE(allowed_args)];
    mp_arg_parse_all(n_args, pos_args, kw_args, MP_ARRAY_SIZE(allowed_args), allowed_args, args);

    int flags = args[ARG_flags].u_int;

    mp_obj_t buf_obj = args[ARG_buf].u_obj;
    mp_buffer_info_t bufinfo;
    mp_get_buffer_raise(buf_obj, &bufinfo, MP_BUFFER_READ);

    std::vector<uchar> buf((uchar *)bufinfo.buf, (uchar *)bufinfo.buf + bufinfo.len);

    Mat img;
    try {
        img = imdecode(buf, flags);
    } catch(Exception &e) {
        mp_raise_msg(&mp_type_Exception, MP_ERROR_TEXT(e.what()));
    }

    if(img.empty())
        return mp_const_none;

    return mat_to_mp_obj(img);
}
MP_DEFINE_CONST_FUN_OBJ_KW(cv2_imgcodecs_imdecode_obj, 1, cv2_imgcodecs_imdecode_fun);

STATIC mp_obj_t cv2_imgcodecs_imencode_fun(size_t n_args, const mp_obj_t *pos_args, mp_map_t *kw_args)
{
    enum { ARG_ext, ARG_img, ARG_params };
    static const mp_arg_t allowed_args[] = {
        { MP_QSTR_ext,    MP_ARG_REQUIRED | MP_ARG_OBJ, {.u_obj = MP_OBJ_NULL} },
        { MP_QSTR_img,    MP_ARG_REQUIRED | MP_ARG_OBJ, {.u_obj = MP_OBJ_NULL} },
        { MP_QSTR_params, MP_ARG_OBJ, {.u_obj = mp_const_none} },
    };
    mp_arg_val_t args[MP_ARRAY_SIZE(allowed_args)];
    mp_arg_parse_all(n_args, pos_args, kw_args, MP_ARRAY_SIZE(allowed_args), allowed_args, args);

    const char *ext = mp_obj_str_get_str(args[ARG_ext].u_obj);
    Mat img = mp_obj_to_mat(args[ARG_img].u_obj);

    std::vector<int> params;
    if(args[ARG_params].u_obj != mp_const_none)
    {
        mp_obj_t params_obj = args[ARG_params].u_obj;
        if(mp_obj_is_type(params_obj, &mp_type_list) || mp_obj_is_type(params_obj, &mp_type_tuple))
        {
            size_t len;
            mp_obj_t *items;
            mp_obj_get_array(params_obj, &len, &items);
            for(size_t i = 0; i < len; i++)
                params.push_back(mp_obj_get_int(items[i]));
        }
    }

    std::vector<uchar> buf;
    bool success;
    try {
        success = imencode(ext, img, buf, params);
    } catch(Exception &e) {
        mp_raise_msg(&mp_type_Exception, MP_ERROR_TEXT(e.what()));
    }

    mp_obj_t result[2];
    result[0] = mp_obj_new_bool(success);
    if(success)
        result[1] = mp_obj_new_bytes(buf.data(), buf.size());
    else
        result[1] = mp_const_none;
    return mp_obj_new_tuple(2, result);
}
MP_DEFINE_CONST_FUN_OBJ_KW(cv2_imgcodecs_imencode_obj, 1, cv2_imgcodecs_imencode_fun);
