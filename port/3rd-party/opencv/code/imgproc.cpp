#include "imgproc.h"
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
#include <exception>

using namespace cv;

// ---- Image Filtering ----

STATIC mp_obj_t cv2_imgproc_Canny_fun(size_t n_args, const mp_obj_t *pos_args, mp_map_t *kw_args)
{
    enum { ARG_image, ARG_threshold1, ARG_threshold2, ARG_edges, ARG_apertureSize, ARG_L2gradient };
    static const mp_arg_t allowed_args[] = {
        { MP_QSTR_image,        MP_ARG_REQUIRED | MP_ARG_OBJ, {.u_obj = MP_OBJ_NULL} },
        { MP_QSTR_threshold1,   MP_ARG_REQUIRED | MP_ARG_OBJ, {.u_obj = MP_OBJ_NULL} },
        { MP_QSTR_threshold2,   MP_ARG_REQUIRED | MP_ARG_OBJ, {.u_obj = MP_OBJ_NULL} },
        { MP_QSTR_edges,        MP_ARG_OBJ, {.u_obj = mp_const_none} },
        { MP_QSTR_apertureSize, MP_ARG_INT, {.u_int = 3} },
        { MP_QSTR_L2gradient,   MP_ARG_BOOL, {.u_bool = false} },
    };
    mp_arg_val_t args[MP_ARRAY_SIZE(allowed_args)];
    mp_arg_parse_all(n_args, pos_args, kw_args, MP_ARRAY_SIZE(allowed_args), allowed_args, args);

    Mat image = mp_obj_to_mat(args[ARG_image].u_obj);
    Mat edges = mp_obj_to_mat(args[ARG_edges].u_obj);
    double threshold1 = mp_obj_get_float(args[ARG_threshold1].u_obj);
    double threshold2 = mp_obj_get_float(args[ARG_threshold2].u_obj);
    int apertureSize = args[ARG_apertureSize].u_int;
    bool L2gradient = args[ARG_L2gradient].u_bool;

    try {
        Canny(image, edges, threshold1, threshold2, apertureSize, L2gradient);
    } catch(Exception &e) {
        mp_raise_msg(&mp_type_Exception, MP_ERROR_TEXT(e.what()));
    }
    return mat_to_mp_obj(edges);
}
MP_DEFINE_CONST_FUN_OBJ_KW(cv2_imgproc_Canny_obj, 1, cv2_imgproc_Canny_fun);

STATIC mp_obj_t cv2_imgproc_GaussianBlur_fun(size_t n_args, const mp_obj_t *pos_args, mp_map_t *kw_args)
{
    enum { ARG_src, ARG_ksize, ARG_sigmaX, ARG_dst, ARG_sigmaY, ARG_borderType };
    static const mp_arg_t allowed_args[] = {
        { MP_QSTR_src,        MP_ARG_REQUIRED | MP_ARG_OBJ, {.u_obj = MP_OBJ_NULL} },
        { MP_QSTR_ksize,      MP_ARG_REQUIRED | MP_ARG_OBJ, {.u_obj = MP_OBJ_NULL} },
        { MP_QSTR_sigmaX,     MP_ARG_REQUIRED | MP_ARG_OBJ, {.u_obj = MP_OBJ_NULL} },
        { MP_QSTR_dst,        MP_ARG_OBJ, {.u_obj = mp_const_none} },
        { MP_QSTR_sigmaY,     MP_ARG_OBJ, {.u_obj = mp_const_none} },
        { MP_QSTR_borderType, MP_ARG_INT, {.u_int = BORDER_DEFAULT} },
    };
    mp_arg_val_t args[MP_ARRAY_SIZE(allowed_args)];
    mp_arg_parse_all(n_args, pos_args, kw_args, MP_ARRAY_SIZE(allowed_args), allowed_args, args);

    Mat src = mp_obj_to_mat(args[ARG_src].u_obj);
    Size ksize = mp_obj_to_size(args[ARG_ksize].u_obj);
    double sigmaX = mp_obj_get_float(args[ARG_sigmaX].u_obj);
    Mat dst = mp_obj_to_mat(args[ARG_dst].u_obj);
    double sigmaY = args[ARG_sigmaY].u_obj == mp_const_none ? 0 : mp_obj_get_float(args[ARG_sigmaY].u_obj);
    int borderType = args[ARG_borderType].u_int;

    try {
        GaussianBlur(src, dst, ksize, sigmaX, sigmaY, borderType);
    } catch(Exception &e) {
        mp_raise_msg(&mp_type_Exception, MP_ERROR_TEXT(e.what()));
    }
    return mat_to_mp_obj(dst);
}
MP_DEFINE_CONST_FUN_OBJ_KW(cv2_imgproc_GaussianBlur_obj, 1, cv2_imgproc_GaussianBlur_fun);

STATIC mp_obj_t cv2_imgproc_blur_fun(size_t n_args, const mp_obj_t *pos_args, mp_map_t *kw_args)
{
    enum { ARG_src, ARG_ksize, ARG_dst, ARG_anchor, ARG_borderType };
    static const mp_arg_t allowed_args[] = {
        { MP_QSTR_src,        MP_ARG_REQUIRED | MP_ARG_OBJ, {.u_obj = MP_OBJ_NULL} },
        { MP_QSTR_ksize,      MP_ARG_REQUIRED | MP_ARG_OBJ, {.u_obj = MP_OBJ_NULL} },
        { MP_QSTR_dst,        MP_ARG_OBJ, {.u_obj = mp_const_none} },
        { MP_QSTR_anchor,     MP_ARG_OBJ, {.u_obj = mp_const_none} },
        { MP_QSTR_borderType, MP_ARG_INT, {.u_int = BORDER_DEFAULT} },
    };
    mp_arg_val_t args[MP_ARRAY_SIZE(allowed_args)];
    mp_arg_parse_all(n_args, pos_args, kw_args, MP_ARRAY_SIZE(allowed_args), allowed_args, args);

    Mat src = mp_obj_to_mat(args[ARG_src].u_obj);
    Size ksize = mp_obj_to_size(args[ARG_ksize].u_obj);
    Mat dst = mp_obj_to_mat(args[ARG_dst].u_obj);
    Point anchor = args[ARG_anchor].u_obj == mp_const_none ? Point(-1,-1) : mp_obj_to_point(args[ARG_anchor].u_obj);
    int borderType = args[ARG_borderType].u_int;

    try {
        blur(src, dst, ksize, anchor, borderType);
    } catch(Exception &e) {
        mp_raise_msg(&mp_type_Exception, MP_ERROR_TEXT(e.what()));
    }
    return mat_to_mp_obj(dst);
}
MP_DEFINE_CONST_FUN_OBJ_KW(cv2_imgproc_blur_obj, 1, cv2_imgproc_blur_fun);

STATIC mp_obj_t cv2_imgproc_medianBlur_fun(size_t n_args, const mp_obj_t *pos_args, mp_map_t *kw_args)
{
    enum { ARG_src, ARG_ksize, ARG_dst };
    static const mp_arg_t allowed_args[] = {
        { MP_QSTR_src,   MP_ARG_REQUIRED | MP_ARG_OBJ, {.u_obj = MP_OBJ_NULL} },
        { MP_QSTR_ksize, MP_ARG_REQUIRED | MP_ARG_INT, {.u_int = 0} },
        { MP_QSTR_dst,   MP_ARG_OBJ, {.u_obj = mp_const_none} },
    };
    mp_arg_val_t args[MP_ARRAY_SIZE(allowed_args)];
    mp_arg_parse_all(n_args, pos_args, kw_args, MP_ARRAY_SIZE(allowed_args), allowed_args, args);

    Mat src = mp_obj_to_mat(args[ARG_src].u_obj);
    int ksize = args[ARG_ksize].u_int;
    Mat dst = mp_obj_to_mat(args[ARG_dst].u_obj);

    try {
        medianBlur(src, dst, ksize);
    } catch(Exception &e) {
        mp_raise_msg(&mp_type_Exception, MP_ERROR_TEXT(e.what()));
    }
    return mat_to_mp_obj(dst);
}
MP_DEFINE_CONST_FUN_OBJ_KW(cv2_imgproc_medianBlur_obj, 1, cv2_imgproc_medianBlur_fun);

STATIC mp_obj_t cv2_imgproc_bilateralFilter_fun(size_t n_args, const mp_obj_t *pos_args, mp_map_t *kw_args)
{
    enum { ARG_src, ARG_d, ARG_sigmaColor, ARG_sigmaSpace, ARG_dst, ARG_borderType };
    static const mp_arg_t allowed_args[] = {
        { MP_QSTR_src,         MP_ARG_REQUIRED | MP_ARG_OBJ, {.u_obj = MP_OBJ_NULL} },
        { MP_QSTR_d,           MP_ARG_REQUIRED | MP_ARG_INT, {.u_int = 0} },
        { MP_QSTR_sigmaColor,  MP_ARG_REQUIRED | MP_ARG_OBJ, {.u_obj = MP_OBJ_NULL} },
        { MP_QSTR_sigmaSpace,  MP_ARG_REQUIRED | MP_ARG_OBJ, {.u_obj = MP_OBJ_NULL} },
        { MP_QSTR_dst,         MP_ARG_OBJ, {.u_obj = mp_const_none} },
        { MP_QSTR_borderType,  MP_ARG_INT, {.u_int = BORDER_DEFAULT} },
    };
    mp_arg_val_t args[MP_ARRAY_SIZE(allowed_args)];
    mp_arg_parse_all(n_args, pos_args, kw_args, MP_ARRAY_SIZE(allowed_args), allowed_args, args);

    Mat src = mp_obj_to_mat(args[ARG_src].u_obj);
    int d = args[ARG_d].u_int;
    double sigmaColor = mp_obj_get_float(args[ARG_sigmaColor].u_obj);
    double sigmaSpace = mp_obj_get_float(args[ARG_sigmaSpace].u_obj);
    Mat dst = mp_obj_to_mat(args[ARG_dst].u_obj);
    int borderType = args[ARG_borderType].u_int;

    try {
        bilateralFilter(src, dst, d, sigmaColor, sigmaSpace, borderType);
    } catch(Exception &e) {
        mp_raise_msg(&mp_type_Exception, MP_ERROR_TEXT(e.what()));
    }
    return mat_to_mp_obj(dst);
}
MP_DEFINE_CONST_FUN_OBJ_KW(cv2_imgproc_bilateralFilter_obj, 1, cv2_imgproc_bilateralFilter_fun);

STATIC mp_obj_t cv2_imgproc_boxFilter_fun(size_t n_args, const mp_obj_t *pos_args, mp_map_t *kw_args)
{
    enum { ARG_src, ARG_ddepth, ARG_ksize, ARG_dst, ARG_anchor, ARG_normalize, ARG_borderType };
    static const mp_arg_t allowed_args[] = {
        { MP_QSTR_src,        MP_ARG_REQUIRED | MP_ARG_OBJ, {.u_obj = MP_OBJ_NULL} },
        { MP_QSTR_ddepth,     MP_ARG_REQUIRED | MP_ARG_INT, {.u_int = -1} },
        { MP_QSTR_ksize,      MP_ARG_REQUIRED | MP_ARG_OBJ, {.u_obj = MP_OBJ_NULL} },
        { MP_QSTR_dst,        MP_ARG_OBJ, {.u_obj = mp_const_none} },
        { MP_QSTR_anchor,     MP_ARG_OBJ, {.u_obj = mp_const_none} },
        { MP_QSTR_normalize,  MP_ARG_BOOL, {.u_bool = true} },
        { MP_QSTR_borderType, MP_ARG_INT, {.u_int = BORDER_DEFAULT} },
    };
    mp_arg_val_t args[MP_ARRAY_SIZE(allowed_args)];
    mp_arg_parse_all(n_args, pos_args, kw_args, MP_ARRAY_SIZE(allowed_args), allowed_args, args);

    Mat src = mp_obj_to_mat(args[ARG_src].u_obj);
    int ddepth = args[ARG_ddepth].u_int;
    Size ksize = mp_obj_to_size(args[ARG_ksize].u_obj);
    Mat dst = mp_obj_to_mat(args[ARG_dst].u_obj);
    Point anchor = args[ARG_anchor].u_obj == mp_const_none ? Point(-1,-1) : mp_obj_to_point(args[ARG_anchor].u_obj);
    bool normalize = args[ARG_normalize].u_bool;
    int borderType = args[ARG_borderType].u_int;

    try {
        boxFilter(src, dst, ddepth, ksize, anchor, normalize, borderType);
    } catch(Exception &e) {
        mp_raise_msg(&mp_type_Exception, MP_ERROR_TEXT(e.what()));
    }
    return mat_to_mp_obj(dst);
}
MP_DEFINE_CONST_FUN_OBJ_KW(cv2_imgproc_boxFilter_obj, 1, cv2_imgproc_boxFilter_fun);

STATIC mp_obj_t cv2_imgproc_filter2D_fun(size_t n_args, const mp_obj_t *pos_args, mp_map_t *kw_args)
{
    enum { ARG_src, ARG_ddepth, ARG_kernel, ARG_dst, ARG_anchor, ARG_delta, ARG_borderType };
    static const mp_arg_t allowed_args[] = {
        { MP_QSTR_src,        MP_ARG_REQUIRED | MP_ARG_OBJ, {.u_obj = MP_OBJ_NULL} },
        { MP_QSTR_ddepth,     MP_ARG_REQUIRED | MP_ARG_INT, {.u_int = -1} },
        { MP_QSTR_kernel,     MP_ARG_REQUIRED | MP_ARG_OBJ, {.u_obj = MP_OBJ_NULL} },
        { MP_QSTR_dst,        MP_ARG_OBJ, {.u_obj = mp_const_none} },
        { MP_QSTR_anchor,     MP_ARG_OBJ, {.u_obj = mp_const_none} },
        { MP_QSTR_delta,      MP_ARG_OBJ, {.u_obj = mp_const_none} },
        { MP_QSTR_borderType, MP_ARG_INT, {.u_int = BORDER_DEFAULT} },
    };
    mp_arg_val_t args[MP_ARRAY_SIZE(allowed_args)];
    mp_arg_parse_all(n_args, pos_args, kw_args, MP_ARRAY_SIZE(allowed_args), allowed_args, args);

    Mat src = mp_obj_to_mat(args[ARG_src].u_obj);
    int ddepth = args[ARG_ddepth].u_int;
    Mat kernel = mp_obj_to_mat(args[ARG_kernel].u_obj);
    Mat dst = mp_obj_to_mat(args[ARG_dst].u_obj);
    Point anchor = args[ARG_anchor].u_obj == mp_const_none ? Point(-1,-1) : mp_obj_to_point(args[ARG_anchor].u_obj);
    double delta = args[ARG_delta].u_obj == mp_const_none ? 0 : mp_obj_get_float(args[ARG_delta].u_obj);
    int borderType = args[ARG_borderType].u_int;

    try {
        filter2D(src, dst, ddepth, kernel, anchor, delta, borderType);
    } catch(Exception &e) {
        mp_raise_msg(&mp_type_Exception, MP_ERROR_TEXT(e.what()));
    }
    return mat_to_mp_obj(dst);
}
MP_DEFINE_CONST_FUN_OBJ_KW(cv2_imgproc_filter2D_obj, 1, cv2_imgproc_filter2D_fun);

STATIC mp_obj_t cv2_imgproc_Sobel_fun(size_t n_args, const mp_obj_t *pos_args, mp_map_t *kw_args)
{
    enum { ARG_src, ARG_ddepth, ARG_dx, ARG_dy, ARG_dst, ARG_ksize, ARG_scale, ARG_delta, ARG_borderType };
    static const mp_arg_t allowed_args[] = {
        { MP_QSTR_src,        MP_ARG_REQUIRED | MP_ARG_OBJ, {.u_obj = MP_OBJ_NULL} },
        { MP_QSTR_ddepth,     MP_ARG_REQUIRED | MP_ARG_INT, {.u_int = -1} },
        { MP_QSTR_dx,         MP_ARG_REQUIRED | MP_ARG_INT, {.u_int = 0} },
        { MP_QSTR_dy,         MP_ARG_REQUIRED | MP_ARG_INT, {.u_int = 0} },
        { MP_QSTR_dst,        MP_ARG_OBJ, {.u_obj = mp_const_none} },
        { MP_QSTR_ksize,      MP_ARG_INT, {.u_int = 3} },
        { MP_QSTR_scale,      MP_ARG_OBJ, {.u_obj = mp_const_none} },
        { MP_QSTR_delta,      MP_ARG_OBJ, {.u_obj = mp_const_none} },
        { MP_QSTR_borderType, MP_ARG_INT, {.u_int = BORDER_DEFAULT} },
    };
    mp_arg_val_t args[MP_ARRAY_SIZE(allowed_args)];
    mp_arg_parse_all(n_args, pos_args, kw_args, MP_ARRAY_SIZE(allowed_args), allowed_args, args);

    Mat src = mp_obj_to_mat(args[ARG_src].u_obj);
    int ddepth = args[ARG_ddepth].u_int;
    int dx = args[ARG_dx].u_int;
    int dy = args[ARG_dy].u_int;
    Mat dst = mp_obj_to_mat(args[ARG_dst].u_obj);
    int ksize = args[ARG_ksize].u_int;
    double scale = args[ARG_scale].u_obj == mp_const_none ? 1 : mp_obj_get_float(args[ARG_scale].u_obj);
    double delta = args[ARG_delta].u_obj == mp_const_none ? 0 : mp_obj_get_float(args[ARG_delta].u_obj);
    int borderType = args[ARG_borderType].u_int;

    try {
        Sobel(src, dst, ddepth, dx, dy, ksize, scale, delta, borderType);
    } catch(Exception &e) {
        mp_raise_msg(&mp_type_Exception, MP_ERROR_TEXT(e.what()));
    }
    return mat_to_mp_obj(dst);
}
MP_DEFINE_CONST_FUN_OBJ_KW(cv2_imgproc_Sobel_obj, 1, cv2_imgproc_Sobel_fun);

STATIC mp_obj_t cv2_imgproc_Scharr_fun(size_t n_args, const mp_obj_t *pos_args, mp_map_t *kw_args)
{
    enum { ARG_src, ARG_ddepth, ARG_dx, ARG_dy, ARG_dst, ARG_scale, ARG_delta, ARG_borderType };
    static const mp_arg_t allowed_args[] = {
        { MP_QSTR_src,        MP_ARG_REQUIRED | MP_ARG_OBJ, {.u_obj = MP_OBJ_NULL} },
        { MP_QSTR_ddepth,     MP_ARG_REQUIRED | MP_ARG_INT, {.u_int = -1} },
        { MP_QSTR_dx,         MP_ARG_REQUIRED | MP_ARG_INT, {.u_int = 0} },
        { MP_QSTR_dy,         MP_ARG_REQUIRED | MP_ARG_INT, {.u_int = 0} },
        { MP_QSTR_dst,        MP_ARG_OBJ, {.u_obj = mp_const_none} },
        { MP_QSTR_scale,      MP_ARG_OBJ, {.u_obj = mp_const_none} },
        { MP_QSTR_delta,      MP_ARG_OBJ, {.u_obj = mp_const_none} },
        { MP_QSTR_borderType, MP_ARG_INT, {.u_int = BORDER_DEFAULT} },
    };
    mp_arg_val_t args[MP_ARRAY_SIZE(allowed_args)];
    mp_arg_parse_all(n_args, pos_args, kw_args, MP_ARRAY_SIZE(allowed_args), allowed_args, args);

    Mat src = mp_obj_to_mat(args[ARG_src].u_obj);
    int ddepth = args[ARG_ddepth].u_int;
    int dx = args[ARG_dx].u_int;
    int dy = args[ARG_dy].u_int;
    Mat dst = mp_obj_to_mat(args[ARG_dst].u_obj);
    double scale = args[ARG_scale].u_obj == mp_const_none ? 1 : mp_obj_get_float(args[ARG_scale].u_obj);
    double delta = args[ARG_delta].u_obj == mp_const_none ? 0 : mp_obj_get_float(args[ARG_delta].u_obj);
    int borderType = args[ARG_borderType].u_int;

    try {
        Scharr(src, dst, ddepth, dx, dy, scale, delta, borderType);
    } catch(Exception &e) {
        mp_raise_msg(&mp_type_Exception, MP_ERROR_TEXT(e.what()));
    }
    return mat_to_mp_obj(dst);
}
MP_DEFINE_CONST_FUN_OBJ_KW(cv2_imgproc_Scharr_obj, 1, cv2_imgproc_Scharr_fun);

STATIC mp_obj_t cv2_imgproc_Laplacian_fun(size_t n_args, const mp_obj_t *pos_args, mp_map_t *kw_args)
{
    enum { ARG_src, ARG_ddepth, ARG_dst, ARG_ksize, ARG_scale, ARG_delta, ARG_borderType };
    static const mp_arg_t allowed_args[] = {
        { MP_QSTR_src,        MP_ARG_REQUIRED | MP_ARG_OBJ, {.u_obj = MP_OBJ_NULL} },
        { MP_QSTR_ddepth,     MP_ARG_REQUIRED | MP_ARG_INT, {.u_int = -1} },
        { MP_QSTR_dst,        MP_ARG_OBJ, {.u_obj = mp_const_none} },
        { MP_QSTR_ksize,      MP_ARG_INT, {.u_int = 1} },
        { MP_QSTR_scale,      MP_ARG_OBJ, {.u_obj = mp_const_none} },
        { MP_QSTR_delta,      MP_ARG_OBJ, {.u_obj = mp_const_none} },
        { MP_QSTR_borderType, MP_ARG_INT, {.u_int = BORDER_DEFAULT} },
    };
    mp_arg_val_t args[MP_ARRAY_SIZE(allowed_args)];
    mp_arg_parse_all(n_args, pos_args, kw_args, MP_ARRAY_SIZE(allowed_args), allowed_args, args);

    Mat src = mp_obj_to_mat(args[ARG_src].u_obj);
    int ddepth = args[ARG_ddepth].u_int;
    Mat dst = mp_obj_to_mat(args[ARG_dst].u_obj);
    int ksize = args[ARG_ksize].u_int;
    double scale = args[ARG_scale].u_obj == mp_const_none ? 1 : mp_obj_get_float(args[ARG_scale].u_obj);
    double delta = args[ARG_delta].u_obj == mp_const_none ? 0 : mp_obj_get_float(args[ARG_delta].u_obj);
    int borderType = args[ARG_borderType].u_int;

    try {
        Laplacian(src, dst, ddepth, ksize, scale, delta, borderType);
    } catch(Exception &e) {
        mp_raise_msg(&mp_type_Exception, MP_ERROR_TEXT(e.what()));
    }
    return mat_to_mp_obj(dst);
}
MP_DEFINE_CONST_FUN_OBJ_KW(cv2_imgproc_Laplacian_obj, 1, cv2_imgproc_Laplacian_fun);

STATIC mp_obj_t cv2_imgproc_erode_fun(size_t n_args, const mp_obj_t *pos_args, mp_map_t *kw_args)
{
    enum { ARG_src, ARG_kernel, ARG_dst, ARG_anchor, ARG_iterations, ARG_borderType };
    static const mp_arg_t allowed_args[] = {
        { MP_QSTR_src,        MP_ARG_REQUIRED | MP_ARG_OBJ, {.u_obj = MP_OBJ_NULL} },
        { MP_QSTR_kernel,     MP_ARG_REQUIRED | MP_ARG_OBJ, {.u_obj = MP_OBJ_NULL} },
        { MP_QSTR_dst,        MP_ARG_OBJ, {.u_obj = mp_const_none} },
        { MP_QSTR_anchor,     MP_ARG_OBJ, {.u_obj = mp_const_none} },
        { MP_QSTR_iterations, MP_ARG_INT, {.u_int = 1} },
        { MP_QSTR_borderType, MP_ARG_INT, {.u_int = BORDER_CONSTANT} },
    };
    mp_arg_val_t args[MP_ARRAY_SIZE(allowed_args)];
    mp_arg_parse_all(n_args, pos_args, kw_args, MP_ARRAY_SIZE(allowed_args), allowed_args, args);

    Mat src = mp_obj_to_mat(args[ARG_src].u_obj);
    Mat kernel = mp_obj_to_mat(args[ARG_kernel].u_obj);
    Mat dst = mp_obj_to_mat(args[ARG_dst].u_obj);
    Point anchor = args[ARG_anchor].u_obj == mp_const_none ? Point(-1,-1) : mp_obj_to_point(args[ARG_anchor].u_obj);
    int iterations = args[ARG_iterations].u_int;
    int borderType = args[ARG_borderType].u_int;

    try {
        erode(src, dst, kernel, anchor, iterations, borderType);
    } catch(Exception &e) {
        mp_raise_msg(&mp_type_Exception, MP_ERROR_TEXT(e.what()));
    }
    return mat_to_mp_obj(dst);
}
MP_DEFINE_CONST_FUN_OBJ_KW(cv2_imgproc_erode_obj, 1, cv2_imgproc_erode_fun);

STATIC mp_obj_t cv2_imgproc_dilate_fun(size_t n_args, const mp_obj_t *pos_args, mp_map_t *kw_args)
{
    enum { ARG_src, ARG_kernel, ARG_dst, ARG_anchor, ARG_iterations, ARG_borderType };
    static const mp_arg_t allowed_args[] = {
        { MP_QSTR_src,        MP_ARG_REQUIRED | MP_ARG_OBJ, {.u_obj = MP_OBJ_NULL} },
        { MP_QSTR_kernel,     MP_ARG_REQUIRED | MP_ARG_OBJ, {.u_obj = MP_OBJ_NULL} },
        { MP_QSTR_dst,        MP_ARG_OBJ, {.u_obj = mp_const_none} },
        { MP_QSTR_anchor,     MP_ARG_OBJ, {.u_obj = mp_const_none} },
        { MP_QSTR_iterations, MP_ARG_INT, {.u_int = 1} },
        { MP_QSTR_borderType, MP_ARG_INT, {.u_int = BORDER_CONSTANT} },
    };
    mp_arg_val_t args[MP_ARRAY_SIZE(allowed_args)];
    mp_arg_parse_all(n_args, pos_args, kw_args, MP_ARRAY_SIZE(allowed_args), allowed_args, args);

    Mat src = mp_obj_to_mat(args[ARG_src].u_obj);
    Mat kernel = mp_obj_to_mat(args[ARG_kernel].u_obj);
    Mat dst = mp_obj_to_mat(args[ARG_dst].u_obj);
    Point anchor = args[ARG_anchor].u_obj == mp_const_none ? Point(-1,-1) : mp_obj_to_point(args[ARG_anchor].u_obj);
    int iterations = args[ARG_iterations].u_int;
    int borderType = args[ARG_borderType].u_int;

    try {
        dilate(src, dst, kernel, anchor, iterations, borderType);
    } catch(Exception &e) {
        mp_raise_msg(&mp_type_Exception, MP_ERROR_TEXT(e.what()));
    }
    return mat_to_mp_obj(dst);
}
MP_DEFINE_CONST_FUN_OBJ_KW(cv2_imgproc_dilate_obj, 1, cv2_imgproc_dilate_fun);

STATIC mp_obj_t cv2_imgproc_morphologyEx_fun(size_t n_args, const mp_obj_t *pos_args, mp_map_t *kw_args)
{
    enum { ARG_src, ARG_op, ARG_kernel, ARG_dst, ARG_anchor, ARG_iterations, ARG_borderType };
    static const mp_arg_t allowed_args[] = {
        { MP_QSTR_src,        MP_ARG_REQUIRED | MP_ARG_OBJ, {.u_obj = MP_OBJ_NULL} },
        { MP_QSTR_op,         MP_ARG_REQUIRED | MP_ARG_INT, {.u_int = 0} },
        { MP_QSTR_kernel,     MP_ARG_REQUIRED | MP_ARG_OBJ, {.u_obj = MP_OBJ_NULL} },
        { MP_QSTR_dst,        MP_ARG_OBJ, {.u_obj = mp_const_none} },
        { MP_QSTR_anchor,     MP_ARG_OBJ, {.u_obj = mp_const_none} },
        { MP_QSTR_iterations, MP_ARG_INT, {.u_int = 1} },
        { MP_QSTR_borderType, MP_ARG_INT, {.u_int = BORDER_CONSTANT} },
    };
    mp_arg_val_t args[MP_ARRAY_SIZE(allowed_args)];
    mp_arg_parse_all(n_args, pos_args, kw_args, MP_ARRAY_SIZE(allowed_args), allowed_args, args);

    Mat src = mp_obj_to_mat(args[ARG_src].u_obj);
    int op = args[ARG_op].u_int;
    Mat kernel = mp_obj_to_mat(args[ARG_kernel].u_obj);
    Mat dst = mp_obj_to_mat(args[ARG_dst].u_obj);
    Point anchor = args[ARG_anchor].u_obj == mp_const_none ? Point(-1,-1) : mp_obj_to_point(args[ARG_anchor].u_obj);
    int iterations = args[ARG_iterations].u_int;
    int borderType = args[ARG_borderType].u_int;

    try {
        morphologyEx(src, dst, op, kernel, anchor, iterations, borderType);
    } catch(Exception &e) {
        mp_raise_msg(&mp_type_Exception, MP_ERROR_TEXT(e.what()));
    }
    return mat_to_mp_obj(dst);
}
MP_DEFINE_CONST_FUN_OBJ_KW(cv2_imgproc_morphologyEx_obj, 1, cv2_imgproc_morphologyEx_fun);

STATIC mp_obj_t cv2_imgproc_getStructuringElement_fun(size_t n_args, const mp_obj_t *pos_args, mp_map_t *kw_args)
{
    enum { ARG_shape, ARG_ksize, ARG_anchor };
    static const mp_arg_t allowed_args[] = {
        { MP_QSTR_shape,  MP_ARG_REQUIRED | MP_ARG_INT, {.u_int = 0} },
        { MP_QSTR_ksize,  MP_ARG_REQUIRED | MP_ARG_OBJ, {.u_obj = MP_OBJ_NULL} },
        { MP_QSTR_anchor, MP_ARG_OBJ, {.u_obj = mp_const_none} },
    };
    mp_arg_val_t args[MP_ARRAY_SIZE(allowed_args)];
    mp_arg_parse_all(n_args, pos_args, kw_args, MP_ARRAY_SIZE(allowed_args), allowed_args, args);

    int shape = args[ARG_shape].u_int;
    Size ksize = mp_obj_to_size(args[ARG_ksize].u_obj);
    Point anchor = args[ARG_anchor].u_obj == mp_const_none ? Point(-1,-1) : mp_obj_to_point(args[ARG_anchor].u_obj);

    Mat retval;
    try {
        retval = getStructuringElement(shape, ksize, anchor);
    } catch(Exception &e) {
        mp_raise_msg(&mp_type_Exception, MP_ERROR_TEXT(e.what()));
    }
    return mat_to_mp_obj(retval);
}
MP_DEFINE_CONST_FUN_OBJ_KW(cv2_imgproc_getStructuringElement_obj, 1, cv2_imgproc_getStructuringElement_fun);

STATIC mp_obj_t cv2_imgproc_threshold_fun(size_t n_args, const mp_obj_t *pos_args, mp_map_t *kw_args)
{
    enum { ARG_src, ARG_thresh, ARG_maxval, ARG_type, ARG_dst };
    static const mp_arg_t allowed_args[] = {
        { MP_QSTR_src,    MP_ARG_REQUIRED | MP_ARG_OBJ, {.u_obj = MP_OBJ_NULL} },
        { MP_QSTR_thresh, MP_ARG_REQUIRED | MP_ARG_OBJ, {.u_obj = MP_OBJ_NULL} },
        { MP_QSTR_maxval, MP_ARG_REQUIRED | MP_ARG_OBJ, {.u_obj = MP_OBJ_NULL} },
        { MP_QSTR_type,   MP_ARG_REQUIRED | MP_ARG_INT, {.u_int = 0} },
        { MP_QSTR_dst,    MP_ARG_OBJ, {.u_obj = mp_const_none} },
    };
    mp_arg_val_t args[MP_ARRAY_SIZE(allowed_args)];
    mp_arg_parse_all(n_args, pos_args, kw_args, MP_ARRAY_SIZE(allowed_args), allowed_args, args);

    Mat src = mp_obj_to_mat(args[ARG_src].u_obj);
    double thresh = mp_obj_get_float(args[ARG_thresh].u_obj);
    double maxval = mp_obj_get_float(args[ARG_maxval].u_obj);
    int type = args[ARG_type].u_int;
    Mat dst = mp_obj_to_mat(args[ARG_dst].u_obj);

    double retval;
    try {
        retval = threshold(src, dst, thresh, maxval, type);
    } catch(Exception &e) {
        mp_raise_msg(&mp_type_Exception, MP_ERROR_TEXT(e.what()));
    }

    mp_obj_t result[2];
    result[0] = mp_obj_new_float(retval);
    result[1] = mat_to_mp_obj(dst);
    return mp_obj_new_tuple(2, result);
}
MP_DEFINE_CONST_FUN_OBJ_KW(cv2_imgproc_threshold_obj, 1, cv2_imgproc_threshold_fun);

STATIC mp_obj_t cv2_imgproc_adaptiveThreshold_fun(size_t n_args, const mp_obj_t *pos_args, mp_map_t *kw_args)
{
    enum { ARG_src, ARG_maxValue, ARG_adaptiveMethod, ARG_thresholdType, ARG_blockSize, ARG_C, ARG_dst };
    static const mp_arg_t allowed_args[] = {
        { MP_QSTR_src,             MP_ARG_REQUIRED | MP_ARG_OBJ, {.u_obj = MP_OBJ_NULL} },
        { MP_QSTR_maxValue,        MP_ARG_REQUIRED | MP_ARG_OBJ, {.u_obj = MP_OBJ_NULL} },
        { MP_QSTR_adaptiveMethod,  MP_ARG_REQUIRED | MP_ARG_INT, {.u_int = 0} },
        { MP_QSTR_thresholdType,   MP_ARG_REQUIRED | MP_ARG_INT, {.u_int = 0} },
        { MP_QSTR_blockSize,       MP_ARG_REQUIRED | MP_ARG_INT, {.u_int = 3} },
        { MP_QSTR_C,               MP_ARG_REQUIRED | MP_ARG_OBJ, {.u_obj = MP_OBJ_NULL} },
        { MP_QSTR_dst,             MP_ARG_OBJ, {.u_obj = mp_const_none} },
    };
    mp_arg_val_t args[MP_ARRAY_SIZE(allowed_args)];
    mp_arg_parse_all(n_args, pos_args, kw_args, MP_ARRAY_SIZE(allowed_args), allowed_args, args);

    Mat src = mp_obj_to_mat(args[ARG_src].u_obj);
    double maxValue = mp_obj_get_float(args[ARG_maxValue].u_obj);
    int adaptiveMethod = args[ARG_adaptiveMethod].u_int;
    int thresholdType = args[ARG_thresholdType].u_int;
    int blockSize = args[ARG_blockSize].u_int;
    double C = mp_obj_get_float(args[ARG_C].u_obj);
    Mat dst = mp_obj_to_mat(args[ARG_dst].u_obj);

    try {
        adaptiveThreshold(src, dst, maxValue, adaptiveMethod, thresholdType, blockSize, C);
    } catch(Exception &e) {
        mp_raise_msg(&mp_type_Exception, MP_ERROR_TEXT(e.what()));
    }
    return mat_to_mp_obj(dst);
}
MP_DEFINE_CONST_FUN_OBJ_KW(cv2_imgproc_adaptiveThreshold_obj, 1, cv2_imgproc_adaptiveThreshold_fun);

STATIC mp_obj_t cv2_imgproc_cvtColor_fun(size_t n_args, const mp_obj_t *pos_args, mp_map_t *kw_args)
{
    enum { ARG_src, ARG_code, ARG_dst, ARG_dstCn };
    static const mp_arg_t allowed_args[] = {
        { MP_QSTR_src,   MP_ARG_REQUIRED | MP_ARG_OBJ, {.u_obj = MP_OBJ_NULL} },
        { MP_QSTR_code,  MP_ARG_REQUIRED | MP_ARG_INT, {.u_int = 0} },
        { MP_QSTR_dst,   MP_ARG_OBJ, {.u_obj = mp_const_none} },
        { MP_QSTR_dstCn, MP_ARG_INT, {.u_int = 0} },
    };
    mp_arg_val_t args[MP_ARRAY_SIZE(allowed_args)];
    mp_arg_parse_all(n_args, pos_args, kw_args, MP_ARRAY_SIZE(allowed_args), allowed_args, args);

    Mat src = mp_obj_to_mat(args[ARG_src].u_obj);
    int code = args[ARG_code].u_int;
    Mat dst = mp_obj_to_mat(args[ARG_dst].u_obj);
    int dstCn = args[ARG_dstCn].u_int;

    try {
        cvtColor(src, dst, code, dstCn);
    } catch(Exception &e) {
        mp_raise_msg(&mp_type_Exception, MP_ERROR_TEXT(e.what()));
    }
    return mat_to_mp_obj(dst);
}
MP_DEFINE_CONST_FUN_OBJ_KW(cv2_imgproc_cvtColor_obj, 1, cv2_imgproc_cvtColor_fun);

STATIC mp_obj_t cv2_imgproc_resize_fun(size_t n_args, const mp_obj_t *pos_args, mp_map_t *kw_args)
{
    enum { ARG_src, ARG_dsize, ARG_dst, ARG_fx, ARG_fy, ARG_interpolation };
    static const mp_arg_t allowed_args[] = {
        { MP_QSTR_src,           MP_ARG_REQUIRED | MP_ARG_OBJ, {.u_obj = MP_OBJ_NULL} },
        { MP_QSTR_dsize,         MP_ARG_REQUIRED | MP_ARG_OBJ, {.u_obj = MP_OBJ_NULL} },
        { MP_QSTR_dst,           MP_ARG_OBJ, {.u_obj = mp_const_none} },
        { MP_QSTR_fx,            MP_ARG_OBJ, {.u_obj = mp_const_none} },
        { MP_QSTR_fy,            MP_ARG_OBJ, {.u_obj = mp_const_none} },
        { MP_QSTR_interpolation, MP_ARG_INT, {.u_int = INTER_LINEAR} },
    };
    mp_arg_val_t args[MP_ARRAY_SIZE(allowed_args)];
    mp_arg_parse_all(n_args, pos_args, kw_args, MP_ARRAY_SIZE(allowed_args), allowed_args, args);

    Mat src = mp_obj_to_mat(args[ARG_src].u_obj);
    Size dsize = mp_obj_to_size(args[ARG_dsize].u_obj);
    Mat dst = mp_obj_to_mat(args[ARG_dst].u_obj);
    double fx = args[ARG_fx].u_obj == mp_const_none ? 0 : mp_obj_get_float(args[ARG_fx].u_obj);
    double fy = args[ARG_fy].u_obj == mp_const_none ? 0 : mp_obj_get_float(args[ARG_fy].u_obj);
    int interpolation = args[ARG_interpolation].u_int;

    try {
        resize(src, dst, dsize, fx, fy, interpolation);
    } catch(Exception &e) {
        mp_raise_msg(&mp_type_Exception, MP_ERROR_TEXT(e.what()));
    }
    return mat_to_mp_obj(dst);
}
MP_DEFINE_CONST_FUN_OBJ_KW(cv2_imgproc_resize_obj, 1, cv2_imgproc_resize_fun);

STATIC mp_obj_t cv2_imgproc_warpAffine_fun(size_t n_args, const mp_obj_t *pos_args, mp_map_t *kw_args)
{
    enum { ARG_src, ARG_M, ARG_dsize, ARG_dst, ARG_flags, ARG_borderMode, ARG_borderValue };
    static const mp_arg_t allowed_args[] = {
        { MP_QSTR_src,         MP_ARG_REQUIRED | MP_ARG_OBJ, {.u_obj = MP_OBJ_NULL} },
        { MP_QSTR_M,           MP_ARG_REQUIRED | MP_ARG_OBJ, {.u_obj = MP_OBJ_NULL} },
        { MP_QSTR_dsize,       MP_ARG_REQUIRED | MP_ARG_OBJ, {.u_obj = MP_OBJ_NULL} },
        { MP_QSTR_dst,         MP_ARG_OBJ, {.u_obj = mp_const_none} },
        { MP_QSTR_flags,       MP_ARG_INT, {.u_int = INTER_LINEAR} },
        { MP_QSTR_borderMode,  MP_ARG_INT, {.u_int = BORDER_CONSTANT} },
        { MP_QSTR_borderValue, MP_ARG_OBJ, {.u_obj = mp_const_none} },
    };
    mp_arg_val_t args[MP_ARRAY_SIZE(allowed_args)];
    mp_arg_parse_all(n_args, pos_args, kw_args, MP_ARRAY_SIZE(allowed_args), allowed_args, args);

    Mat src = mp_obj_to_mat(args[ARG_src].u_obj);
    Mat M = mp_obj_to_mat(args[ARG_M].u_obj);
    Size dsize = mp_obj_to_size(args[ARG_dsize].u_obj);
    Mat dst = mp_obj_to_mat(args[ARG_dst].u_obj);
    int flags = args[ARG_flags].u_int;
    int borderMode = args[ARG_borderMode].u_int;
    Scalar borderValue = args[ARG_borderValue].u_obj == mp_const_none ? Scalar() : mp_obj_to_scalar(args[ARG_borderValue].u_obj);

    try {
        warpAffine(src, dst, M, dsize, flags, borderMode, borderValue);
    } catch(Exception &e) {
        mp_raise_msg(&mp_type_Exception, MP_ERROR_TEXT(e.what()));
    }
    return mat_to_mp_obj(dst);
}
MP_DEFINE_CONST_FUN_OBJ_KW(cv2_imgproc_warpAffine_obj, 1, cv2_imgproc_warpAffine_fun);

STATIC mp_obj_t cv2_imgproc_warpPerspective_fun(size_t n_args, const mp_obj_t *pos_args, mp_map_t *kw_args)
{
    enum { ARG_src, ARG_M, ARG_dsize, ARG_dst, ARG_flags, ARG_borderMode, ARG_borderValue };
    static const mp_arg_t allowed_args[] = {
        { MP_QSTR_src,         MP_ARG_REQUIRED | MP_ARG_OBJ, {.u_obj = MP_OBJ_NULL} },
        { MP_QSTR_M,           MP_ARG_REQUIRED | MP_ARG_OBJ, {.u_obj = MP_OBJ_NULL} },
        { MP_QSTR_dsize,       MP_ARG_REQUIRED | MP_ARG_OBJ, {.u_obj = MP_OBJ_NULL} },
        { MP_QSTR_dst,         MP_ARG_OBJ, {.u_obj = mp_const_none} },
        { MP_QSTR_flags,       MP_ARG_INT, {.u_int = INTER_LINEAR} },
        { MP_QSTR_borderMode,  MP_ARG_INT, {.u_int = BORDER_CONSTANT} },
        { MP_QSTR_borderValue, MP_ARG_OBJ, {.u_obj = mp_const_none} },
    };
    mp_arg_val_t args[MP_ARRAY_SIZE(allowed_args)];
    mp_arg_parse_all(n_args, pos_args, kw_args, MP_ARRAY_SIZE(allowed_args), allowed_args, args);

    Mat src = mp_obj_to_mat(args[ARG_src].u_obj);
    Mat M = mp_obj_to_mat(args[ARG_M].u_obj);
    Size dsize = mp_obj_to_size(args[ARG_dsize].u_obj);
    Mat dst = mp_obj_to_mat(args[ARG_dst].u_obj);
    int flags = args[ARG_flags].u_int;
    int borderMode = args[ARG_borderMode].u_int;
    Scalar borderValue = args[ARG_borderValue].u_obj == mp_const_none ? Scalar() : mp_obj_to_scalar(args[ARG_borderValue].u_obj);

    try {
        warpPerspective(src, dst, M, dsize, flags, borderMode, borderValue);
    } catch(Exception &e) {
        mp_raise_msg(&mp_type_Exception, MP_ERROR_TEXT(e.what()));
    }
    return mat_to_mp_obj(dst);
}
MP_DEFINE_CONST_FUN_OBJ_KW(cv2_imgproc_warpPerspective_obj, 1, cv2_imgproc_warpPerspective_fun);

STATIC mp_obj_t cv2_imgproc_getRotationMatrix2D_fun(size_t n_args, const mp_obj_t *pos_args, mp_map_t *kw_args)
{
    enum { ARG_center, ARG_angle, ARG_scale };
    static const mp_arg_t allowed_args[] = {
        { MP_QSTR_center, MP_ARG_REQUIRED | MP_ARG_OBJ, {.u_obj = MP_OBJ_NULL} },
        { MP_QSTR_angle,  MP_ARG_REQUIRED | MP_ARG_OBJ, {.u_obj = MP_OBJ_NULL} },
        { MP_QSTR_scale,  MP_ARG_REQUIRED | MP_ARG_OBJ, {.u_obj = MP_OBJ_NULL} },
    };
    mp_arg_val_t args[MP_ARRAY_SIZE(allowed_args)];
    mp_arg_parse_all(n_args, pos_args, kw_args, MP_ARRAY_SIZE(allowed_args), allowed_args, args);

    Point2f center = (Point2f)mp_obj_to_point(args[ARG_center].u_obj);
    double angle = mp_obj_get_float(args[ARG_angle].u_obj);
    double scale = mp_obj_get_float(args[ARG_scale].u_obj);

    Mat retval;
    try {
        retval = getRotationMatrix2D(center, angle, scale);
    } catch(Exception &e) {
        mp_raise_msg(&mp_type_Exception, MP_ERROR_TEXT(e.what()));
    }
    return mat_to_mp_obj(retval);
}
MP_DEFINE_CONST_FUN_OBJ_KW(cv2_imgproc_getRotationMatrix2D_obj, 1, cv2_imgproc_getRotationMatrix2D_fun);

// ---- Drawing Functions ----

STATIC mp_obj_t cv2_imgproc_line_fun(size_t n_args, const mp_obj_t *pos_args, mp_map_t *kw_args)
{
    enum { ARG_img, ARG_pt1, ARG_pt2, ARG_color, ARG_thickness, ARG_lineType, ARG_shift };
    static const mp_arg_t allowed_args[] = {
        { MP_QSTR_img,       MP_ARG_REQUIRED | MP_ARG_OBJ, {.u_obj = MP_OBJ_NULL} },
        { MP_QSTR_pt1,       MP_ARG_REQUIRED | MP_ARG_OBJ, {.u_obj = MP_OBJ_NULL} },
        { MP_QSTR_pt2,       MP_ARG_REQUIRED | MP_ARG_OBJ, {.u_obj = MP_OBJ_NULL} },
        { MP_QSTR_color,     MP_ARG_REQUIRED | MP_ARG_OBJ, {.u_obj = MP_OBJ_NULL} },
        { MP_QSTR_thickness, MP_ARG_INT, {.u_int = 1} },
        { MP_QSTR_lineType,  MP_ARG_INT, {.u_int = LINE_8} },
        { MP_QSTR_shift,     MP_ARG_INT, {.u_int = 0} },
    };
    mp_arg_val_t args[MP_ARRAY_SIZE(allowed_args)];
    mp_arg_parse_all(n_args, pos_args, kw_args, MP_ARRAY_SIZE(allowed_args), allowed_args, args);

    Mat img = mp_obj_to_mat(args[ARG_img].u_obj);
    Point pt1 = mp_obj_to_point(args[ARG_pt1].u_obj);
    Point pt2 = mp_obj_to_point(args[ARG_pt2].u_obj);
    Scalar color = mp_obj_to_scalar(args[ARG_color].u_obj);
    int thickness = args[ARG_thickness].u_int;
    int lineType = args[ARG_lineType].u_int;
    int shift = args[ARG_shift].u_int;

    try {
        line(img, pt1, pt2, color, thickness, lineType, shift);
    } catch(Exception &e) {
        mp_raise_msg(&mp_type_Exception, MP_ERROR_TEXT(e.what()));
    }
    return mat_to_mp_obj(img);
}
MP_DEFINE_CONST_FUN_OBJ_KW(cv2_imgproc_line_obj, 1, cv2_imgproc_line_fun);

STATIC mp_obj_t cv2_imgproc_rectangle_fun(size_t n_args, const mp_obj_t *pos_args, mp_map_t *kw_args)
{
    enum { ARG_img, ARG_pt1, ARG_pt2, ARG_color, ARG_thickness, ARG_lineType, ARG_shift };
    static const mp_arg_t allowed_args[] = {
        { MP_QSTR_img,       MP_ARG_REQUIRED | MP_ARG_OBJ, {.u_obj = MP_OBJ_NULL} },
        { MP_QSTR_pt1,       MP_ARG_REQUIRED | MP_ARG_OBJ, {.u_obj = MP_OBJ_NULL} },
        { MP_QSTR_pt2,       MP_ARG_REQUIRED | MP_ARG_OBJ, {.u_obj = MP_OBJ_NULL} },
        { MP_QSTR_color,     MP_ARG_REQUIRED | MP_ARG_OBJ, {.u_obj = MP_OBJ_NULL} },
        { MP_QSTR_thickness, MP_ARG_INT, {.u_int = 1} },
        { MP_QSTR_lineType,  MP_ARG_INT, {.u_int = LINE_8} },
        { MP_QSTR_shift,     MP_ARG_INT, {.u_int = 0} },
    };
    mp_arg_val_t args[MP_ARRAY_SIZE(allowed_args)];
    mp_arg_parse_all(n_args, pos_args, kw_args, MP_ARRAY_SIZE(allowed_args), allowed_args, args);

    Mat img = mp_obj_to_mat(args[ARG_img].u_obj);
    Point pt1 = mp_obj_to_point(args[ARG_pt1].u_obj);
    Point pt2 = mp_obj_to_point(args[ARG_pt2].u_obj);
    Scalar color = mp_obj_to_scalar(args[ARG_color].u_obj);
    int thickness = args[ARG_thickness].u_int;
    int lineType = args[ARG_lineType].u_int;
    int shift = args[ARG_shift].u_int;

    try {
        rectangle(img, pt1, pt2, color, thickness, lineType, shift);
    } catch(Exception &e) {
        mp_raise_msg(&mp_type_Exception, MP_ERROR_TEXT(e.what()));
    }
    return mat_to_mp_obj(img);
}
MP_DEFINE_CONST_FUN_OBJ_KW(cv2_imgproc_rectangle_obj, 1, cv2_imgproc_rectangle_fun);

STATIC mp_obj_t cv2_imgproc_circle_fun(size_t n_args, const mp_obj_t *pos_args, mp_map_t *kw_args)
{
    enum { ARG_img, ARG_center, ARG_radius, ARG_color, ARG_thickness, ARG_lineType, ARG_shift };
    static const mp_arg_t allowed_args[] = {
        { MP_QSTR_img,       MP_ARG_REQUIRED | MP_ARG_OBJ, {.u_obj = MP_OBJ_NULL} },
        { MP_QSTR_center,    MP_ARG_REQUIRED | MP_ARG_OBJ, {.u_obj = MP_OBJ_NULL} },
        { MP_QSTR_radius,    MP_ARG_REQUIRED | MP_ARG_INT, {.u_int = 0} },
        { MP_QSTR_color,     MP_ARG_REQUIRED | MP_ARG_OBJ, {.u_obj = MP_OBJ_NULL} },
        { MP_QSTR_thickness, MP_ARG_INT, {.u_int = 1} },
        { MP_QSTR_lineType,  MP_ARG_INT, {.u_int = LINE_8} },
        { MP_QSTR_shift,     MP_ARG_INT, {.u_int = 0} },
    };
    mp_arg_val_t args[MP_ARRAY_SIZE(allowed_args)];
    mp_arg_parse_all(n_args, pos_args, kw_args, MP_ARRAY_SIZE(allowed_args), allowed_args, args);

    Mat img = mp_obj_to_mat(args[ARG_img].u_obj);
    Point center = mp_obj_to_point(args[ARG_center].u_obj);
    int radius = args[ARG_radius].u_int;
    Scalar color = mp_obj_to_scalar(args[ARG_color].u_obj);
    int thickness = args[ARG_thickness].u_int;
    int lineType = args[ARG_lineType].u_int;
    int shift = args[ARG_shift].u_int;

    try {
        circle(img, center, radius, color, thickness, lineType, shift);
    } catch(Exception &e) {
        mp_raise_msg(&mp_type_Exception, MP_ERROR_TEXT(e.what()));
    }
    return mat_to_mp_obj(img);
}
MP_DEFINE_CONST_FUN_OBJ_KW(cv2_imgproc_circle_obj, 1, cv2_imgproc_circle_fun);

STATIC mp_obj_t cv2_imgproc_ellipse_fun(size_t n_args, const mp_obj_t *pos_args, mp_map_t *kw_args)
{
    enum { ARG_img, ARG_center, ARG_axes, ARG_angle, ARG_startAngle, ARG_endAngle, ARG_color, ARG_thickness, ARG_lineType, ARG_shift };
    static const mp_arg_t allowed_args[] = {
        { MP_QSTR_img,        MP_ARG_REQUIRED | MP_ARG_OBJ, {.u_obj = MP_OBJ_NULL} },
        { MP_QSTR_center,     MP_ARG_REQUIRED | MP_ARG_OBJ, {.u_obj = MP_OBJ_NULL} },
        { MP_QSTR_axes,       MP_ARG_REQUIRED | MP_ARG_OBJ, {.u_obj = MP_OBJ_NULL} },
        { MP_QSTR_angle,      MP_ARG_REQUIRED | MP_ARG_OBJ, {.u_obj = MP_OBJ_NULL} },
        { MP_QSTR_startAngle, MP_ARG_REQUIRED | MP_ARG_OBJ, {.u_obj = MP_OBJ_NULL} },
        { MP_QSTR_endAngle,   MP_ARG_REQUIRED | MP_ARG_OBJ, {.u_obj = MP_OBJ_NULL} },
        { MP_QSTR_color,      MP_ARG_REQUIRED | MP_ARG_OBJ, {.u_obj = MP_OBJ_NULL} },
        { MP_QSTR_thickness,  MP_ARG_INT, {.u_int = 1} },
        { MP_QSTR_lineType,   MP_ARG_INT, {.u_int = LINE_8} },
        { MP_QSTR_shift,      MP_ARG_INT, {.u_int = 0} },
    };
    mp_arg_val_t args[MP_ARRAY_SIZE(allowed_args)];
    mp_arg_parse_all(n_args, pos_args, kw_args, MP_ARRAY_SIZE(allowed_args), allowed_args, args);

    Mat img = mp_obj_to_mat(args[ARG_img].u_obj);
    Point center = mp_obj_to_point(args[ARG_center].u_obj);
    Size axes = mp_obj_to_size(args[ARG_axes].u_obj);
    double angle = mp_obj_get_float(args[ARG_angle].u_obj);
    double startAngle = mp_obj_get_float(args[ARG_startAngle].u_obj);
    double endAngle = mp_obj_get_float(args[ARG_endAngle].u_obj);
    Scalar color = mp_obj_to_scalar(args[ARG_color].u_obj);
    int thickness = args[ARG_thickness].u_int;
    int lineType = args[ARG_lineType].u_int;
    int shift = args[ARG_shift].u_int;

    try {
        ellipse(img, center, axes, angle, startAngle, endAngle, color, thickness, lineType, shift);
    } catch(Exception &e) {
        mp_raise_msg(&mp_type_Exception, MP_ERROR_TEXT(e.what()));
    }
    return mat_to_mp_obj(img);
}
MP_DEFINE_CONST_FUN_OBJ_KW(cv2_imgproc_ellipse_obj, 1, cv2_imgproc_ellipse_fun);

STATIC mp_obj_t cv2_imgproc_putText_fun(size_t n_args, const mp_obj_t *pos_args, mp_map_t *kw_args)
{
    enum { ARG_img, ARG_text, ARG_org, ARG_fontFace, ARG_fontScale, ARG_color, ARG_thickness, ARG_lineType, ARG_bottomLeftOrigin };
    static const mp_arg_t allowed_args[] = {
        { MP_QSTR_img,              MP_ARG_REQUIRED | MP_ARG_OBJ, {.u_obj = MP_OBJ_NULL} },
        { MP_QSTR_text,             MP_ARG_REQUIRED | MP_ARG_OBJ, {.u_obj = MP_OBJ_NULL} },
        { MP_QSTR_org,              MP_ARG_REQUIRED | MP_ARG_OBJ, {.u_obj = MP_OBJ_NULL} },
        { MP_QSTR_fontFace,         MP_ARG_REQUIRED | MP_ARG_INT, {.u_int = 0} },
        { MP_QSTR_fontScale,        MP_ARG_REQUIRED | MP_ARG_OBJ, {.u_obj = MP_OBJ_NULL} },
        { MP_QSTR_color,            MP_ARG_REQUIRED | MP_ARG_OBJ, {.u_obj = MP_OBJ_NULL} },
        { MP_QSTR_thickness,        MP_ARG_INT, {.u_int = 1} },
        { MP_QSTR_lineType,         MP_ARG_INT, {.u_int = LINE_8} },
        { MP_QSTR_bottomLeftOrigin, MP_ARG_BOOL, {.u_bool = false} },
    };
    mp_arg_val_t args[MP_ARRAY_SIZE(allowed_args)];
    mp_arg_parse_all(n_args, pos_args, kw_args, MP_ARRAY_SIZE(allowed_args), allowed_args, args);

    Mat img = mp_obj_to_mat(args[ARG_img].u_obj);
    const char *text = mp_obj_str_get_str(args[ARG_text].u_obj);
    Point org = mp_obj_to_point(args[ARG_org].u_obj);
    int fontFace = args[ARG_fontFace].u_int;
    double fontScale = mp_obj_get_float(args[ARG_fontScale].u_obj);
    Scalar color = mp_obj_to_scalar(args[ARG_color].u_obj);
    int thickness = args[ARG_thickness].u_int;
    int lineType = args[ARG_lineType].u_int;
    bool bottomLeftOrigin = args[ARG_bottomLeftOrigin].u_bool;

    try {
        putText(img, text, org, fontFace, fontScale, color, thickness, lineType, bottomLeftOrigin);
    } catch(Exception &e) {
        mp_raise_msg(&mp_type_Exception, MP_ERROR_TEXT(e.what()));
    }
    return mat_to_mp_obj(img);
}
MP_DEFINE_CONST_FUN_OBJ_KW(cv2_imgproc_putText_obj, 1, cv2_imgproc_putText_fun);

STATIC mp_obj_t cv2_imgproc_arrowedLine_fun(size_t n_args, const mp_obj_t *pos_args, mp_map_t *kw_args)
{
    enum { ARG_img, ARG_pt1, ARG_pt2, ARG_color, ARG_thickness, ARG_line_type, ARG_shift, ARG_tipLength };
    static const mp_arg_t allowed_args[] = {
        { MP_QSTR_img,       MP_ARG_REQUIRED | MP_ARG_OBJ, {.u_obj = MP_OBJ_NULL} },
        { MP_QSTR_pt1,       MP_ARG_REQUIRED | MP_ARG_OBJ, {.u_obj = MP_OBJ_NULL} },
        { MP_QSTR_pt2,       MP_ARG_REQUIRED | MP_ARG_OBJ, {.u_obj = MP_OBJ_NULL} },
        { MP_QSTR_color,     MP_ARG_REQUIRED | MP_ARG_OBJ, {.u_obj = MP_OBJ_NULL} },
        { MP_QSTR_thickness, MP_ARG_INT, {.u_int = 1} },
        { MP_QSTR_line_type, MP_ARG_INT, {.u_int = LINE_8} },
        { MP_QSTR_shift,     MP_ARG_INT, {.u_int = 0} },
        { MP_QSTR_tipLength, MP_ARG_OBJ, {.u_obj = mp_const_none} },
    };
    mp_arg_val_t args[MP_ARRAY_SIZE(allowed_args)];
    mp_arg_parse_all(n_args, pos_args, kw_args, MP_ARRAY_SIZE(allowed_args), allowed_args, args);

    Mat img = mp_obj_to_mat(args[ARG_img].u_obj);
    Point pt1 = mp_obj_to_point(args[ARG_pt1].u_obj);
    Point pt2 = mp_obj_to_point(args[ARG_pt2].u_obj);
    Scalar color = mp_obj_to_scalar(args[ARG_color].u_obj);
    int thickness = args[ARG_thickness].u_int;
    int line_type = args[ARG_line_type].u_int;
    int shift = args[ARG_shift].u_int;
    double tipLength = args[ARG_tipLength].u_obj == mp_const_none ? 0.1 : mp_obj_get_float(args[ARG_tipLength].u_obj);

    try {
        arrowedLine(img, pt1, pt2, color, thickness, line_type, shift, tipLength);
    } catch(Exception &e) {
        mp_raise_msg(&mp_type_Exception, MP_ERROR_TEXT(e.what()));
    }
    return mat_to_mp_obj(img);
}
MP_DEFINE_CONST_FUN_OBJ_KW(cv2_imgproc_arrowedLine_obj, 1, cv2_imgproc_arrowedLine_fun);

STATIC mp_obj_t cv2_imgproc_drawMarker_fun(size_t n_args, const mp_obj_t *pos_args, mp_map_t *kw_args)
{
    enum { ARG_img, ARG_position, ARG_color, ARG_markerType, ARG_markerSize, ARG_thickness, ARG_line_type };
    static const mp_arg_t allowed_args[] = {
        { MP_QSTR_img,        MP_ARG_REQUIRED | MP_ARG_OBJ, {.u_obj = MP_OBJ_NULL} },
        { MP_QSTR_position,   MP_ARG_REQUIRED | MP_ARG_OBJ, {.u_obj = MP_OBJ_NULL} },
        { MP_QSTR_color,      MP_ARG_REQUIRED | MP_ARG_OBJ, {.u_obj = MP_OBJ_NULL} },
        { MP_QSTR_markerType, MP_ARG_INT, {.u_int = MARKER_CROSS} },
        { MP_QSTR_markerSize, MP_ARG_INT, {.u_int = 20} },
        { MP_QSTR_thickness,  MP_ARG_INT, {.u_int = 1} },
        { MP_QSTR_line_type,  MP_ARG_INT, {.u_int = LINE_8} },
    };
    mp_arg_val_t args[MP_ARRAY_SIZE(allowed_args)];
    mp_arg_parse_all(n_args, pos_args, kw_args, MP_ARRAY_SIZE(allowed_args), allowed_args, args);

    Mat img = mp_obj_to_mat(args[ARG_img].u_obj);
    Point position = mp_obj_to_point(args[ARG_position].u_obj);
    Scalar color = mp_obj_to_scalar(args[ARG_color].u_obj);
    int markerType = args[ARG_markerType].u_int;
    int markerSize = args[ARG_markerSize].u_int;
    int thickness = args[ARG_thickness].u_int;
    int line_type = args[ARG_line_type].u_int;

    try {
        drawMarker(img, position, color, markerType, markerSize, thickness, line_type);
    } catch(Exception &e) {
        mp_raise_msg(&mp_type_Exception, MP_ERROR_TEXT(e.what()));
    }
    return mat_to_mp_obj(img);
}
MP_DEFINE_CONST_FUN_OBJ_KW(cv2_imgproc_drawMarker_obj, 1, cv2_imgproc_drawMarker_fun);

STATIC mp_obj_t cv2_imgproc_fillPoly_fun(size_t n_args, const mp_obj_t *pos_args, mp_map_t *kw_args)
{
    enum { ARG_img, ARG_pts, ARG_color, ARG_lineType, ARG_shift, ARG_offset };
    static const mp_arg_t allowed_args[] = {
        { MP_QSTR_img,      MP_ARG_REQUIRED | MP_ARG_OBJ, {.u_obj = MP_OBJ_NULL} },
        { MP_QSTR_pts,      MP_ARG_REQUIRED | MP_ARG_OBJ, {.u_obj = MP_OBJ_NULL} },
        { MP_QSTR_color,    MP_ARG_REQUIRED | MP_ARG_OBJ, {.u_obj = MP_OBJ_NULL} },
        { MP_QSTR_lineType, MP_ARG_INT, {.u_int = LINE_8} },
        { MP_QSTR_shift,    MP_ARG_INT, {.u_int = 0} },
        { MP_QSTR_offset,   MP_ARG_OBJ, {.u_obj = mp_const_none} },
    };
    mp_arg_val_t args[MP_ARRAY_SIZE(allowed_args)];
    mp_arg_parse_all(n_args, pos_args, kw_args, MP_ARRAY_SIZE(allowed_args), allowed_args, args);

    Mat img = mp_obj_to_mat(args[ARG_img].u_obj);
    Scalar color = mp_obj_to_scalar(args[ARG_color].u_obj);
    int lineType = args[ARG_lineType].u_int;
    int shift = args[ARG_shift].u_int;
    Point offset = args[ARG_offset].u_obj == mp_const_none ? Point() : mp_obj_to_point(args[ARG_offset].u_obj);

    mp_obj_t pts_obj = args[ARG_pts].u_obj;
    if(!mp_obj_is_type(pts_obj, &mp_type_list) && !mp_obj_is_type(pts_obj, &mp_type_tuple))
    {
        mp_raise_TypeError(MP_ERROR_TEXT("pts must be a list of contours"));
    }

    size_t ncontours;
    mp_obj_t *contours_items;
    mp_obj_get_array(pts_obj, &ncontours, &contours_items);

    std::vector<std::vector<Point>> pts;
    for(size_t c = 0; c < ncontours; c++)
    {
        ndarray_obj_t *contour = ndarray_from_mp_obj(contours_items[c], 0);
        if(!contour)
        {
            mp_raise_TypeError(MP_ERROR_TEXT("pts elements must be ndarrays"));
        }
        if(contour->ndim != 2)
        {
            mp_raise_ValueError(MP_ERROR_TEXT("pts elements must be Nx2 arrays"));
        }
        size_t rows = contour->shape[ULAB_MAX_DIMS - 2];
        size_t cols = contour->shape[ULAB_MAX_DIMS - 1];
        if(cols < 2)
        {
            mp_raise_ValueError(MP_ERROR_TEXT("pts elements must be Nx2 arrays"));
        }

        std::vector<Point> poly(rows);
        for(size_t r = 0; r < rows; r++)
        {
            mp_float_t x = ndarray_get_float_index(contour->array, contour->dtype, r * cols);
            mp_float_t y = ndarray_get_float_index(contour->array, contour->dtype, r * cols + 1);
            poly[r] = Point((int)x, (int)y);
        }
        pts.push_back(poly);
    }

    try {
        fillPoly(img, pts, color, lineType, shift, offset);
    } catch(Exception &e) {
        mp_raise_msg(&mp_type_Exception, MP_ERROR_TEXT(e.what()));
    }
    return mat_to_mp_obj(img);
}
MP_DEFINE_CONST_FUN_OBJ_KW(cv2_imgproc_fillPoly_obj, 1, cv2_imgproc_fillPoly_fun);

// ---- Structural Analysis ----

STATIC mp_obj_t cv2_imgproc_findContours_fun(size_t n_args, const mp_obj_t *pos_args, mp_map_t *kw_args)
{
    enum { ARG_image, ARG_mode, ARG_method, ARG_contours, ARG_hierarchy, ARG_offset };
    static const mp_arg_t allowed_args[] = {
        { MP_QSTR_image,     MP_ARG_REQUIRED | MP_ARG_OBJ, {.u_obj = MP_OBJ_NULL} },
        { MP_QSTR_mode,      MP_ARG_REQUIRED | MP_ARG_INT, {.u_int = 0} },
        { MP_QSTR_method,    MP_ARG_REQUIRED | MP_ARG_INT, {.u_int = 0} },
        { MP_QSTR_contours,  MP_ARG_OBJ, {.u_obj = mp_const_none} },
        { MP_QSTR_hierarchy, MP_ARG_OBJ, {.u_obj = mp_const_none} },
        { MP_QSTR_offset,    MP_ARG_OBJ, {.u_obj = mp_const_none} },
    };
    mp_arg_val_t args[MP_ARRAY_SIZE(allowed_args)];
    mp_arg_parse_all(n_args, pos_args, kw_args, MP_ARRAY_SIZE(allowed_args), allowed_args, args);

    Mat image = mp_obj_to_mat(args[ARG_image].u_obj);
    int mode = args[ARG_mode].u_int;
    int method = args[ARG_method].u_int;
    Point offset = args[ARG_offset].u_obj == mp_const_none ? Point() : mp_obj_to_point(args[ARG_offset].u_obj);

    std::vector<std::vector<Point>> contours;
    std::vector<Vec4i> hierarchy;

    try {
        findContours(image, contours, hierarchy, mode, method, offset);
    } catch(Exception &e) {
        mp_raise_msg(&mp_type_Exception, MP_ERROR_TEXT(e.what()));
    }

    // Convert contours to list of ndarrays
    mp_obj_t contours_list = mp_obj_new_list(0, NULL);
    for(auto &c : contours)
    {
        size_t len = c.size();
        if(len == 0) continue;
        size_t shape[ULAB_MAX_DIMS];
        for(int i = 0; i < ULAB_MAX_DIMS - 2; i++)
            shape[i] = 0;
        shape[ULAB_MAX_DIMS - 2] = len;
        shape[ULAB_MAX_DIMS - 1] = 2;

        ndarray_obj_t *arr = ndarray_new_dense_ndarray(2, shape, NDARRAY_FLOAT);
        if(arr)
        {
            mp_float_t *data = (mp_float_t *)arr->array;
            for(size_t i = 0; i < len; i++)
            {
                data[i * 2] = (mp_float_t)c[i].x;
                data[i * 2 + 1] = (mp_float_t)c[i].y;
            }
            mp_obj_list_append(contours_list, MP_OBJ_FROM_PTR(arr));
        }
    }

    // Convert hierarchy to ndarray
    mp_obj_t hierarchy_obj = mp_const_none;
    if(!hierarchy.empty())
    {
        size_t hlen = hierarchy.size();
        size_t hshape[ULAB_MAX_DIMS];
        for(int i = 0; i < ULAB_MAX_DIMS - 2; i++)
            hshape[i] = 0;
        hshape[ULAB_MAX_DIMS - 2] = hlen;
        hshape[ULAB_MAX_DIMS - 1] = 4;

        ndarray_obj_t *harr = ndarray_new_dense_ndarray(2, hshape, NDARRAY_INT16);
        if(harr)
        {
            int16_t *hdata = (int16_t *)harr->array;
            for(size_t i = 0; i < hlen; i++)
            {
                hdata[i * 4] = (int16_t)hierarchy[i][0];
                hdata[i * 4 + 1] = (int16_t)hierarchy[i][1];
                hdata[i * 4 + 2] = (int16_t)hierarchy[i][2];
                hdata[i * 4 + 3] = (int16_t)hierarchy[i][3];
            }
            hierarchy_obj = MP_OBJ_FROM_PTR(harr);
        }
    }

    mp_obj_t result[2];
    result[0] = contours_list;
    result[1] = hierarchy_obj;
    return mp_obj_new_tuple(2, result);
}
MP_DEFINE_CONST_FUN_OBJ_KW(cv2_imgproc_findContours_obj, 1, cv2_imgproc_findContours_fun);

STATIC mp_obj_t cv2_imgproc_drawContours_fun(size_t n_args, const mp_obj_t *pos_args, mp_map_t *kw_args)
{
    enum { ARG_image, ARG_contours, ARG_contourIdx, ARG_color, ARG_thickness, ARG_lineType, ARG_hierarchy, ARG_maxLevel, ARG_offset };
    static const mp_arg_t allowed_args[] = {
        { MP_QSTR_image,      MP_ARG_REQUIRED | MP_ARG_OBJ, {.u_obj = MP_OBJ_NULL} },
        { MP_QSTR_contours,   MP_ARG_REQUIRED | MP_ARG_OBJ, {.u_obj = MP_OBJ_NULL} },
        { MP_QSTR_contourIdx, MP_ARG_REQUIRED | MP_ARG_INT, {.u_int = 0} },
        { MP_QSTR_color,      MP_ARG_REQUIRED | MP_ARG_OBJ, {.u_obj = MP_OBJ_NULL} },
        { MP_QSTR_thickness,  MP_ARG_INT, {.u_int = 1} },
        { MP_QSTR_lineType,   MP_ARG_INT, {.u_int = LINE_8} },
        { MP_QSTR_hierarchy,  MP_ARG_OBJ, {.u_obj = mp_const_none} },
        { MP_QSTR_maxLevel,   MP_ARG_INT, {.u_int = INT_MAX} },
        { MP_QSTR_offset,     MP_ARG_OBJ, {.u_obj = mp_const_none} },
    };
    mp_arg_val_t args[MP_ARRAY_SIZE(allowed_args)];
    mp_arg_parse_all(n_args, pos_args, kw_args, MP_ARRAY_SIZE(allowed_args), allowed_args, args);

    Mat image = mp_obj_to_mat(args[ARG_image].u_obj);
    int contourIdx = args[ARG_contourIdx].u_int;
    Scalar color = mp_obj_to_scalar(args[ARG_color].u_obj);
    int thickness = args[ARG_thickness].u_int;
    int lineType = args[ARG_lineType].u_int;
    int maxLevel = args[ARG_maxLevel].u_int;
    Point offset = args[ARG_offset].u_obj == mp_const_none ? Point() : mp_obj_to_point(args[ARG_offset].u_obj);

    mp_obj_t contours_obj = args[ARG_contours].u_obj;
    mp_obj_t hierarchy_obj = args[ARG_hierarchy].u_obj;

    std::vector<std::vector<Point>> contours;
    if(mp_obj_is_type(contours_obj, &mp_type_list) || mp_obj_is_type(contours_obj, &mp_type_tuple))
    {
        size_t ncontours;
        mp_obj_t *items;
        mp_obj_get_array(contours_obj, &ncontours, &items);
        for(size_t i = 0; i < ncontours; i++)
        {
            ndarray_obj_t *contour_arr = ndarray_from_mp_obj(items[i], 0);
            if(!contour_arr)
                mp_raise_TypeError(MP_ERROR_TEXT("contours list items must be ndarrays"));
            if(contour_arr->ndim != 2)
                mp_raise_ValueError(MP_ERROR_TEXT("contours must be Nx2 arrays"));

            size_t rows = contour_arr->shape[ULAB_MAX_DIMS - 2];
            std::vector<Point> pts(rows);
            for(size_t r = 0; r < rows; r++)
            {
                mp_float_t x = ndarray_get_float_index(contour_arr->array, contour_arr->dtype, r * 2);
                mp_float_t y = ndarray_get_float_index(contour_arr->array, contour_arr->dtype, r * 2 + 1);
                pts[r] = Point((int)x, (int)y);
            }
            contours.push_back(pts);
        }
    }

    Mat hierarchy_mat;
    if(hierarchy_obj != mp_const_none)
        hierarchy_mat = mp_obj_to_mat(hierarchy_obj);

    try {
        if(hierarchy_mat.empty())
            drawContours(image, contours, contourIdx, color, thickness, lineType, noArray(), maxLevel, offset);
        else
            drawContours(image, contours, contourIdx, color, thickness, lineType, hierarchy_mat, maxLevel, offset);
    } catch(Exception &e) {
        mp_raise_msg(&mp_type_Exception, MP_ERROR_TEXT(e.what()));
    }
    return mat_to_mp_obj(image);
}
MP_DEFINE_CONST_FUN_OBJ_KW(cv2_imgproc_drawContours_obj, 1, cv2_imgproc_drawContours_fun);

STATIC mp_obj_t cv2_imgproc_contourArea_fun(size_t n_args, const mp_obj_t *pos_args, mp_map_t *kw_args)
{
    enum { ARG_contour, ARG_oriented };
    static const mp_arg_t allowed_args[] = {
        { MP_QSTR_contour,  MP_ARG_REQUIRED | MP_ARG_OBJ, {.u_obj = MP_OBJ_NULL} },
        { MP_QSTR_oriented, MP_ARG_BOOL, {.u_bool = false} },
    };
    mp_arg_val_t args[MP_ARRAY_SIZE(allowed_args)];
    mp_arg_parse_all(n_args, pos_args, kw_args, MP_ARRAY_SIZE(allowed_args), allowed_args, args);

    ndarray_obj_t *contour_arr = ndarray_from_mp_obj(args[ARG_contour].u_obj, 0);
    if(!contour_arr || contour_arr->ndim != 2)
        mp_raise_ValueError(MP_ERROR_TEXT("contour must be Nx2 ndarray"));

    size_t rows = contour_arr->shape[ULAB_MAX_DIMS - 2];
    std::vector<Point> pts(rows);
    for(size_t r = 0; r < rows; r++)
    {
        mp_float_t x = ndarray_get_float_index(contour_arr->array, contour_arr->dtype, r * 2);
        mp_float_t y = ndarray_get_float_index(contour_arr->array, contour_arr->dtype, r * 2 + 1);
        pts[r] = Point((int)x, (int)y);
    }
    bool oriented = args[ARG_oriented].u_bool;

    double area;
    try {
        area = contourArea(pts, oriented);
    } catch(Exception &e) {
        mp_raise_msg(&mp_type_Exception, MP_ERROR_TEXT(e.what()));
    }
    return mp_obj_new_float(area);
}
MP_DEFINE_CONST_FUN_OBJ_KW(cv2_imgproc_contourArea_obj, 1, cv2_imgproc_contourArea_fun);

STATIC mp_obj_t cv2_imgproc_arcLength_fun(size_t n_args, const mp_obj_t *pos_args, mp_map_t *kw_args)
{
    enum { ARG_curve, ARG_closed };
    static const mp_arg_t allowed_args[] = {
        { MP_QSTR_curve,  MP_ARG_REQUIRED | MP_ARG_OBJ, {.u_obj = MP_OBJ_NULL} },
        { MP_QSTR_closed, MP_ARG_REQUIRED | MP_ARG_BOOL, {.u_bool = false} },
    };
    mp_arg_val_t args[MP_ARRAY_SIZE(allowed_args)];
    mp_arg_parse_all(n_args, pos_args, kw_args, MP_ARRAY_SIZE(allowed_args), allowed_args, args);

    ndarray_obj_t *curve_arr = ndarray_from_mp_obj(args[ARG_curve].u_obj, 0);
    if(!curve_arr || curve_arr->ndim != 2)
        mp_raise_ValueError(MP_ERROR_TEXT("curve must be Nx2 ndarray"));

    size_t rows = curve_arr->shape[ULAB_MAX_DIMS - 2];
    std::vector<Point> pts(rows);
    for(size_t r = 0; r < rows; r++)
    {
        mp_float_t x = ndarray_get_float_index(curve_arr->array, curve_arr->dtype, r * 2);
        mp_float_t y = ndarray_get_float_index(curve_arr->array, curve_arr->dtype, r * 2 + 1);
        pts[r] = Point((int)x, (int)y);
    }
    bool closed = args[ARG_closed].u_bool;

    double length;
    try {
        length = arcLength(pts, closed);
    } catch(Exception &e) {
        mp_raise_msg(&mp_type_Exception, MP_ERROR_TEXT(e.what()));
    }
    return mp_obj_new_float(length);
}
MP_DEFINE_CONST_FUN_OBJ_KW(cv2_imgproc_arcLength_obj, 1, cv2_imgproc_arcLength_fun);

STATIC mp_obj_t cv2_imgproc_boundingRect_fun(size_t n_args, const mp_obj_t *pos_args, mp_map_t *kw_args)
{
    enum { ARG_array };
    static const mp_arg_t allowed_args[] = {
        { MP_QSTR_array, MP_ARG_REQUIRED | MP_ARG_OBJ, {.u_obj = MP_OBJ_NULL} },
    };
    mp_arg_val_t args[MP_ARRAY_SIZE(allowed_args)];
    mp_arg_parse_all(n_args, pos_args, kw_args, MP_ARRAY_SIZE(allowed_args), allowed_args, args);

    Mat src = mp_obj_to_mat(args[ARG_array].u_obj);
    Rect r;
    try {
        r = boundingRect(src);
    } catch(Exception &e) {
        mp_raise_msg(&mp_type_Exception, MP_ERROR_TEXT(e.what()));
    }

    mp_obj_t result[4];
    result[0] = mp_obj_new_int(r.x);
    result[1] = mp_obj_new_int(r.y);
    result[2] = mp_obj_new_int(r.width);
    result[3] = mp_obj_new_int(r.height);
    return mp_obj_new_tuple(4, result);
}
MP_DEFINE_CONST_FUN_OBJ_KW(cv2_imgproc_boundingRect_obj, 1, cv2_imgproc_boundingRect_fun);

STATIC mp_obj_t cv2_imgproc_approxPolyDP_fun(size_t n_args, const mp_obj_t *pos_args, mp_map_t *kw_args)
{
    enum { ARG_curve, ARG_epsilon, ARG_closed };
    static const mp_arg_t allowed_args[] = {
        { MP_QSTR_curve,   MP_ARG_REQUIRED | MP_ARG_OBJ, {.u_obj = MP_OBJ_NULL} },
        { MP_QSTR_epsilon, MP_ARG_REQUIRED | MP_ARG_OBJ, {.u_obj = MP_OBJ_NULL} },
        { MP_QSTR_closed,  MP_ARG_REQUIRED | MP_ARG_BOOL, {.u_bool = false} },
    };
    mp_arg_val_t args[MP_ARRAY_SIZE(allowed_args)];
    mp_arg_parse_all(n_args, pos_args, kw_args, MP_ARRAY_SIZE(allowed_args), allowed_args, args);

    ndarray_obj_t *curve_arr = ndarray_from_mp_obj(args[ARG_curve].u_obj, 0);
    if(!curve_arr || curve_arr->ndim != 2)
        mp_raise_ValueError(MP_ERROR_TEXT("curve must be Nx2 ndarray"));

    double epsilon = mp_obj_get_float(args[ARG_epsilon].u_obj);
    bool closed = args[ARG_closed].u_bool;

    size_t rows = curve_arr->shape[ULAB_MAX_DIMS - 2];
    std::vector<Point> pts(rows);
    for(size_t r = 0; r < rows; r++)
    {
        mp_float_t x = ndarray_get_float_index(curve_arr->array, curve_arr->dtype, r * 2);
        mp_float_t y = ndarray_get_float_index(curve_arr->array, curve_arr->dtype, r * 2 + 1);
        pts[r] = Point((int)x, (int)y);
    }

    std::vector<Point> approx;
    try {
        approxPolyDP(pts, approx, epsilon, closed);
    } catch(Exception &e) {
        mp_raise_msg(&mp_type_Exception, MP_ERROR_TEXT(e.what()));
    }

    size_t approx_shape[ULAB_MAX_DIMS];
    for(int i = 0; i < ULAB_MAX_DIMS - 2; i++)
        approx_shape[i] = 0;
    approx_shape[ULAB_MAX_DIMS - 2] = approx.size();
    approx_shape[ULAB_MAX_DIMS - 1] = 2;

    ndarray_obj_t *result_arr = ndarray_new_dense_ndarray(2, approx_shape, NDARRAY_FLOAT);
    mp_float_t *data = (mp_float_t *)result_arr->array;
    for(size_t i = 0; i < approx.size(); i++)
    {
        data[i * 2] = (float)approx[i].x;
        data[i * 2 + 1] = (float)approx[i].y;
    }
    return MP_OBJ_FROM_PTR(result_arr);
}
MP_DEFINE_CONST_FUN_OBJ_KW(cv2_imgproc_approxPolyDP_obj, 1, cv2_imgproc_approxPolyDP_fun);

STATIC mp_obj_t cv2_imgproc_convexHull_fun(size_t n_args, const mp_obj_t *pos_args, mp_map_t *kw_args)
{
    enum { ARG_points, ARG_hull, ARG_clockwise, ARG_returnPoints };
    static const mp_arg_t allowed_args[] = {
        { MP_QSTR_points,       MP_ARG_REQUIRED | MP_ARG_OBJ, {.u_obj = MP_OBJ_NULL} },
        { MP_QSTR_hull,         MP_ARG_OBJ, {.u_obj = mp_const_none} },
        { MP_QSTR_clockwise,    MP_ARG_BOOL, {.u_bool = false} },
        { MP_QSTR_returnPoints, MP_ARG_BOOL, {.u_bool = true} },
    };
    mp_arg_val_t args[MP_ARRAY_SIZE(allowed_args)];
    mp_arg_parse_all(n_args, pos_args, kw_args, MP_ARRAY_SIZE(allowed_args), allowed_args, args);

    ndarray_obj_t *pts_arr = ndarray_from_mp_obj(args[ARG_points].u_obj, 0);
    if(!pts_arr || pts_arr->ndim != 2)
        mp_raise_ValueError(MP_ERROR_TEXT("points must be Nx2 ndarray"));

    size_t rows = pts_arr->shape[ULAB_MAX_DIMS - 2];
    std::vector<Point> pts(rows);
    for(size_t r = 0; r < rows; r++)
    {
        mp_float_t x = ndarray_get_float_index(pts_arr->array, pts_arr->dtype, r * 2);
        mp_float_t y = ndarray_get_float_index(pts_arr->array, pts_arr->dtype, r * 2 + 1);
        pts[r] = Point((int)x, (int)y);
    }
    bool clockwise = args[ARG_clockwise].u_bool;
    bool returnPoints = args[ARG_returnPoints].u_bool;

    std::vector<Point> hull;
    try {
        convexHull(pts, hull, clockwise, returnPoints);
    } catch(Exception &e) {
        mp_raise_msg(&mp_type_Exception, MP_ERROR_TEXT(e.what()));
    }

    size_t hull_shape[ULAB_MAX_DIMS];
    for(int i = 0; i < ULAB_MAX_DIMS - 2; i++)
        hull_shape[i] = 0;
    hull_shape[ULAB_MAX_DIMS - 2] = hull.size();
    hull_shape[ULAB_MAX_DIMS - 1] = 2;

    ndarray_obj_t *result_arr = ndarray_new_dense_ndarray(2, hull_shape, NDARRAY_FLOAT);
    mp_float_t *data = (mp_float_t *)result_arr->array;
    for(size_t i = 0; i < hull.size(); i++)
    {
        data[i * 2] = (float)hull[i].x;
        data[i * 2 + 1] = (float)hull[i].y;
    }
    return MP_OBJ_FROM_PTR(result_arr);
}
MP_DEFINE_CONST_FUN_OBJ_KW(cv2_imgproc_convexHull_obj, 1, cv2_imgproc_convexHull_fun);

STATIC mp_obj_t cv2_imgproc_minAreaRect_fun(size_t n_args, const mp_obj_t *pos_args, mp_map_t *kw_args)
{
    enum { ARG_points };
    static const mp_arg_t allowed_args[] = {
        { MP_QSTR_points, MP_ARG_REQUIRED | MP_ARG_OBJ, {.u_obj = MP_OBJ_NULL} },
    };
    mp_arg_val_t args[MP_ARRAY_SIZE(allowed_args)];
    mp_arg_parse_all(n_args, pos_args, kw_args, MP_ARRAY_SIZE(allowed_args), allowed_args, args);

    ndarray_obj_t *pts_arr = ndarray_from_mp_obj(args[ARG_points].u_obj, 0);
    if(!pts_arr || pts_arr->ndim != 2)
        mp_raise_ValueError(MP_ERROR_TEXT("points must be Nx2 ndarray"));

    size_t rows = pts_arr->shape[ULAB_MAX_DIMS - 2];
    std::vector<Point2f> pts(rows);
    for(size_t r = 0; r < rows; r++)
    {
        mp_float_t x = ndarray_get_float_index(pts_arr->array, pts_arr->dtype, r * 2);
        mp_float_t y = ndarray_get_float_index(pts_arr->array, pts_arr->dtype, r * 2 + 1);
        pts[r] = Point2f(x, y);
    }

    RotatedRect r;
    try {
        r = minAreaRect(pts);
    } catch(Exception &e) {
        mp_raise_msg(&mp_type_Exception, MP_ERROR_TEXT(e.what()));
    }

    mp_obj_t center_arr[] = { mp_obj_new_float(r.center.x), mp_obj_new_float(r.center.y) };
    mp_obj_t size_arr[] = { mp_obj_new_float(r.size.width), mp_obj_new_float(r.size.height) };
    mp_obj_t result[3];
    result[0] = mp_obj_new_tuple(2, center_arr);
    result[1] = mp_obj_new_tuple(2, size_arr);
    result[2] = mp_obj_new_float(r.angle);
    return mp_obj_new_tuple(3, result);
}
MP_DEFINE_CONST_FUN_OBJ_KW(cv2_imgproc_minAreaRect_obj, 1, cv2_imgproc_minAreaRect_fun);

STATIC mp_obj_t cv2_imgproc_minEnclosingCircle_fun(size_t n_args, const mp_obj_t *pos_args, mp_map_t *kw_args)
{
    enum { ARG_points };
    static const mp_arg_t allowed_args[] = {
        { MP_QSTR_points, MP_ARG_REQUIRED | MP_ARG_OBJ, {.u_obj = MP_OBJ_NULL} },
    };
    mp_arg_val_t args[MP_ARRAY_SIZE(allowed_args)];
    mp_arg_parse_all(n_args, pos_args, kw_args, MP_ARRAY_SIZE(allowed_args), allowed_args, args);

    ndarray_obj_t *pts_arr = ndarray_from_mp_obj(args[ARG_points].u_obj, 0);
    if(!pts_arr || pts_arr->ndim != 2)
        mp_raise_ValueError(MP_ERROR_TEXT("points must be Nx2 ndarray"));

    size_t rows = pts_arr->shape[ULAB_MAX_DIMS - 2];
    std::vector<Point2f> pts(rows);
    for(size_t r = 0; r < rows; r++)
    {
        mp_float_t x = ndarray_get_float_index(pts_arr->array, pts_arr->dtype, r * 2);
        mp_float_t y = ndarray_get_float_index(pts_arr->array, pts_arr->dtype, r * 2 + 1);
        pts[r] = Point2f(x, y);
    }

    Point2f center;
    float radius;
    try {
        minEnclosingCircle(pts, center, radius);
    } catch(Exception &e) {
        mp_raise_msg(&mp_type_Exception, MP_ERROR_TEXT(e.what()));
    }

    mp_obj_t center_arr[] = { mp_obj_new_float(center.x), mp_obj_new_float(center.y) };
    mp_obj_t result[2];
    result[0] = mp_obj_new_tuple(2, center_arr);
    result[1] = mp_obj_new_float(radius);
    return mp_obj_new_tuple(2, result);
}
MP_DEFINE_CONST_FUN_OBJ_KW(cv2_imgproc_minEnclosingCircle_obj, 1, cv2_imgproc_minEnclosingCircle_fun);

STATIC mp_obj_t cv2_imgproc_matchTemplate_fun(size_t n_args, const mp_obj_t *pos_args, mp_map_t *kw_args)
{
    enum { ARG_image, ARG_templ, ARG_method, ARG_result, ARG_mask };
    static const mp_arg_t allowed_args[] = {
        { MP_QSTR_image,  MP_ARG_REQUIRED | MP_ARG_OBJ, {.u_obj = MP_OBJ_NULL} },
        { MP_QSTR_templ,  MP_ARG_REQUIRED | MP_ARG_OBJ, {.u_obj = MP_OBJ_NULL} },
        { MP_QSTR_method, MP_ARG_REQUIRED | MP_ARG_INT, {.u_int = 0} },
        { MP_QSTR_result, MP_ARG_OBJ, {.u_obj = mp_const_none} },
        { MP_QSTR_mask,   MP_ARG_OBJ, {.u_obj = mp_const_none} },
    };
    mp_arg_val_t args[MP_ARRAY_SIZE(allowed_args)];
    mp_arg_parse_all(n_args, pos_args, kw_args, MP_ARRAY_SIZE(allowed_args), allowed_args, args);

    Mat image = mp_obj_to_mat(args[ARG_image].u_obj);
    Mat templ = mp_obj_to_mat(args[ARG_templ].u_obj);
    int method = args[ARG_method].u_int;
    Mat result = mp_obj_to_mat(args[ARG_result].u_obj);
    Mat mask = mp_obj_to_mat(args[ARG_mask].u_obj);

    try {
        matchTemplate(image, templ, result, method, mask.empty() ? noArray() : (InputArray)mask);
    } catch(Exception &e) {
        mp_raise_msg(&mp_type_Exception, MP_ERROR_TEXT(e.what()));
    }
    return mat_to_mp_obj(result);
}
MP_DEFINE_CONST_FUN_OBJ_KW(cv2_imgproc_matchTemplate_obj, 1, cv2_imgproc_matchTemplate_fun);

STATIC mp_obj_t cv2_imgproc_moments_fun(size_t n_args, const mp_obj_t *pos_args, mp_map_t *kw_args)
{
    enum { ARG_array, ARG_binaryImage };
    static const mp_arg_t allowed_args[] = {
        { MP_QSTR_array,       MP_ARG_REQUIRED | MP_ARG_OBJ, {.u_obj = MP_OBJ_NULL} },
        { MP_QSTR_binaryImage, MP_ARG_BOOL, {.u_bool = false} },
    };
    mp_arg_val_t args[MP_ARRAY_SIZE(allowed_args)];
    mp_arg_parse_all(n_args, pos_args, kw_args, MP_ARRAY_SIZE(allowed_args), allowed_args, args);

    Mat array = mp_obj_to_mat(args[ARG_array].u_obj);
    bool binaryImage = args[ARG_binaryImage].u_bool;

    Moments m;
    try {
        m = moments(array, binaryImage);
    } catch(Exception &e) {
        mp_raise_msg(&mp_type_Exception, MP_ERROR_TEXT(e.what()));
    }

    mp_obj_t dict = mp_obj_new_dict(0);
    mp_obj_dict_store(dict, MP_OBJ_NEW_QSTR(MP_QSTR_m00), mp_obj_new_float(m.m00));
    mp_obj_dict_store(dict, MP_OBJ_NEW_QSTR(MP_QSTR_m10), mp_obj_new_float(m.m10));
    mp_obj_dict_store(dict, MP_OBJ_NEW_QSTR(MP_QSTR_m01), mp_obj_new_float(m.m01));
    mp_obj_dict_store(dict, MP_OBJ_NEW_QSTR(MP_QSTR_m20), mp_obj_new_float(m.m20));
    mp_obj_dict_store(dict, MP_OBJ_NEW_QSTR(MP_QSTR_m11), mp_obj_new_float(m.m11));
    mp_obj_dict_store(dict, MP_OBJ_NEW_QSTR(MP_QSTR_m02), mp_obj_new_float(m.m02));
    mp_obj_dict_store(dict, MP_OBJ_NEW_QSTR(MP_QSTR_m30), mp_obj_new_float(m.m30));
    mp_obj_dict_store(dict, MP_OBJ_NEW_QSTR(MP_QSTR_m21), mp_obj_new_float(m.m21));
    mp_obj_dict_store(dict, MP_OBJ_NEW_QSTR(MP_QSTR_m12), mp_obj_new_float(m.m12));
    mp_obj_dict_store(dict, MP_OBJ_NEW_QSTR(MP_QSTR_m03), mp_obj_new_float(m.m03));
    mp_obj_dict_store(dict, MP_OBJ_NEW_QSTR(MP_QSTR_mu20), mp_obj_new_float(m.mu20));
    mp_obj_dict_store(dict, MP_OBJ_NEW_QSTR(MP_QSTR_mu11), mp_obj_new_float(m.mu11));
    mp_obj_dict_store(dict, MP_OBJ_NEW_QSTR(MP_QSTR_mu02), mp_obj_new_float(m.mu02));
    mp_obj_dict_store(dict, MP_OBJ_NEW_QSTR(MP_QSTR_mu30), mp_obj_new_float(m.mu30));
    mp_obj_dict_store(dict, MP_OBJ_NEW_QSTR(MP_QSTR_mu21), mp_obj_new_float(m.mu21));
    mp_obj_dict_store(dict, MP_OBJ_NEW_QSTR(MP_QSTR_mu12), mp_obj_new_float(m.mu12));
    mp_obj_dict_store(dict, MP_OBJ_NEW_QSTR(MP_QSTR_mu03), mp_obj_new_float(m.mu03));
    mp_obj_dict_store(dict, MP_OBJ_NEW_QSTR(MP_QSTR_nu20), mp_obj_new_float(m.nu20));
    mp_obj_dict_store(dict, MP_OBJ_NEW_QSTR(MP_QSTR_nu11), mp_obj_new_float(m.nu11));
    mp_obj_dict_store(dict, MP_OBJ_NEW_QSTR(MP_QSTR_nu02), mp_obj_new_float(m.nu02));
    mp_obj_dict_store(dict, MP_OBJ_NEW_QSTR(MP_QSTR_nu30), mp_obj_new_float(m.nu30));
    mp_obj_dict_store(dict, MP_OBJ_NEW_QSTR(MP_QSTR_nu21), mp_obj_new_float(m.nu21));
    mp_obj_dict_store(dict, MP_OBJ_NEW_QSTR(MP_QSTR_nu12), mp_obj_new_float(m.nu12));
    mp_obj_dict_store(dict, MP_OBJ_NEW_QSTR(MP_QSTR_nu03), mp_obj_new_float(m.nu03));
    return dict;
}
MP_DEFINE_CONST_FUN_OBJ_KW(cv2_imgproc_moments_obj, 1, cv2_imgproc_moments_fun);

STATIC mp_obj_t cv2_imgproc_HoughLines_fun(size_t n_args, const mp_obj_t *pos_args, mp_map_t *kw_args)
{
    enum { ARG_image, ARG_rho, ARG_theta, ARG_threshold, ARG_lines, ARG_srn, ARG_stn, ARG_min_theta, ARG_max_theta };
    static const mp_arg_t allowed_args[] = {
        { MP_QSTR_image,     MP_ARG_REQUIRED | MP_ARG_OBJ, {.u_obj = MP_OBJ_NULL} },
        { MP_QSTR_rho,       MP_ARG_REQUIRED | MP_ARG_OBJ, {.u_obj = MP_OBJ_NULL} },
        { MP_QSTR_theta,     MP_ARG_REQUIRED | MP_ARG_OBJ, {.u_obj = MP_OBJ_NULL} },
        { MP_QSTR_threshold, MP_ARG_REQUIRED | MP_ARG_INT, {.u_int = 0} },
        { MP_QSTR_lines,     MP_ARG_OBJ, {.u_obj = mp_const_none} },
        { MP_QSTR_srn,       MP_ARG_OBJ, {.u_obj = mp_const_none} },
        { MP_QSTR_stn,       MP_ARG_OBJ, {.u_obj = mp_const_none} },
        { MP_QSTR_min_theta, MP_ARG_OBJ, {.u_obj = mp_const_none} },
        { MP_QSTR_max_theta, MP_ARG_OBJ, {.u_obj = mp_const_none} },
    };
    mp_arg_val_t args[MP_ARRAY_SIZE(allowed_args)];
    mp_arg_parse_all(n_args, pos_args, kw_args, MP_ARRAY_SIZE(allowed_args), allowed_args, args);

    Mat image = mp_obj_to_mat(args[ARG_image].u_obj);
    double rho = mp_obj_get_float(args[ARG_rho].u_obj);
    double theta = mp_obj_get_float(args[ARG_theta].u_obj);
    int threshold = args[ARG_threshold].u_int;
    double srn = args[ARG_srn].u_obj == mp_const_none ? 0 : mp_obj_get_float(args[ARG_srn].u_obj);
    double stn = args[ARG_stn].u_obj == mp_const_none ? 0 : mp_obj_get_float(args[ARG_stn].u_obj);
    double min_theta = args[ARG_min_theta].u_obj == mp_const_none ? 0 : mp_obj_get_float(args[ARG_min_theta].u_obj);
    double max_theta = args[ARG_max_theta].u_obj == mp_const_none ? CV_PI : mp_obj_get_float(args[ARG_max_theta].u_obj);

    std::vector<Vec2f> lines;
    try {
        HoughLines(image, lines, rho, theta, threshold, srn, stn, min_theta, max_theta);
    } catch(Exception &e) {
        mp_raise_msg(&mp_type_Exception, MP_ERROR_TEXT(e.what()));
    }

    size_t shape[ULAB_MAX_DIMS];
    for(int i = 0; i < ULAB_MAX_DIMS - 2; i++)
        shape[i] = 0;
    shape[ULAB_MAX_DIMS - 2] = lines.size();
    shape[ULAB_MAX_DIMS - 1] = 2;

    ndarray_obj_t *result_arr = ndarray_new_dense_ndarray(2, shape, NDARRAY_FLOAT);
    mp_float_t *data = (mp_float_t *)result_arr->array;
    for(size_t i = 0; i < lines.size(); i++)
    {
        data[i * 2] = lines[i][0];
        data[i * 2 + 1] = lines[i][1];
    }
    return MP_OBJ_FROM_PTR(result_arr);
}
MP_DEFINE_CONST_FUN_OBJ_KW(cv2_imgproc_HoughLines_obj, 1, cv2_imgproc_HoughLines_fun);

STATIC mp_obj_t cv2_imgproc_HoughLinesP_fun(size_t n_args, const mp_obj_t *pos_args, mp_map_t *kw_args)
{
    enum { ARG_image, ARG_rho, ARG_theta, ARG_threshold, ARG_lines, ARG_minLineLength, ARG_maxLineGap };
    static const mp_arg_t allowed_args[] = {
        { MP_QSTR_image,         MP_ARG_REQUIRED | MP_ARG_OBJ, {.u_obj = MP_OBJ_NULL} },
        { MP_QSTR_rho,           MP_ARG_REQUIRED | MP_ARG_OBJ, {.u_obj = MP_OBJ_NULL} },
        { MP_QSTR_theta,         MP_ARG_REQUIRED | MP_ARG_OBJ, {.u_obj = MP_OBJ_NULL} },
        { MP_QSTR_threshold,     MP_ARG_REQUIRED | MP_ARG_INT, {.u_int = 0} },
        { MP_QSTR_lines,         MP_ARG_OBJ, {.u_obj = mp_const_none} },
        { MP_QSTR_minLineLength, MP_ARG_OBJ, {.u_obj = mp_const_none} },
        { MP_QSTR_maxLineGap,    MP_ARG_OBJ, {.u_obj = mp_const_none} },
    };
    mp_arg_val_t args[MP_ARRAY_SIZE(allowed_args)];
    mp_arg_parse_all(n_args, pos_args, kw_args, MP_ARRAY_SIZE(allowed_args), allowed_args, args);

    Mat image = mp_obj_to_mat(args[ARG_image].u_obj);
    double rho = mp_obj_get_float(args[ARG_rho].u_obj);
    double theta = mp_obj_get_float(args[ARG_theta].u_obj);
    int threshold = args[ARG_threshold].u_int;
    double minLineLength = args[ARG_minLineLength].u_obj == mp_const_none ? 0 : mp_obj_get_float(args[ARG_minLineLength].u_obj);
    double maxLineGap = args[ARG_maxLineGap].u_obj == mp_const_none ? 0 : mp_obj_get_float(args[ARG_maxLineGap].u_obj);

    std::vector<Vec4i> lines;
    try {
        HoughLinesP(image, lines, rho, theta, threshold, minLineLength, maxLineGap);
    } catch(Exception &e) {
        mp_raise_msg(&mp_type_Exception, MP_ERROR_TEXT(e.what()));
    }

    size_t shape[ULAB_MAX_DIMS];
    for(int i = 0; i < ULAB_MAX_DIMS - 2; i++)
        shape[i] = 0;
    shape[ULAB_MAX_DIMS - 2] = lines.size();
    shape[ULAB_MAX_DIMS - 1] = 4;

    ndarray_obj_t *result_arr = ndarray_new_dense_ndarray(2, shape, NDARRAY_FLOAT);
    mp_float_t *data = (mp_float_t *)result_arr->array;
    for(size_t i = 0; i < lines.size(); i++)
    {
        data[i * 4] = (float)lines[i][0];
        data[i * 4 + 1] = (float)lines[i][1];
        data[i * 4 + 2] = (float)lines[i][2];
        data[i * 4 + 3] = (float)lines[i][3];
    }
    return MP_OBJ_FROM_PTR(result_arr);
}
MP_DEFINE_CONST_FUN_OBJ_KW(cv2_imgproc_HoughLinesP_obj, 1, cv2_imgproc_HoughLinesP_fun);

STATIC mp_obj_t cv2_imgproc_HoughCircles_fun(size_t n_args, const mp_obj_t *pos_args, mp_map_t *kw_args)
{
    enum { ARG_image, ARG_method, ARG_dp, ARG_minDist, ARG_circles, ARG_param1, ARG_param2, ARG_minRadius, ARG_maxRadius };
    static const mp_arg_t allowed_args[] = {
        { MP_QSTR_image,     MP_ARG_REQUIRED | MP_ARG_OBJ, {.u_obj = MP_OBJ_NULL} },
        { MP_QSTR_method,    MP_ARG_REQUIRED | MP_ARG_INT, {.u_int = 0} },
        { MP_QSTR_dp,        MP_ARG_REQUIRED | MP_ARG_OBJ, {.u_obj = MP_OBJ_NULL} },
        { MP_QSTR_minDist,   MP_ARG_REQUIRED | MP_ARG_OBJ, {.u_obj = MP_OBJ_NULL} },
        { MP_QSTR_circles,   MP_ARG_OBJ, {.u_obj = mp_const_none} },
        { MP_QSTR_param1,    MP_ARG_OBJ, {.u_obj = mp_const_none} },
        { MP_QSTR_param2,    MP_ARG_OBJ, {.u_obj = mp_const_none} },
        { MP_QSTR_minRadius, MP_ARG_INT, {.u_int = 0} },
        { MP_QSTR_maxRadius, MP_ARG_INT, {.u_int = 0} },
    };
    mp_arg_val_t args[MP_ARRAY_SIZE(allowed_args)];
    mp_arg_parse_all(n_args, pos_args, kw_args, MP_ARRAY_SIZE(allowed_args), allowed_args, args);

    Mat image = mp_obj_to_mat(args[ARG_image].u_obj);
    int method = args[ARG_method].u_int;
    double dp = mp_obj_get_float(args[ARG_dp].u_obj);
    double minDist = mp_obj_get_float(args[ARG_minDist].u_obj);
    double param1 = args[ARG_param1].u_obj == mp_const_none ? 100 : mp_obj_get_float(args[ARG_param1].u_obj);
    double param2 = args[ARG_param2].u_obj == mp_const_none ? 100 : mp_obj_get_float(args[ARG_param2].u_obj);
    int minRadius = args[ARG_minRadius].u_int;
    int maxRadius = args[ARG_maxRadius].u_int;

    std::vector<Vec3f> circles;
    try {
        HoughCircles(image, circles, method, dp, minDist, param1, param2, minRadius, maxRadius);
    } catch(Exception &e) {
        mp_raise_msg(&mp_type_Exception, MP_ERROR_TEXT(e.what()));
    }

    size_t shape[ULAB_MAX_DIMS];
    for(int i = 0; i < ULAB_MAX_DIMS - 2; i++)
        shape[i] = 0;
    shape[ULAB_MAX_DIMS - 2] = circles.size();
    shape[ULAB_MAX_DIMS - 1] = 3;

    ndarray_obj_t *result_arr = ndarray_new_dense_ndarray(2, shape, NDARRAY_FLOAT);
    mp_float_t *data = (mp_float_t *)result_arr->array;
    for(size_t i = 0; i < circles.size(); i++)
    {
        data[i * 3] = circles[i][0];
        data[i * 3 + 1] = circles[i][1];
        data[i * 3 + 2] = circles[i][2];
    }
    return MP_OBJ_FROM_PTR(result_arr);
}
MP_DEFINE_CONST_FUN_OBJ_KW(cv2_imgproc_HoughCircles_obj, 1, cv2_imgproc_HoughCircles_fun);

STATIC mp_obj_t cv2_imgproc_equalizeHist_fun(size_t n_args, const mp_obj_t *pos_args, mp_map_t *kw_args)
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
        equalizeHist(src, dst);
    } catch(Exception &e) {
        mp_raise_msg(&mp_type_Exception, MP_ERROR_TEXT(e.what()));
    }
    return mat_to_mp_obj(dst);
}
MP_DEFINE_CONST_FUN_OBJ_KW(cv2_imgproc_equalizeHist_obj, 1, cv2_imgproc_equalizeHist_fun);
