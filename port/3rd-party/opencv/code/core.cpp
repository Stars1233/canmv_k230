#include "core.h"
#include "convert.h"
#include "numpy.h"
extern "C" {
#include "ndarray.h"
#include "ulab.h"
#include "py/runtime.h"
}
#include "cv_upy_macros.h"
#include <opencv2/core.hpp>
#include <opencv2/imgproc.hpp>

using namespace cv;

STATIC mp_obj_t cv2_core_inRange_fun(size_t n_args, const mp_obj_t *pos_args, mp_map_t *kw_args)
{
    enum { ARG_src, ARG_lowerb, ARG_upperb, ARG_dst };
    static const mp_arg_t allowed_args[] = {
        { MP_QSTR_src,     MP_ARG_REQUIRED | MP_ARG_OBJ, {.u_obj = MP_OBJ_NULL} },
        { MP_QSTR_lowerb,  MP_ARG_REQUIRED | MP_ARG_OBJ, {.u_obj = MP_OBJ_NULL} },
        { MP_QSTR_upperb,  MP_ARG_REQUIRED | MP_ARG_OBJ, {.u_obj = MP_OBJ_NULL} },
        { MP_QSTR_dst,     MP_ARG_OBJ, {.u_obj = mp_const_none} },
    };
    mp_arg_val_t args[MP_ARRAY_SIZE(allowed_args)];
    mp_arg_parse_all(n_args, pos_args, kw_args, MP_ARRAY_SIZE(allowed_args), allowed_args, args);

    Mat src = mp_obj_to_mat(args[ARG_src].u_obj);
    Mat lowerb = mp_obj_to_mat(args[ARG_lowerb].u_obj);
    Mat upperb = mp_obj_to_mat(args[ARG_upperb].u_obj);
    Mat dst = mp_obj_to_mat(args[ARG_dst].u_obj);

    try {
        inRange(src, lowerb, upperb, dst);
    } catch(Exception &e) {
        mp_raise_msg(&mp_type_Exception, MP_ERROR_TEXT(e.what()));
    }

    return mat_to_mp_obj(dst);
}
MP_DEFINE_CONST_FUN_OBJ_KW(cv2_core_inRange_obj, 1, cv2_core_inRange_fun);

STATIC mp_obj_t cv2_core_convertScaleAbs_fun(size_t n_args, const mp_obj_t *pos_args, mp_map_t *kw_args)
{
    enum { ARG_src, ARG_dst, ARG_alpha, ARG_beta };
    static const mp_arg_t allowed_args[] = {
        { MP_QSTR_src,    MP_ARG_REQUIRED | MP_ARG_OBJ, {.u_obj = MP_OBJ_NULL} },
        { MP_QSTR_dst,    MP_ARG_OBJ, {.u_obj = mp_const_none} },
        { MP_QSTR_alpha,  MP_ARG_OBJ, {.u_obj = mp_const_none} },
        { MP_QSTR_beta,   MP_ARG_OBJ, {.u_obj = mp_const_none} },
    };
    mp_arg_val_t args[MP_ARRAY_SIZE(allowed_args)];
    mp_arg_parse_all(n_args, pos_args, kw_args, MP_ARRAY_SIZE(allowed_args), allowed_args, args);

    Mat src = mp_obj_to_mat(args[ARG_src].u_obj);
    Mat dst = mp_obj_to_mat(args[ARG_dst].u_obj);
    mp_float_t alpha = args[ARG_alpha].u_obj == mp_const_none ? 1.0 : mp_obj_get_float(args[ARG_alpha].u_obj);
    mp_float_t beta  = args[ARG_beta].u_obj  == mp_const_none ? 0.0 : mp_obj_get_float(args[ARG_beta].u_obj);

    try {
        convertScaleAbs(src, dst, alpha, beta);
    } catch(Exception &e) {
        mp_raise_msg(&mp_type_Exception, MP_ERROR_TEXT(e.what()));
    }

    return mat_to_mp_obj(dst);
}
MP_DEFINE_CONST_FUN_OBJ_KW(cv2_core_convertScaleAbs_obj, 0, cv2_core_convertScaleAbs_fun);

STATIC mp_obj_t cv2_core_minMaxLoc_fun(size_t n_args, const mp_obj_t *pos_args, mp_map_t *kw_args)
{
    enum { ARG_src, ARG_mask };
    static const mp_arg_t allowed_args[] = {
        { MP_QSTR_src,  MP_ARG_REQUIRED | MP_ARG_OBJ, {.u_obj = MP_OBJ_NULL} },
        { MP_QSTR_mask, MP_ARG_OBJ, {.u_obj = mp_const_none} },
    };
    mp_arg_val_t args[MP_ARRAY_SIZE(allowed_args)];
    mp_arg_parse_all(n_args, pos_args, kw_args, MP_ARRAY_SIZE(allowed_args), allowed_args, args);

    Mat src = mp_obj_to_mat(args[ARG_src].u_obj);
    Mat mask = mp_obj_to_mat(args[ARG_mask].u_obj);

    double minVal, maxVal;
    Point minLoc, maxLoc;

    try {
        minMaxLoc(src, &minVal, &maxVal, &minLoc, &maxLoc, mask.empty() ? noArray() : (InputArray)mask);
    } catch(Exception &e) {
        mp_raise_msg(&mp_type_Exception, MP_ERROR_TEXT(e.what()));
    }

    mp_obj_t minLoc_arr[] = { mp_obj_new_int(minLoc.x), mp_obj_new_int(minLoc.y) };
    mp_obj_t maxLoc_arr[] = { mp_obj_new_int(maxLoc.x), mp_obj_new_int(maxLoc.y) };

    mp_obj_t result[4];
    result[0] = mp_obj_new_float(minVal);
    result[1] = mp_obj_new_float(maxVal);
    result[2] = mp_obj_new_tuple(2, minLoc_arr);
    result[3] = mp_obj_new_tuple(2, maxLoc_arr);
    return mp_obj_new_tuple(4, result);
}
MP_DEFINE_CONST_FUN_OBJ_KW(cv2_core_minMaxLoc_obj, 1, cv2_core_minMaxLoc_fun);

STATIC mp_obj_t cv2_core_LUT_fun(size_t n_args, const mp_obj_t *pos_args, mp_map_t *kw_args)
{
    enum { ARG_src, ARG_lut, ARG_dst };
    static const mp_arg_t allowed_args[] = {
        { MP_QSTR_src, MP_ARG_REQUIRED | MP_ARG_OBJ, {.u_obj = MP_OBJ_NULL} },
        { MP_QSTR_lut, MP_ARG_REQUIRED | MP_ARG_OBJ, {.u_obj = MP_OBJ_NULL} },
        { MP_QSTR_dst, MP_ARG_OBJ, {.u_obj = mp_const_none} },
    };
    mp_arg_val_t args[MP_ARRAY_SIZE(allowed_args)];
    mp_arg_parse_all(n_args, pos_args, kw_args, MP_ARRAY_SIZE(allowed_args), allowed_args, args);

    Mat src = mp_obj_to_mat(args[ARG_src].u_obj);
    Mat lut = mp_obj_to_mat(args[ARG_lut].u_obj);
    Mat dst = mp_obj_to_mat(args[ARG_dst].u_obj);

    try {
        LUT(src, lut, dst);
    } catch(Exception &e) {
        mp_raise_msg(&mp_type_Exception, MP_ERROR_TEXT(e.what()));
    }

    return mat_to_mp_obj(dst);
}
MP_DEFINE_CONST_FUN_OBJ_KW(cv2_core_LUT_obj, 1, cv2_core_LUT_fun);

STATIC mp_obj_t cv2_core_countNonZero_fun(size_t n_args, const mp_obj_t *pos_args, mp_map_t *kw_args)
{
    enum { ARG_src };
    static const mp_arg_t allowed_args[] = {
        { MP_QSTR_src, MP_ARG_REQUIRED | MP_ARG_OBJ, {.u_obj = MP_OBJ_NULL} },
    };
    mp_arg_val_t args[MP_ARRAY_SIZE(allowed_args)];
    mp_arg_parse_all(n_args, pos_args, kw_args, MP_ARRAY_SIZE(allowed_args), allowed_args, args);

    Mat src = mp_obj_to_mat(args[ARG_src].u_obj);

    int count = 0;
    try {
        count = countNonZero(src);
    } catch(Exception &e) {
        mp_raise_msg(&mp_type_Exception, MP_ERROR_TEXT(e.what()));
    }

    return mp_obj_new_int(count);
}
MP_DEFINE_CONST_FUN_OBJ_KW(cv2_core_countNonZero_obj, 1, cv2_core_countNonZero_fun);

STATIC mp_obj_t cv2_core_flip_fun(size_t n_args, const mp_obj_t *pos_args, mp_map_t *kw_args)
{
    enum { ARG_src, ARG_flipCode, ARG_dst };
    static const mp_arg_t allowed_args[] = {
        { MP_QSTR_src,       MP_ARG_REQUIRED | MP_ARG_OBJ, {.u_obj = MP_OBJ_NULL} },
        { MP_QSTR_flipCode,  MP_ARG_REQUIRED | MP_ARG_INT, {.u_int = 0} },
        { MP_QSTR_dst,       MP_ARG_OBJ, {.u_obj = mp_const_none} },
    };
    mp_arg_val_t args[MP_ARRAY_SIZE(allowed_args)];
    mp_arg_parse_all(n_args, pos_args, kw_args, MP_ARRAY_SIZE(allowed_args), allowed_args, args);

    Mat src = mp_obj_to_mat(args[ARG_src].u_obj);
    Mat dst = mp_obj_to_mat(args[ARG_dst].u_obj);
    int flipCode = args[ARG_flipCode].u_int;

    try {
        flip(src, dst, flipCode);
    } catch(Exception &e) {
        mp_raise_msg(&mp_type_Exception, MP_ERROR_TEXT(e.what()));
    }

    return mat_to_mp_obj(dst);
}
MP_DEFINE_CONST_FUN_OBJ_KW(cv2_core_flip_obj, 1, cv2_core_flip_fun);

STATIC mp_obj_t cv2_core_rotate_fun(size_t n_args, const mp_obj_t *pos_args, mp_map_t *kw_args)
{
    enum { ARG_src, ARG_rotateCode, ARG_dst };
    static const mp_arg_t allowed_args[] = {
        { MP_QSTR_src,         MP_ARG_REQUIRED | MP_ARG_OBJ, {.u_obj = MP_OBJ_NULL} },
        { MP_QSTR_rotateCode,  MP_ARG_REQUIRED | MP_ARG_INT, {.u_int = 0} },
        { MP_QSTR_dst,         MP_ARG_OBJ, {.u_obj = mp_const_none} },
    };
    mp_arg_val_t args[MP_ARRAY_SIZE(allowed_args)];
    mp_arg_parse_all(n_args, pos_args, kw_args, MP_ARRAY_SIZE(allowed_args), allowed_args, args);

    Mat src = mp_obj_to_mat(args[ARG_src].u_obj);
    Mat dst = mp_obj_to_mat(args[ARG_dst].u_obj);
    int rotateCode = args[ARG_rotateCode].u_int;

    try {
        rotate(src, dst, rotateCode);
    } catch(Exception &e) {
        mp_raise_msg(&mp_type_Exception, MP_ERROR_TEXT(e.what()));
    }

    return mat_to_mp_obj(dst);
}
MP_DEFINE_CONST_FUN_OBJ_KW(cv2_core_rotate_obj, 1, cv2_core_rotate_fun);

STATIC mp_obj_t cv2_core_transpose_fun(size_t n_args, const mp_obj_t *pos_args, mp_map_t *kw_args)
{
    enum { ARG_src, ARG_dst };
    static const mp_arg_t allowed_args[] = {
        { MP_QSTR_src, MP_ARG_REQUIRED | MP_ARG_OBJ, {.u_obj = MP_OBJ_NULL} },
        { MP_QSTR_dst, MP_ARG_OBJ, {.u_obj = mp_const_none} },
    };
    mp_arg_val_t args[MP_ARRAY_SIZE(allowed_args)];
    mp_arg_parse_all(n_args, pos_args, kw_args, MP_ARRAY_SIZE(allowed_args), allowed_args, args);

    Mat src = mp_obj_to_mat(args[ARG_src].u_obj);
    Mat dst = mp_obj_to_mat(args[ARG_dst].u_obj);

    try {
        transpose(src, dst);
    } catch(Exception &e) {
        mp_raise_msg(&mp_type_Exception, MP_ERROR_TEXT(e.what()));
    }

    return mat_to_mp_obj(dst);
}
MP_DEFINE_CONST_FUN_OBJ_KW(cv2_core_transpose_obj, 1, cv2_core_transpose_fun);
