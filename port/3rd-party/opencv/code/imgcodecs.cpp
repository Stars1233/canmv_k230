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
