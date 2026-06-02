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

// ---- Arithmetic Operations ----

STATIC mp_obj_t cv2_core_add_fun(size_t n_args, const mp_obj_t *pos_args, mp_map_t *kw_args)
{
    enum { ARG_src1, ARG_src2, ARG_dst, ARG_mask, ARG_dtype };
    static const mp_arg_t allowed_args[] = {
        { MP_QSTR_src1,  MP_ARG_REQUIRED | MP_ARG_OBJ, {.u_obj = MP_OBJ_NULL} },
        { MP_QSTR_src2,  MP_ARG_REQUIRED | MP_ARG_OBJ, {.u_obj = MP_OBJ_NULL} },
        { MP_QSTR_dst,   MP_ARG_OBJ, {.u_obj = mp_const_none} },
        { MP_QSTR_mask,  MP_ARG_OBJ, {.u_obj = mp_const_none} },
        { MP_QSTR_dtype, MP_ARG_INT, {.u_int = -1} },
    };
    mp_arg_val_t args[MP_ARRAY_SIZE(allowed_args)];
    mp_arg_parse_all(n_args, pos_args, kw_args, MP_ARRAY_SIZE(allowed_args), allowed_args, args);

    Mat src1 = mp_obj_to_mat(args[ARG_src1].u_obj);
    Mat src2 = mp_obj_to_mat(args[ARG_src2].u_obj);
    Mat dst = mp_obj_to_mat(args[ARG_dst].u_obj);
    Mat mask = mp_obj_to_mat(args[ARG_mask].u_obj);
    int dtype = args[ARG_dtype].u_int;

    try {
        add(src1, src2, dst, mask.empty() ? noArray() : (InputArray)mask, dtype);
    } catch(Exception &e) {
        mp_raise_msg(&mp_type_Exception, MP_ERROR_TEXT(e.what()));
    }
    return mat_to_mp_obj(dst);
}
MP_DEFINE_CONST_FUN_OBJ_KW(cv2_core_add_obj, 1, cv2_core_add_fun);

STATIC mp_obj_t cv2_core_subtract_fun(size_t n_args, const mp_obj_t *pos_args, mp_map_t *kw_args)
{
    enum { ARG_src1, ARG_src2, ARG_dst, ARG_mask, ARG_dtype };
    static const mp_arg_t allowed_args[] = {
        { MP_QSTR_src1,  MP_ARG_REQUIRED | MP_ARG_OBJ, {.u_obj = MP_OBJ_NULL} },
        { MP_QSTR_src2,  MP_ARG_REQUIRED | MP_ARG_OBJ, {.u_obj = MP_OBJ_NULL} },
        { MP_QSTR_dst,   MP_ARG_OBJ, {.u_obj = mp_const_none} },
        { MP_QSTR_mask,  MP_ARG_OBJ, {.u_obj = mp_const_none} },
        { MP_QSTR_dtype, MP_ARG_INT, {.u_int = -1} },
    };
    mp_arg_val_t args[MP_ARRAY_SIZE(allowed_args)];
    mp_arg_parse_all(n_args, pos_args, kw_args, MP_ARRAY_SIZE(allowed_args), allowed_args, args);

    Mat src1 = mp_obj_to_mat(args[ARG_src1].u_obj);
    Mat src2 = mp_obj_to_mat(args[ARG_src2].u_obj);
    Mat dst = mp_obj_to_mat(args[ARG_dst].u_obj);
    Mat mask = mp_obj_to_mat(args[ARG_mask].u_obj);
    int dtype = args[ARG_dtype].u_int;

    try {
        subtract(src1, src2, dst, mask.empty() ? noArray() : (InputArray)mask, dtype);
    } catch(Exception &e) {
        mp_raise_msg(&mp_type_Exception, MP_ERROR_TEXT(e.what()));
    }
    return mat_to_mp_obj(dst);
}
MP_DEFINE_CONST_FUN_OBJ_KW(cv2_core_subtract_obj, 1, cv2_core_subtract_fun);

STATIC mp_obj_t cv2_core_multiply_fun(size_t n_args, const mp_obj_t *pos_args, mp_map_t *kw_args)
{
    enum { ARG_src1, ARG_src2, ARG_dst, ARG_scale, ARG_dtype };
    static const mp_arg_t allowed_args[] = {
        { MP_QSTR_src1,  MP_ARG_REQUIRED | MP_ARG_OBJ, {.u_obj = MP_OBJ_NULL} },
        { MP_QSTR_src2,  MP_ARG_REQUIRED | MP_ARG_OBJ, {.u_obj = MP_OBJ_NULL} },
        { MP_QSTR_dst,   MP_ARG_OBJ, {.u_obj = mp_const_none} },
        { MP_QSTR_scale, MP_ARG_OBJ, {.u_obj = mp_const_none} },
        { MP_QSTR_dtype, MP_ARG_INT, {.u_int = -1} },
    };
    mp_arg_val_t args[MP_ARRAY_SIZE(allowed_args)];
    mp_arg_parse_all(n_args, pos_args, kw_args, MP_ARRAY_SIZE(allowed_args), allowed_args, args);

    Mat src1 = mp_obj_to_mat(args[ARG_src1].u_obj);
    Mat src2 = mp_obj_to_mat(args[ARG_src2].u_obj);
    Mat dst = mp_obj_to_mat(args[ARG_dst].u_obj);
    double scale = args[ARG_scale].u_obj == mp_const_none ? 1.0 : mp_obj_get_float(args[ARG_scale].u_obj);
    int dtype = args[ARG_dtype].u_int;

    try {
        multiply(src1, src2, dst, scale, dtype);
    } catch(Exception &e) {
        mp_raise_msg(&mp_type_Exception, MP_ERROR_TEXT(e.what()));
    }
    return mat_to_mp_obj(dst);
}
MP_DEFINE_CONST_FUN_OBJ_KW(cv2_core_multiply_obj, 1, cv2_core_multiply_fun);

STATIC mp_obj_t cv2_core_divide_fun(size_t n_args, const mp_obj_t *pos_args, mp_map_t *kw_args)
{
    enum { ARG_src1, ARG_src2, ARG_dst, ARG_scale, ARG_dtype };
    static const mp_arg_t allowed_args[] = {
        { MP_QSTR_src1,  MP_ARG_REQUIRED | MP_ARG_OBJ, {.u_obj = MP_OBJ_NULL} },
        { MP_QSTR_src2,  MP_ARG_REQUIRED | MP_ARG_OBJ, {.u_obj = MP_OBJ_NULL} },
        { MP_QSTR_dst,   MP_ARG_OBJ, {.u_obj = mp_const_none} },
        { MP_QSTR_scale, MP_ARG_OBJ, {.u_obj = mp_const_none} },
        { MP_QSTR_dtype, MP_ARG_INT, {.u_int = -1} },
    };
    mp_arg_val_t args[MP_ARRAY_SIZE(allowed_args)];
    mp_arg_parse_all(n_args, pos_args, kw_args, MP_ARRAY_SIZE(allowed_args), allowed_args, args);

    Mat src1 = mp_obj_to_mat(args[ARG_src1].u_obj);
    Mat src2 = mp_obj_to_mat(args[ARG_src2].u_obj);
    Mat dst = mp_obj_to_mat(args[ARG_dst].u_obj);
    double scale = args[ARG_scale].u_obj == mp_const_none ? 1.0 : mp_obj_get_float(args[ARG_scale].u_obj);
    int dtype = args[ARG_dtype].u_int;

    try {
        divide(src1, src2, dst, scale, dtype);
    } catch(Exception &e) {
        mp_raise_msg(&mp_type_Exception, MP_ERROR_TEXT(e.what()));
    }
    return mat_to_mp_obj(dst);
}
MP_DEFINE_CONST_FUN_OBJ_KW(cv2_core_divide_obj, 1, cv2_core_divide_fun);

STATIC mp_obj_t cv2_core_addWeighted_fun(size_t n_args, const mp_obj_t *pos_args, mp_map_t *kw_args)
{
    enum { ARG_src1, ARG_alpha, ARG_src2, ARG_beta, ARG_gamma, ARG_dst, ARG_dtype };
    static const mp_arg_t allowed_args[] = {
        { MP_QSTR_src1,  MP_ARG_REQUIRED | MP_ARG_OBJ, {.u_obj = MP_OBJ_NULL} },
        { MP_QSTR_alpha, MP_ARG_REQUIRED | MP_ARG_OBJ, {.u_obj = MP_OBJ_NULL} },
        { MP_QSTR_src2,  MP_ARG_REQUIRED | MP_ARG_OBJ, {.u_obj = MP_OBJ_NULL} },
        { MP_QSTR_beta,  MP_ARG_REQUIRED | MP_ARG_OBJ, {.u_obj = MP_OBJ_NULL} },
        { MP_QSTR_gamma, MP_ARG_REQUIRED | MP_ARG_OBJ, {.u_obj = MP_OBJ_NULL} },
        { MP_QSTR_dst,   MP_ARG_OBJ, {.u_obj = mp_const_none} },
        { MP_QSTR_dtype, MP_ARG_INT, {.u_int = -1} },
    };
    mp_arg_val_t args[MP_ARRAY_SIZE(allowed_args)];
    mp_arg_parse_all(n_args, pos_args, kw_args, MP_ARRAY_SIZE(allowed_args), allowed_args, args);

    Mat src1 = mp_obj_to_mat(args[ARG_src1].u_obj);
    double alpha = mp_obj_get_float(args[ARG_alpha].u_obj);
    Mat src2 = mp_obj_to_mat(args[ARG_src2].u_obj);
    double beta = mp_obj_get_float(args[ARG_beta].u_obj);
    double gamma = mp_obj_get_float(args[ARG_gamma].u_obj);
    Mat dst = mp_obj_to_mat(args[ARG_dst].u_obj);
    int dtype = args[ARG_dtype].u_int;

    try {
        addWeighted(src1, alpha, src2, beta, gamma, dst, dtype);
    } catch(Exception &e) {
        mp_raise_msg(&mp_type_Exception, MP_ERROR_TEXT(e.what()));
    }
    return mat_to_mp_obj(dst);
}
MP_DEFINE_CONST_FUN_OBJ_KW(cv2_core_addWeighted_obj, 1, cv2_core_addWeighted_fun);

STATIC mp_obj_t cv2_core_absdiff_fun(size_t n_args, const mp_obj_t *pos_args, mp_map_t *kw_args)
{
    enum { ARG_src1, ARG_src2, ARG_dst };
    static const mp_arg_t allowed_args[] = {
        { MP_QSTR_src1, MP_ARG_REQUIRED | MP_ARG_OBJ, {.u_obj = MP_OBJ_NULL} },
        { MP_QSTR_src2, MP_ARG_REQUIRED | MP_ARG_OBJ, {.u_obj = MP_OBJ_NULL} },
        { MP_QSTR_dst,  MP_ARG_OBJ, {.u_obj = mp_const_none} },
    };
    mp_arg_val_t args[MP_ARRAY_SIZE(allowed_args)];
    mp_arg_parse_all(n_args, pos_args, kw_args, MP_ARRAY_SIZE(allowed_args), allowed_args, args);

    Mat src1 = mp_obj_to_mat(args[ARG_src1].u_obj);
    Mat src2 = mp_obj_to_mat(args[ARG_src2].u_obj);
    Mat dst = mp_obj_to_mat(args[ARG_dst].u_obj);

    try {
        absdiff(src1, src2, dst);
    } catch(Exception &e) {
        mp_raise_msg(&mp_type_Exception, MP_ERROR_TEXT(e.what()));
    }
    return mat_to_mp_obj(dst);
}
MP_DEFINE_CONST_FUN_OBJ_KW(cv2_core_absdiff_obj, 1, cv2_core_absdiff_fun);

// ---- Bitwise Operations ----

STATIC mp_obj_t cv2_core_bitwise_and_fun(size_t n_args, const mp_obj_t *pos_args, mp_map_t *kw_args)
{
    enum { ARG_src1, ARG_src2, ARG_dst, ARG_mask };
    static const mp_arg_t allowed_args[] = {
        { MP_QSTR_src1, MP_ARG_REQUIRED | MP_ARG_OBJ, {.u_obj = MP_OBJ_NULL} },
        { MP_QSTR_src2, MP_ARG_REQUIRED | MP_ARG_OBJ, {.u_obj = MP_OBJ_NULL} },
        { MP_QSTR_dst,  MP_ARG_OBJ, {.u_obj = mp_const_none} },
        { MP_QSTR_mask, MP_ARG_OBJ, {.u_obj = mp_const_none} },
    };
    mp_arg_val_t args[MP_ARRAY_SIZE(allowed_args)];
    mp_arg_parse_all(n_args, pos_args, kw_args, MP_ARRAY_SIZE(allowed_args), allowed_args, args);

    Mat src1 = mp_obj_to_mat(args[ARG_src1].u_obj);
    Mat src2 = mp_obj_to_mat(args[ARG_src2].u_obj);
    Mat dst = mp_obj_to_mat(args[ARG_dst].u_obj);
    Mat mask = mp_obj_to_mat(args[ARG_mask].u_obj);

    try {
        bitwise_and(src1, src2, dst, mask.empty() ? noArray() : (InputArray)mask);
    } catch(Exception &e) {
        mp_raise_msg(&mp_type_Exception, MP_ERROR_TEXT(e.what()));
    }
    return mat_to_mp_obj(dst);
}
MP_DEFINE_CONST_FUN_OBJ_KW(cv2_core_bitwise_and_obj, 1, cv2_core_bitwise_and_fun);

STATIC mp_obj_t cv2_core_bitwise_or_fun(size_t n_args, const mp_obj_t *pos_args, mp_map_t *kw_args)
{
    enum { ARG_src1, ARG_src2, ARG_dst, ARG_mask };
    static const mp_arg_t allowed_args[] = {
        { MP_QSTR_src1, MP_ARG_REQUIRED | MP_ARG_OBJ, {.u_obj = MP_OBJ_NULL} },
        { MP_QSTR_src2, MP_ARG_REQUIRED | MP_ARG_OBJ, {.u_obj = MP_OBJ_NULL} },
        { MP_QSTR_dst,  MP_ARG_OBJ, {.u_obj = mp_const_none} },
        { MP_QSTR_mask, MP_ARG_OBJ, {.u_obj = mp_const_none} },
    };
    mp_arg_val_t args[MP_ARRAY_SIZE(allowed_args)];
    mp_arg_parse_all(n_args, pos_args, kw_args, MP_ARRAY_SIZE(allowed_args), allowed_args, args);

    Mat src1 = mp_obj_to_mat(args[ARG_src1].u_obj);
    Mat src2 = mp_obj_to_mat(args[ARG_src2].u_obj);
    Mat dst = mp_obj_to_mat(args[ARG_dst].u_obj);
    Mat mask = mp_obj_to_mat(args[ARG_mask].u_obj);

    try {
        bitwise_or(src1, src2, dst, mask.empty() ? noArray() : (InputArray)mask);
    } catch(Exception &e) {
        mp_raise_msg(&mp_type_Exception, MP_ERROR_TEXT(e.what()));
    }
    return mat_to_mp_obj(dst);
}
MP_DEFINE_CONST_FUN_OBJ_KW(cv2_core_bitwise_or_obj, 1, cv2_core_bitwise_or_fun);

STATIC mp_obj_t cv2_core_bitwise_xor_fun(size_t n_args, const mp_obj_t *pos_args, mp_map_t *kw_args)
{
    enum { ARG_src1, ARG_src2, ARG_dst, ARG_mask };
    static const mp_arg_t allowed_args[] = {
        { MP_QSTR_src1, MP_ARG_REQUIRED | MP_ARG_OBJ, {.u_obj = MP_OBJ_NULL} },
        { MP_QSTR_src2, MP_ARG_REQUIRED | MP_ARG_OBJ, {.u_obj = MP_OBJ_NULL} },
        { MP_QSTR_dst,  MP_ARG_OBJ, {.u_obj = mp_const_none} },
        { MP_QSTR_mask, MP_ARG_OBJ, {.u_obj = mp_const_none} },
    };
    mp_arg_val_t args[MP_ARRAY_SIZE(allowed_args)];
    mp_arg_parse_all(n_args, pos_args, kw_args, MP_ARRAY_SIZE(allowed_args), allowed_args, args);

    Mat src1 = mp_obj_to_mat(args[ARG_src1].u_obj);
    Mat src2 = mp_obj_to_mat(args[ARG_src2].u_obj);
    Mat dst = mp_obj_to_mat(args[ARG_dst].u_obj);
    Mat mask = mp_obj_to_mat(args[ARG_mask].u_obj);

    try {
        bitwise_xor(src1, src2, dst, mask.empty() ? noArray() : (InputArray)mask);
    } catch(Exception &e) {
        mp_raise_msg(&mp_type_Exception, MP_ERROR_TEXT(e.what()));
    }
    return mat_to_mp_obj(dst);
}
MP_DEFINE_CONST_FUN_OBJ_KW(cv2_core_bitwise_xor_obj, 1, cv2_core_bitwise_xor_fun);

STATIC mp_obj_t cv2_core_bitwise_not_fun(size_t n_args, const mp_obj_t *pos_args, mp_map_t *kw_args)
{
    enum { ARG_src, ARG_dst, ARG_mask };
    static const mp_arg_t allowed_args[] = {
        { MP_QSTR_src,  MP_ARG_REQUIRED | MP_ARG_OBJ, {.u_obj = MP_OBJ_NULL} },
        { MP_QSTR_dst,  MP_ARG_OBJ, {.u_obj = mp_const_none} },
        { MP_QSTR_mask, MP_ARG_OBJ, {.u_obj = mp_const_none} },
    };
    mp_arg_val_t args[MP_ARRAY_SIZE(allowed_args)];
    mp_arg_parse_all(n_args, pos_args, kw_args, MP_ARRAY_SIZE(allowed_args), allowed_args, args);

    Mat src = mp_obj_to_mat(args[ARG_src].u_obj);
    Mat dst = mp_obj_to_mat(args[ARG_dst].u_obj);
    Mat mask = mp_obj_to_mat(args[ARG_mask].u_obj);

    try {
        bitwise_not(src, dst, mask.empty() ? noArray() : (InputArray)mask);
    } catch(Exception &e) {
        mp_raise_msg(&mp_type_Exception, MP_ERROR_TEXT(e.what()));
    }
    return mat_to_mp_obj(dst);
}
MP_DEFINE_CONST_FUN_OBJ_KW(cv2_core_bitwise_not_obj, 1, cv2_core_bitwise_not_fun);

// ---- Channel Operations ----

STATIC mp_obj_t cv2_core_split_fun(size_t n_args, const mp_obj_t *pos_args, mp_map_t *kw_args)
{
    enum { ARG_m };
    static const mp_arg_t allowed_args[] = {
        { MP_QSTR_m, MP_ARG_REQUIRED | MP_ARG_OBJ, {.u_obj = MP_OBJ_NULL} },
    };
    mp_arg_val_t args[MP_ARRAY_SIZE(allowed_args)];
    mp_arg_parse_all(n_args, pos_args, kw_args, MP_ARRAY_SIZE(allowed_args), allowed_args, args);

    Mat m = mp_obj_to_mat(args[ARG_m].u_obj);
    std::vector<Mat> channels;
    try {
        split(m, channels);
    } catch(Exception &e) {
        mp_raise_msg(&mp_type_Exception, MP_ERROR_TEXT(e.what()));
    }

    mp_obj_t result = mp_obj_new_list(0, NULL);
    for(auto &ch : channels)
        mp_obj_list_append(result, mat_to_mp_obj(ch));
    return result;
}
MP_DEFINE_CONST_FUN_OBJ_KW(cv2_core_split_obj, 1, cv2_core_split_fun);

STATIC mp_obj_t cv2_core_merge_fun(size_t n_args, const mp_obj_t *pos_args, mp_map_t *kw_args)
{
    enum { ARG_mv };
    static const mp_arg_t allowed_args[] = {
        { MP_QSTR_mv, MP_ARG_REQUIRED | MP_ARG_OBJ, {.u_obj = MP_OBJ_NULL} },
    };
    mp_arg_val_t args[MP_ARRAY_SIZE(allowed_args)];
    mp_arg_parse_all(n_args, pos_args, kw_args, MP_ARRAY_SIZE(allowed_args), allowed_args, args);

    mp_obj_t mv_obj = args[ARG_mv].u_obj;
    if(!mp_obj_is_type(mv_obj, &mp_type_list) && !mp_obj_is_type(mv_obj, &mp_type_tuple))
        mp_raise_TypeError(MP_ERROR_TEXT("mv must be a list or tuple of arrays"));

    size_t n;
    mp_obj_t *items;
    mp_obj_get_array(mv_obj, &n, &items);

    std::vector<Mat> channels(n);
    for(size_t i = 0; i < n; i++)
        channels[i] = mp_obj_to_mat(items[i]);

    Mat dst;
    try {
        merge(channels, dst);
    } catch(Exception &e) {
        mp_raise_msg(&mp_type_Exception, MP_ERROR_TEXT(e.what()));
    }
    return mat_to_mp_obj(dst);
}
MP_DEFINE_CONST_FUN_OBJ_KW(cv2_core_merge_obj, 1, cv2_core_merge_fun);

// ---- Statistics ----

STATIC mp_obj_t cv2_core_mean_fun(size_t n_args, const mp_obj_t *pos_args, mp_map_t *kw_args)
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

    Scalar s;
    try {
        s = mean(src, mask.empty() ? noArray() : (InputArray)mask);
    } catch(Exception &e) {
        mp_raise_msg(&mp_type_Exception, MP_ERROR_TEXT(e.what()));
    }

    int channels = src.channels();
    mp_obj_t result = mp_obj_new_list(0, NULL);
    for(int i = 0; i < channels && i < 4; i++)
        mp_obj_list_append(result, mp_obj_new_float(s[i]));
    return result;
}
MP_DEFINE_CONST_FUN_OBJ_KW(cv2_core_mean_obj, 1, cv2_core_mean_fun);

// ---- Normalization ----

STATIC mp_obj_t cv2_core_normalize_fun(size_t n_args, const mp_obj_t *pos_args, mp_map_t *kw_args)
{
    enum { ARG_src, ARG_dst, ARG_alpha, ARG_beta, ARG_norm_type, ARG_dtype, ARG_mask };
    static const mp_arg_t allowed_args[] = {
        { MP_QSTR_src,       MP_ARG_REQUIRED | MP_ARG_OBJ, {.u_obj = MP_OBJ_NULL} },
        { MP_QSTR_dst,       MP_ARG_OBJ, {.u_obj = mp_const_none} },
        { MP_QSTR_alpha,     MP_ARG_OBJ, {.u_obj = mp_const_none} },
        { MP_QSTR_beta,      MP_ARG_OBJ, {.u_obj = mp_const_none} },
        { MP_QSTR_norm_type, MP_ARG_INT, {.u_int = cv::NORM_L2} },
        { MP_QSTR_dtype,     MP_ARG_INT, {.u_int = -1} },
        { MP_QSTR_mask,      MP_ARG_OBJ, {.u_obj = mp_const_none} },
    };
    mp_arg_val_t args[MP_ARRAY_SIZE(allowed_args)];
    mp_arg_parse_all(n_args, pos_args, kw_args, MP_ARRAY_SIZE(allowed_args), allowed_args, args);

    Mat src = mp_obj_to_mat(args[ARG_src].u_obj);
    Mat dst = mp_obj_to_mat(args[ARG_dst].u_obj);
    double alpha = args[ARG_alpha].u_obj == mp_const_none ? 1.0 : mp_obj_get_float(args[ARG_alpha].u_obj);
    double beta = args[ARG_beta].u_obj == mp_const_none ? 0.0 : mp_obj_get_float(args[ARG_beta].u_obj);
    int norm_type = args[ARG_norm_type].u_int;
    int dtype = args[ARG_dtype].u_int;
    Mat mask = mp_obj_to_mat(args[ARG_mask].u_obj);

    try {
        normalize(src, dst, alpha, beta, norm_type, dtype, mask.empty() ? noArray() : (InputArray)mask);
    } catch(Exception &e) {
        mp_raise_msg(&mp_type_Exception, MP_ERROR_TEXT(e.what()));
    }
    return mat_to_mp_obj(dst);
}
MP_DEFINE_CONST_FUN_OBJ_KW(cv2_core_normalize_obj, 1, cv2_core_normalize_fun);

// ---- Comparison ----

STATIC mp_obj_t cv2_core_compare_fun(size_t n_args, const mp_obj_t *pos_args, mp_map_t *kw_args)
{
    enum { ARG_src1, ARG_src2, ARG_cmpop, ARG_dst };
    static const mp_arg_t allowed_args[] = {
        { MP_QSTR_src1,  MP_ARG_REQUIRED | MP_ARG_OBJ, {.u_obj = MP_OBJ_NULL} },
        { MP_QSTR_src2,  MP_ARG_REQUIRED | MP_ARG_OBJ, {.u_obj = MP_OBJ_NULL} },
        { MP_QSTR_cmpop, MP_ARG_REQUIRED | MP_ARG_INT, {.u_int = 0} },
        { MP_QSTR_dst,   MP_ARG_OBJ, {.u_obj = mp_const_none} },
    };
    mp_arg_val_t args[MP_ARRAY_SIZE(allowed_args)];
    mp_arg_parse_all(n_args, pos_args, kw_args, MP_ARRAY_SIZE(allowed_args), allowed_args, args);

    Mat src1 = mp_obj_to_mat(args[ARG_src1].u_obj);
    Mat src2 = mp_obj_to_mat(args[ARG_src2].u_obj);
    Mat dst = mp_obj_to_mat(args[ARG_dst].u_obj);
    int cmpop = args[ARG_cmpop].u_int;

    try {
        compare(src1, src2, dst, cmpop);
    } catch(Exception &e) {
        mp_raise_msg(&mp_type_Exception, MP_ERROR_TEXT(e.what()));
    }
    return mat_to_mp_obj(dst);
}
MP_DEFINE_CONST_FUN_OBJ_KW(cv2_core_compare_obj, 1, cv2_core_compare_fun);
