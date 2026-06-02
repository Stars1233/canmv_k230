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

    if(!returnPoints)
    {
        std::vector<int> hull_idx;
        try {
            convexHull(pts, hull_idx, clockwise, returnPoints);
        } catch(Exception &e) {
            mp_raise_msg(&mp_type_Exception, MP_ERROR_TEXT(e.what()));
        }
        size_t hull_shape[ULAB_MAX_DIMS];
        for(int i = 0; i < ULAB_MAX_DIMS - 1; i++)
            hull_shape[i] = 0;
        hull_shape[ULAB_MAX_DIMS - 1] = hull_idx.size();
        ndarray_obj_t *result_arr = ndarray_new_dense_ndarray(1, hull_shape, NDARRAY_FLOAT);
        mp_float_t *data = (mp_float_t *)result_arr->array;
        for(size_t i = 0; i < hull_idx.size(); i++)
            data[i] = (float)hull_idx[i];
        return MP_OBJ_FROM_PTR(result_arr);
    }

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

// ---- Pyramid ----

STATIC mp_obj_t cv2_imgproc_pyrDown_fun(size_t n_args, const mp_obj_t *pos_args, mp_map_t *kw_args)
{
    enum { ARG_src, ARG_dst, ARG_dstsize, ARG_borderType };
    static const mp_arg_t allowed_args[] = {
        { MP_QSTR_src,        MP_ARG_REQUIRED | MP_ARG_OBJ, {.u_obj = MP_OBJ_NULL} },
        { MP_QSTR_dst,        MP_ARG_OBJ, {.u_obj = mp_const_none} },
        { MP_QSTR_dstsize,    MP_ARG_OBJ, {.u_obj = mp_const_none} },
        { MP_QSTR_borderType, MP_ARG_INT, {.u_int = BORDER_DEFAULT} },
    };
    mp_arg_val_t args[MP_ARRAY_SIZE(allowed_args)];
    mp_arg_parse_all(n_args, pos_args, kw_args, MP_ARRAY_SIZE(allowed_args), allowed_args, args);

    Mat src = mp_obj_to_mat(args[ARG_src].u_obj);
    Mat dst = mp_obj_to_mat(args[ARG_dst].u_obj);
    Size dstsize = args[ARG_dstsize].u_obj == mp_const_none ? Size() : mp_obj_to_size(args[ARG_dstsize].u_obj);
    int borderType = args[ARG_borderType].u_int;

    try {
        pyrDown(src, dst, dstsize, borderType);
    } catch(Exception &e) {
        mp_raise_msg(&mp_type_Exception, MP_ERROR_TEXT(e.what()));
    }
    return mat_to_mp_obj(dst);
}
MP_DEFINE_CONST_FUN_OBJ_KW(cv2_imgproc_pyrDown_obj, 1, cv2_imgproc_pyrDown_fun);

STATIC mp_obj_t cv2_imgproc_pyrUp_fun(size_t n_args, const mp_obj_t *pos_args, mp_map_t *kw_args)
{
    enum { ARG_src, ARG_dst, ARG_dstsize, ARG_borderType };
    static const mp_arg_t allowed_args[] = {
        { MP_QSTR_src,        MP_ARG_REQUIRED | MP_ARG_OBJ, {.u_obj = MP_OBJ_NULL} },
        { MP_QSTR_dst,        MP_ARG_OBJ, {.u_obj = mp_const_none} },
        { MP_QSTR_dstsize,    MP_ARG_OBJ, {.u_obj = mp_const_none} },
        { MP_QSTR_borderType, MP_ARG_INT, {.u_int = BORDER_DEFAULT} },
    };
    mp_arg_val_t args[MP_ARRAY_SIZE(allowed_args)];
    mp_arg_parse_all(n_args, pos_args, kw_args, MP_ARRAY_SIZE(allowed_args), allowed_args, args);

    Mat src = mp_obj_to_mat(args[ARG_src].u_obj);
    Mat dst = mp_obj_to_mat(args[ARG_dst].u_obj);
    Size dstsize = args[ARG_dstsize].u_obj == mp_const_none ? Size() : mp_obj_to_size(args[ARG_dstsize].u_obj);
    int borderType = args[ARG_borderType].u_int;

    try {
        pyrUp(src, dst, dstsize, borderType);
    } catch(Exception &e) {
        mp_raise_msg(&mp_type_Exception, MP_ERROR_TEXT(e.what()));
    }
    return mat_to_mp_obj(dst);
}
MP_DEFINE_CONST_FUN_OBJ_KW(cv2_imgproc_pyrUp_obj, 1, cv2_imgproc_pyrUp_fun);

STATIC mp_obj_t cv2_imgproc_remap_fun(size_t n_args, const mp_obj_t *pos_args, mp_map_t *kw_args)
{
    enum { ARG_src, ARG_map1, ARG_map2, ARG_interpolation, ARG_dst, ARG_borderMode, ARG_borderValue };
    static const mp_arg_t allowed_args[] = {
        { MP_QSTR_src,           MP_ARG_REQUIRED | MP_ARG_OBJ, {.u_obj = MP_OBJ_NULL} },
        { MP_QSTR_map1,          MP_ARG_REQUIRED | MP_ARG_OBJ, {.u_obj = MP_OBJ_NULL} },
        { MP_QSTR_map2,          MP_ARG_REQUIRED | MP_ARG_OBJ, {.u_obj = MP_OBJ_NULL} },
        { MP_QSTR_interpolation, MP_ARG_REQUIRED | MP_ARG_INT, {.u_int = INTER_LINEAR} },
        { MP_QSTR_dst,           MP_ARG_OBJ, {.u_obj = mp_const_none} },
        { MP_QSTR_borderMode,    MP_ARG_INT, {.u_int = BORDER_CONSTANT} },
        { MP_QSTR_borderValue,   MP_ARG_OBJ, {.u_obj = mp_const_none} },
    };
    mp_arg_val_t args[MP_ARRAY_SIZE(allowed_args)];
    mp_arg_parse_all(n_args, pos_args, kw_args, MP_ARRAY_SIZE(allowed_args), allowed_args, args);

    Mat src = mp_obj_to_mat(args[ARG_src].u_obj);
    Mat map1 = mp_obj_to_mat(args[ARG_map1].u_obj);
    Mat map2 = mp_obj_to_mat(args[ARG_map2].u_obj);
    int interpolation = args[ARG_interpolation].u_int;
    Mat dst = mp_obj_to_mat(args[ARG_dst].u_obj);
    int borderMode = args[ARG_borderMode].u_int;
    Scalar borderValue = args[ARG_borderValue].u_obj == mp_const_none ? Scalar() : mp_obj_to_scalar(args[ARG_borderValue].u_obj);

    try {
        remap(src, dst, map1, map2, interpolation, borderMode, borderValue);
    } catch(Exception &e) {
        mp_raise_msg(&mp_type_Exception, MP_ERROR_TEXT(e.what()));
    }
    return mat_to_mp_obj(dst);
}
MP_DEFINE_CONST_FUN_OBJ_KW(cv2_imgproc_remap_obj, 1, cv2_imgproc_remap_fun);

// ---- Affine/Perspective Transform ----

STATIC mp_obj_t cv2_imgproc_getAffineTransform_fun(size_t n_args, const mp_obj_t *pos_args, mp_map_t *kw_args)
{
    enum { ARG_src, ARG_dst };
    static const mp_arg_t allowed_args[] = {
        { MP_QSTR_src, MP_ARG_REQUIRED | MP_ARG_OBJ, {.u_obj = MP_OBJ_NULL} },
        { MP_QSTR_dst, MP_ARG_REQUIRED | MP_ARG_OBJ, {.u_obj = MP_OBJ_NULL} },
    };
    mp_arg_val_t args[MP_ARRAY_SIZE(allowed_args)];
    mp_arg_parse_all(n_args, pos_args, kw_args, MP_ARRAY_SIZE(allowed_args), allowed_args, args);

    Mat src = mp_obj_to_mat(args[ARG_src].u_obj);
    Mat dst = mp_obj_to_mat(args[ARG_dst].u_obj);

    Mat retval;
    try {
        retval = getAffineTransform(src, dst);
    } catch(Exception &e) {
        mp_raise_msg(&mp_type_Exception, MP_ERROR_TEXT(e.what()));
    }
    return mat_to_mp_obj(retval);
}
MP_DEFINE_CONST_FUN_OBJ_KW(cv2_imgproc_getAffineTransform_obj, 1, cv2_imgproc_getAffineTransform_fun);

STATIC mp_obj_t cv2_imgproc_getPerspectiveTransform_fun(size_t n_args, const mp_obj_t *pos_args, mp_map_t *kw_args)
{
    enum { ARG_src, ARG_dst, ARG_solveMethod };
    static const mp_arg_t allowed_args[] = {
        { MP_QSTR_src,         MP_ARG_REQUIRED | MP_ARG_OBJ, {.u_obj = MP_OBJ_NULL} },
        { MP_QSTR_dst,         MP_ARG_REQUIRED | MP_ARG_OBJ, {.u_obj = MP_OBJ_NULL} },
        { MP_QSTR_solveMethod, MP_ARG_INT, {.u_int = cv::DECOMP_LU} },
    };
    mp_arg_val_t args[MP_ARRAY_SIZE(allowed_args)];
    mp_arg_parse_all(n_args, pos_args, kw_args, MP_ARRAY_SIZE(allowed_args), allowed_args, args);

    Mat src = mp_obj_to_mat(args[ARG_src].u_obj);
    Mat dst = mp_obj_to_mat(args[ARG_dst].u_obj);
    int solveMethod = args[ARG_solveMethod].u_int;

    Mat retval;
    try {
        retval = getPerspectiveTransform(src, dst, solveMethod);
    } catch(Exception &e) {
        mp_raise_msg(&mp_type_Exception, MP_ERROR_TEXT(e.what()));
    }
    return mat_to_mp_obj(retval);
}
MP_DEFINE_CONST_FUN_OBJ_KW(cv2_imgproc_getPerspectiveTransform_obj, 1, cv2_imgproc_getPerspectiveTransform_fun);

// ---- Corner Detection ----

STATIC mp_obj_t cv2_imgproc_cornerHarris_fun(size_t n_args, const mp_obj_t *pos_args, mp_map_t *kw_args)
{
    enum { ARG_src, ARG_blockSize, ARG_ksize, ARG_k, ARG_dst, ARG_borderType };
    static const mp_arg_t allowed_args[] = {
        { MP_QSTR_src,        MP_ARG_REQUIRED | MP_ARG_OBJ, {.u_obj = MP_OBJ_NULL} },
        { MP_QSTR_blockSize,  MP_ARG_REQUIRED | MP_ARG_INT, {.u_int = 0} },
        { MP_QSTR_ksize,      MP_ARG_REQUIRED | MP_ARG_INT, {.u_int = 3} },
        { MP_QSTR_k,          MP_ARG_REQUIRED | MP_ARG_OBJ, {.u_obj = MP_OBJ_NULL} },
        { MP_QSTR_dst,        MP_ARG_OBJ, {.u_obj = mp_const_none} },
        { MP_QSTR_borderType, MP_ARG_INT, {.u_int = BORDER_DEFAULT} },
    };
    mp_arg_val_t args[MP_ARRAY_SIZE(allowed_args)];
    mp_arg_parse_all(n_args, pos_args, kw_args, MP_ARRAY_SIZE(allowed_args), allowed_args, args);

    Mat src = mp_obj_to_mat(args[ARG_src].u_obj);
    int blockSize = args[ARG_blockSize].u_int;
    int ksize = args[ARG_ksize].u_int;
    double k = mp_obj_get_float(args[ARG_k].u_obj);
    Mat dst = mp_obj_to_mat(args[ARG_dst].u_obj);
    int borderType = args[ARG_borderType].u_int;

    try {
        cornerHarris(src, dst, blockSize, ksize, k, borderType);
    } catch(Exception &e) {
        mp_raise_msg(&mp_type_Exception, MP_ERROR_TEXT(e.what()));
    }
    return mat_to_mp_obj(dst);
}
MP_DEFINE_CONST_FUN_OBJ_KW(cv2_imgproc_cornerHarris_obj, 1, cv2_imgproc_cornerHarris_fun);

STATIC mp_obj_t cv2_imgproc_goodFeaturesToTrack_fun(size_t n_args, const mp_obj_t *pos_args, mp_map_t *kw_args)
{
    enum { ARG_image, ARG_maxCorners, ARG_qualityLevel, ARG_minDistance, ARG_mask, ARG_blockSize, ARG_useHarrisDetector, ARG_k, ARG_corners };
    static const mp_arg_t allowed_args[] = {
        { MP_QSTR_image,             MP_ARG_REQUIRED | MP_ARG_OBJ, {.u_obj = MP_OBJ_NULL} },
        { MP_QSTR_maxCorners,        MP_ARG_REQUIRED | MP_ARG_INT, {.u_int = 0} },
        { MP_QSTR_qualityLevel,      MP_ARG_REQUIRED | MP_ARG_OBJ, {.u_obj = MP_OBJ_NULL} },
        { MP_QSTR_minDistance,       MP_ARG_REQUIRED | MP_ARG_OBJ, {.u_obj = MP_OBJ_NULL} },
        { MP_QSTR_mask,              MP_ARG_OBJ, {.u_obj = mp_const_none} },
        { MP_QSTR_blockSize,         MP_ARG_INT, {.u_int = 3} },
        { MP_QSTR_useHarrisDetector, MP_ARG_BOOL, {.u_bool = false} },
        { MP_QSTR_k,                 MP_ARG_OBJ, {.u_obj = mp_const_none} },
        { MP_QSTR_corners,           MP_ARG_OBJ, {.u_obj = mp_const_none} },
    };
    mp_arg_val_t args[MP_ARRAY_SIZE(allowed_args)];
    mp_arg_parse_all(n_args, pos_args, kw_args, MP_ARRAY_SIZE(allowed_args), allowed_args, args);

    Mat image = mp_obj_to_mat(args[ARG_image].u_obj);
    int maxCorners = args[ARG_maxCorners].u_int;
    double qualityLevel = mp_obj_get_float(args[ARG_qualityLevel].u_obj);
    double minDistance = mp_obj_get_float(args[ARG_minDistance].u_obj);
    Mat mask = mp_obj_to_mat(args[ARG_mask].u_obj);
    int blockSize = args[ARG_blockSize].u_int;
    bool useHarrisDetector = args[ARG_useHarrisDetector].u_bool;
    double k = args[ARG_k].u_obj == mp_const_none ? 0.04 : mp_obj_get_float(args[ARG_k].u_obj);

    std::vector<Point2f> corners;
    try {
        goodFeaturesToTrack(image, corners, maxCorners, qualityLevel, minDistance,
                            mask.empty() ? noArray() : (InputArray)mask, blockSize, useHarrisDetector, k);
    } catch(Exception &e) {
        mp_raise_msg(&mp_type_Exception, MP_ERROR_TEXT(e.what()));
    }

    size_t shape[ULAB_MAX_DIMS];
    for(int i = 0; i < ULAB_MAX_DIMS - 2; i++)
        shape[i] = 0;
    shape[ULAB_MAX_DIMS - 2] = corners.size();
    shape[ULAB_MAX_DIMS - 1] = 2;

    ndarray_obj_t *result_arr = ndarray_new_dense_ndarray(2, shape, NDARRAY_FLOAT);
    mp_float_t *data = (mp_float_t *)result_arr->array;
    for(size_t i = 0; i < corners.size(); i++)
    {
        data[i * 2] = corners[i].x;
        data[i * 2 + 1] = corners[i].y;
    }
    return MP_OBJ_FROM_PTR(result_arr);
}
MP_DEFINE_CONST_FUN_OBJ_KW(cv2_imgproc_goodFeaturesToTrack_obj, 1, cv2_imgproc_goodFeaturesToTrack_fun);

STATIC mp_obj_t cv2_imgproc_cornerSubPix_fun(size_t n_args, const mp_obj_t *pos_args, mp_map_t *kw_args)
{
    enum { ARG_image, ARG_corners, ARG_winSize, ARG_zeroZone, ARG_criteria };
    static const mp_arg_t allowed_args[] = {
        { MP_QSTR_image,    MP_ARG_REQUIRED | MP_ARG_OBJ, {.u_obj = MP_OBJ_NULL} },
        { MP_QSTR_corners,  MP_ARG_REQUIRED | MP_ARG_OBJ, {.u_obj = MP_OBJ_NULL} },
        { MP_QSTR_winSize,  MP_ARG_REQUIRED | MP_ARG_OBJ, {.u_obj = MP_OBJ_NULL} },
        { MP_QSTR_zeroZone, MP_ARG_REQUIRED | MP_ARG_OBJ, {.u_obj = MP_OBJ_NULL} },
        { MP_QSTR_criteria, MP_ARG_REQUIRED | MP_ARG_OBJ, {.u_obj = MP_OBJ_NULL} },
    };
    mp_arg_val_t args[MP_ARRAY_SIZE(allowed_args)];
    mp_arg_parse_all(n_args, pos_args, kw_args, MP_ARRAY_SIZE(allowed_args), allowed_args, args);

    Mat image = mp_obj_to_mat(args[ARG_image].u_obj);
    Size winSize = mp_obj_to_size(args[ARG_winSize].u_obj);
    Size zeroZone = mp_obj_to_size(args[ARG_zeroZone].u_obj);

    ndarray_obj_t *corners_arr = ndarray_from_mp_obj(args[ARG_corners].u_obj, 0);
    if(!corners_arr || corners_arr->ndim != 2)
        mp_raise_ValueError(MP_ERROR_TEXT("corners must be Nx2 ndarray"));

    size_t rows = corners_arr->shape[ULAB_MAX_DIMS - 2];
    std::vector<Point2f> corners(rows);
    for(size_t r = 0; r < rows; r++)
    {
        mp_float_t x = ndarray_get_float_index(corners_arr->array, corners_arr->dtype, r * 2);
        mp_float_t y = ndarray_get_float_index(corners_arr->array, corners_arr->dtype, r * 2 + 1);
        corners[r] = Point2f(x, y);
    }

    // criteria: tuple of (type, maxCount, epsilon)
    mp_obj_t criteria_obj = args[ARG_criteria].u_obj;
    size_t clen;
    mp_obj_t *citems;
    mp_obj_get_array(criteria_obj, &clen, &citems);
    if(clen < 3)
        mp_raise_ValueError(MP_ERROR_TEXT("criteria must be (type, maxCount, epsilon)"));

    int termType = mp_obj_get_int(citems[0]);
    int maxCount = mp_obj_get_int(citems[1]);
    double epsilon = mp_obj_get_float(citems[2]);
    TermCriteria criteria(termType, maxCount, epsilon);

    try {
        cornerSubPix(image, corners, winSize, zeroZone, criteria);
    } catch(Exception &e) {
        mp_raise_msg(&mp_type_Exception, MP_ERROR_TEXT(e.what()));
    }

    ndarray_obj_t *result_arr = ndarray_new_dense_ndarray(2, corners_arr->shape, NDARRAY_FLOAT);
    mp_float_t *data = (mp_float_t *)result_arr->array;
    for(size_t i = 0; i < corners.size(); i++)
    {
        data[i * 2] = corners[i].x;
        data[i * 2 + 1] = corners[i].y;
    }
    return MP_OBJ_FROM_PTR(result_arr);
}
MP_DEFINE_CONST_FUN_OBJ_KW(cv2_imgproc_cornerSubPix_obj, 1, cv2_imgproc_cornerSubPix_fun);

// ---- Histogram ----

STATIC mp_obj_t cv2_imgproc_calcHist_fun(size_t n_args, const mp_obj_t *pos_args, mp_map_t *kw_args)
{
    enum { ARG_images, ARG_channels, ARG_mask, ARG_histSize, ARG_ranges };
    static const mp_arg_t allowed_args[] = {
        { MP_QSTR_images,    MP_ARG_REQUIRED | MP_ARG_OBJ, {.u_obj = MP_OBJ_NULL} },
        { MP_QSTR_channels,  MP_ARG_REQUIRED | MP_ARG_OBJ, {.u_obj = MP_OBJ_NULL} },
        { MP_QSTR_mask,      MP_ARG_REQUIRED | MP_ARG_OBJ, {.u_obj = MP_OBJ_NULL} },
        { MP_QSTR_histSize,  MP_ARG_REQUIRED | MP_ARG_OBJ, {.u_obj = MP_OBJ_NULL} },
        { MP_QSTR_ranges,    MP_ARG_REQUIRED | MP_ARG_OBJ, {.u_obj = MP_OBJ_NULL} },
    };
    mp_arg_val_t args[MP_ARRAY_SIZE(allowed_args)];
    mp_arg_parse_all(n_args, pos_args, kw_args, MP_ARRAY_SIZE(allowed_args), allowed_args, args);

    mp_obj_t images_obj = args[ARG_images].u_obj;
    if(!mp_obj_is_type(images_obj, &mp_type_list) && !mp_obj_is_type(images_obj, &mp_type_tuple))
        mp_raise_TypeError(MP_ERROR_TEXT("images must be a list"));

    size_t nimgs;
    mp_obj_t *img_items;
    mp_obj_get_array(images_obj, &nimgs, &img_items);
    std::vector<Mat> images(nimgs);
    for(size_t i = 0; i < nimgs; i++)
        images[i] = mp_obj_to_mat(img_items[i]);

    mp_obj_t channels_obj = args[ARG_channels].u_obj;
    size_t nch;
    mp_obj_t *ch_items;
    mp_obj_get_array(channels_obj, &nch, &ch_items);
    std::vector<int> channels;
    for(size_t i = 0; i < nch; i++)
        channels.push_back(mp_obj_get_int(ch_items[i]));

    Mat mask = mp_obj_to_mat(args[ARG_mask].u_obj);

    mp_obj_t histSize_obj = args[ARG_histSize].u_obj;
    size_t nh;
    mp_obj_t *hs_items;
    mp_obj_get_array(histSize_obj, &nh, &hs_items);
    std::vector<int> histSize;
    for(size_t i = 0; i < nh; i++)
        histSize.push_back(mp_obj_get_int(hs_items[i]));

    mp_obj_t ranges_obj = args[ARG_ranges].u_obj;
    size_t nr;
    mp_obj_t *r_items;
    mp_obj_get_array(ranges_obj, &nr, &r_items);
    std::vector<float> ranges;
    for(size_t i = 0; i < nr; i++)
        ranges.push_back((float)mp_obj_get_float(r_items[i]));

    Mat hist;
    try {
        calcHist(images, channels, mask, hist, histSize, ranges);
    } catch(Exception &e) {
        mp_raise_msg(&mp_type_Exception, MP_ERROR_TEXT(e.what()));
    }
    return mat_to_mp_obj(hist);
}
MP_DEFINE_CONST_FUN_OBJ_KW(cv2_imgproc_calcHist_obj, 1, cv2_imgproc_calcHist_fun);

STATIC mp_obj_t cv2_imgproc_calcBackProject_fun(size_t n_args, const mp_obj_t *pos_args, mp_map_t *kw_args)
{
    enum { ARG_images, ARG_channels, ARG_hist, ARG_dst, ARG_ranges, ARG_scale };
    static const mp_arg_t allowed_args[] = {
        { MP_QSTR_images,   MP_ARG_REQUIRED | MP_ARG_OBJ, {.u_obj = MP_OBJ_NULL} },
        { MP_QSTR_channels, MP_ARG_REQUIRED | MP_ARG_OBJ, {.u_obj = MP_OBJ_NULL} },
        { MP_QSTR_hist,     MP_ARG_REQUIRED | MP_ARG_OBJ, {.u_obj = MP_OBJ_NULL} },
        { MP_QSTR_dst,      MP_ARG_OBJ, {.u_obj = mp_const_none} },
        { MP_QSTR_ranges,   MP_ARG_REQUIRED | MP_ARG_OBJ, {.u_obj = MP_OBJ_NULL} },
        { MP_QSTR_scale,    MP_ARG_OBJ, {.u_obj = mp_const_none} },
    };
    mp_arg_val_t args[MP_ARRAY_SIZE(allowed_args)];
    mp_arg_parse_all(n_args, pos_args, kw_args, MP_ARRAY_SIZE(allowed_args), allowed_args, args);

    mp_obj_t images_obj = args[ARG_images].u_obj;
    if(!mp_obj_is_type(images_obj, &mp_type_list) && !mp_obj_is_type(images_obj, &mp_type_tuple))
        mp_raise_TypeError(MP_ERROR_TEXT("images must be a list"));

    size_t nimgs;
    mp_obj_t *img_items;
    mp_obj_get_array(images_obj, &nimgs, &img_items);
    std::vector<Mat> images(nimgs);
    for(size_t i = 0; i < nimgs; i++)
        images[i] = mp_obj_to_mat(img_items[i]);

    mp_obj_t channels_obj = args[ARG_channels].u_obj;
    size_t nch;
    mp_obj_t *ch_items;
    mp_obj_get_array(channels_obj, &nch, &ch_items);
    std::vector<int> channels;
    for(size_t i = 0; i < nch; i++)
        channels.push_back(mp_obj_get_int(ch_items[i]));

    Mat hist = mp_obj_to_mat(args[ARG_hist].u_obj);
    Mat dst = mp_obj_to_mat(args[ARG_dst].u_obj);

    mp_obj_t ranges_obj = args[ARG_ranges].u_obj;
    size_t nr;
    mp_obj_t *r_items;
    mp_obj_get_array(ranges_obj, &nr, &r_items);
    std::vector<float> ranges;
    for(size_t i = 0; i < nr; i++)
        ranges.push_back((float)mp_obj_get_float(r_items[i]));

    double scale = args[ARG_scale].u_obj == mp_const_none ? 1.0 : mp_obj_get_float(args[ARG_scale].u_obj);

    try {
        calcBackProject(images, channels, hist, dst, ranges, scale);
    } catch(Exception &e) {
        mp_raise_msg(&mp_type_Exception, MP_ERROR_TEXT(e.what()));
    }
    return mat_to_mp_obj(dst);
}
MP_DEFINE_CONST_FUN_OBJ_KW(cv2_imgproc_calcBackProject_obj, 1, cv2_imgproc_calcBackProject_fun);

STATIC mp_obj_t cv2_imgproc_compareHist_fun(size_t n_args, const mp_obj_t *pos_args, mp_map_t *kw_args)
{
    enum { ARG_H1, ARG_H2, ARG_method };
    static const mp_arg_t allowed_args[] = {
        { MP_QSTR_H1,     MP_ARG_REQUIRED | MP_ARG_OBJ, {.u_obj = MP_OBJ_NULL} },
        { MP_QSTR_H2,     MP_ARG_REQUIRED | MP_ARG_OBJ, {.u_obj = MP_OBJ_NULL} },
        { MP_QSTR_method, MP_ARG_REQUIRED | MP_ARG_INT, {.u_int = 0} },
    };
    mp_arg_val_t args[MP_ARRAY_SIZE(allowed_args)];
    mp_arg_parse_all(n_args, pos_args, kw_args, MP_ARRAY_SIZE(allowed_args), allowed_args, args);

    Mat H1 = mp_obj_to_mat(args[ARG_H1].u_obj);
    Mat H2 = mp_obj_to_mat(args[ARG_H2].u_obj);
    int method = args[ARG_method].u_int;

    double result;
    try {
        result = compareHist(H1, H2, method);
    } catch(Exception &e) {
        mp_raise_msg(&mp_type_Exception, MP_ERROR_TEXT(e.what()));
    }
    return mp_obj_new_float(result);
}
MP_DEFINE_CONST_FUN_OBJ_KW(cv2_imgproc_compareHist_obj, 1, cv2_imgproc_compareHist_fun);

// ---- Contour Additions ----

STATIC mp_obj_t cv2_imgproc_polylines_fun(size_t n_args, const mp_obj_t *pos_args, mp_map_t *kw_args)
{
    enum { ARG_img, ARG_pts, ARG_isClosed, ARG_color, ARG_thickness, ARG_lineType, ARG_shift };
    static const mp_arg_t allowed_args[] = {
        { MP_QSTR_img,       MP_ARG_REQUIRED | MP_ARG_OBJ, {.u_obj = MP_OBJ_NULL} },
        { MP_QSTR_pts,       MP_ARG_REQUIRED | MP_ARG_OBJ, {.u_obj = MP_OBJ_NULL} },
        { MP_QSTR_isClosed,  MP_ARG_REQUIRED | MP_ARG_BOOL, {.u_bool = false} },
        { MP_QSTR_color,     MP_ARG_REQUIRED | MP_ARG_OBJ, {.u_obj = MP_OBJ_NULL} },
        { MP_QSTR_thickness, MP_ARG_INT, {.u_int = 1} },
        { MP_QSTR_lineType,  MP_ARG_INT, {.u_int = LINE_8} },
        { MP_QSTR_shift,     MP_ARG_INT, {.u_int = 0} },
    };
    mp_arg_val_t args[MP_ARRAY_SIZE(allowed_args)];
    mp_arg_parse_all(n_args, pos_args, kw_args, MP_ARRAY_SIZE(allowed_args), allowed_args, args);

    Mat img = mp_obj_to_mat(args[ARG_img].u_obj);
    Scalar color = mp_obj_to_scalar(args[ARG_color].u_obj);
    int thickness = args[ARG_thickness].u_int;
    int lineType = args[ARG_lineType].u_int;
    int shift = args[ARG_shift].u_int;
    bool isClosed = args[ARG_isClosed].u_bool;

    mp_obj_t pts_obj = args[ARG_pts].u_obj;
    if(!mp_obj_is_type(pts_obj, &mp_type_list) && !mp_obj_is_type(pts_obj, &mp_type_tuple))
        mp_raise_TypeError(MP_ERROR_TEXT("pts must be a list of contours"));

    size_t ncontours;
    mp_obj_t *contours_items;
    mp_obj_get_array(pts_obj, &ncontours, &contours_items);

    std::vector<std::vector<Point>> pts;
    for(size_t c = 0; c < ncontours; c++)
    {
        ndarray_obj_t *contour = ndarray_from_mp_obj(contours_items[c], 0);
        if(!contour || contour->ndim != 2)
            mp_raise_ValueError(MP_ERROR_TEXT("pts elements must be Nx2 ndarrays"));

        size_t rows = contour->shape[ULAB_MAX_DIMS - 2];
        std::vector<Point> poly(rows);
        for(size_t r = 0; r < rows; r++)
        {
            mp_float_t x = ndarray_get_float_index(contour->array, contour->dtype, r * 2);
            mp_float_t y = ndarray_get_float_index(contour->array, contour->dtype, r * 2 + 1);
            poly[r] = Point((int)x, (int)y);
        }
        pts.push_back(poly);
    }

    try {
        polylines(img, pts, isClosed, color, thickness, lineType, shift);
    } catch(Exception &e) {
        mp_raise_msg(&mp_type_Exception, MP_ERROR_TEXT(e.what()));
    }
    return mat_to_mp_obj(img);
}
MP_DEFINE_CONST_FUN_OBJ_KW(cv2_imgproc_polylines_obj, 1, cv2_imgproc_polylines_fun);

STATIC mp_obj_t cv2_imgproc_getTextSize_fun(size_t n_args, const mp_obj_t *pos_args, mp_map_t *kw_args)
{
    enum { ARG_text, ARG_fontFace, ARG_fontScale, ARG_thickness };
    static const mp_arg_t allowed_args[] = {
        { MP_QSTR_text,      MP_ARG_REQUIRED | MP_ARG_OBJ, {.u_obj = MP_OBJ_NULL} },
        { MP_QSTR_fontFace,  MP_ARG_REQUIRED | MP_ARG_INT, {.u_int = 0} },
        { MP_QSTR_fontScale, MP_ARG_REQUIRED | MP_ARG_OBJ, {.u_obj = MP_OBJ_NULL} },
        { MP_QSTR_thickness, MP_ARG_REQUIRED | MP_ARG_INT, {.u_int = 1} },
    };
    mp_arg_val_t args[MP_ARRAY_SIZE(allowed_args)];
    mp_arg_parse_all(n_args, pos_args, kw_args, MP_ARRAY_SIZE(allowed_args), allowed_args, args);

    const char *text = mp_obj_str_get_str(args[ARG_text].u_obj);
    int fontFace = args[ARG_fontFace].u_int;
    double fontScale = mp_obj_get_float(args[ARG_fontScale].u_obj);
    int thickness = args[ARG_thickness].u_int;

    int baseline;
    Size textSize;
    try {
        textSize = getTextSize(text, fontFace, fontScale, thickness, &baseline);
    } catch(Exception &e) {
        mp_raise_msg(&mp_type_Exception, MP_ERROR_TEXT(e.what()));
    }

    mp_obj_t size_arr[] = { mp_obj_new_int(textSize.width), mp_obj_new_int(textSize.height) };
    mp_obj_t result[2];
    result[0] = mp_obj_new_tuple(2, size_arr);
    result[1] = mp_obj_new_int(baseline);
    return mp_obj_new_tuple(2, result);
}
MP_DEFINE_CONST_FUN_OBJ_KW(cv2_imgproc_getTextSize_obj, 1, cv2_imgproc_getTextSize_fun);

STATIC mp_obj_t cv2_imgproc_pointPolygonTest_fun(size_t n_args, const mp_obj_t *pos_args, mp_map_t *kw_args)
{
    enum { ARG_contour, ARG_pt, ARG_measureDist };
    static const mp_arg_t allowed_args[] = {
        { MP_QSTR_contour,     MP_ARG_REQUIRED | MP_ARG_OBJ, {.u_obj = MP_OBJ_NULL} },
        { MP_QSTR_pt,          MP_ARG_REQUIRED | MP_ARG_OBJ, {.u_obj = MP_OBJ_NULL} },
        { MP_QSTR_measureDist, MP_ARG_BOOL, {.u_bool = false} },
    };
    mp_arg_val_t args[MP_ARRAY_SIZE(allowed_args)];
    mp_arg_parse_all(n_args, pos_args, kw_args, MP_ARRAY_SIZE(allowed_args), allowed_args, args);

    ndarray_obj_t *contour_arr = ndarray_from_mp_obj(args[ARG_contour].u_obj, 0);
    if(!contour_arr || contour_arr->ndim != 2)
        mp_raise_ValueError(MP_ERROR_TEXT("contour must be Nx2 ndarray"));

    size_t rows = contour_arr->shape[ULAB_MAX_DIMS - 2];
    std::vector<Point2f> contour(rows);
    for(size_t r = 0; r < rows; r++)
    {
        mp_float_t x = ndarray_get_float_index(contour_arr->array, contour_arr->dtype, r * 2);
        mp_float_t y = ndarray_get_float_index(contour_arr->array, contour_arr->dtype, r * 2 + 1);
        contour[r] = Point2f(x, y);
    }

    Point2f pt = (Point2f)mp_obj_to_point(args[ARG_pt].u_obj);
    bool measureDist = args[ARG_measureDist].u_bool;

    double result;
    try {
        result = pointPolygonTest(contour, pt, measureDist);
    } catch(Exception &e) {
        mp_raise_msg(&mp_type_Exception, MP_ERROR_TEXT(e.what()));
    }
    return mp_obj_new_float(result);
}
MP_DEFINE_CONST_FUN_OBJ_KW(cv2_imgproc_pointPolygonTest_obj, 1, cv2_imgproc_pointPolygonTest_fun);

STATIC mp_obj_t cv2_imgproc_fitEllipse_fun(size_t n_args, const mp_obj_t *pos_args, mp_map_t *kw_args)
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
        r = fitEllipse(pts);
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
MP_DEFINE_CONST_FUN_OBJ_KW(cv2_imgproc_fitEllipse_obj, 1, cv2_imgproc_fitEllipse_fun);

STATIC mp_obj_t cv2_imgproc_fitLine_fun(size_t n_args, const mp_obj_t *pos_args, mp_map_t *kw_args)
{
    enum { ARG_points, ARG_distType, ARG_param, ARG_reps, ARG_aeps };
    static const mp_arg_t allowed_args[] = {
        { MP_QSTR_points,   MP_ARG_REQUIRED | MP_ARG_OBJ, {.u_obj = MP_OBJ_NULL} },
        { MP_QSTR_distType, MP_ARG_REQUIRED | MP_ARG_INT, {.u_int = cv::DIST_L2} },
        { MP_QSTR_param,    MP_ARG_REQUIRED | MP_ARG_OBJ, {.u_obj = MP_OBJ_NULL} },
        { MP_QSTR_reps,     MP_ARG_REQUIRED | MP_ARG_OBJ, {.u_obj = MP_OBJ_NULL} },
        { MP_QSTR_aeps,     MP_ARG_REQUIRED | MP_ARG_OBJ, {.u_obj = MP_OBJ_NULL} },
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

    int distType = args[ARG_distType].u_int;
    double param = mp_obj_get_float(args[ARG_param].u_obj);
    double reps = mp_obj_get_float(args[ARG_reps].u_obj);
    double aeps = mp_obj_get_float(args[ARG_aeps].u_obj);

    Vec4f line;
    try {
        fitLine(pts, line, distType, param, reps, aeps);
    } catch(Exception &e) {
        mp_raise_msg(&mp_type_Exception, MP_ERROR_TEXT(e.what()));
    }

    mp_obj_t result[4];
    result[0] = mp_obj_new_float(line[0]);
    result[1] = mp_obj_new_float(line[1]);
    result[2] = mp_obj_new_float(line[2]);
    result[3] = mp_obj_new_float(line[3]);
    return mp_obj_new_tuple(4, result);
}
MP_DEFINE_CONST_FUN_OBJ_KW(cv2_imgproc_fitLine_obj, 1, cv2_imgproc_fitLine_fun);

STATIC mp_obj_t cv2_imgproc_convexityDefects_fun(size_t n_args, const mp_obj_t *pos_args, mp_map_t *kw_args)
{
    enum { ARG_contour, ARG_convexhull };
    static const mp_arg_t allowed_args[] = {
        { MP_QSTR_contour,    MP_ARG_REQUIRED | MP_ARG_OBJ, {.u_obj = MP_OBJ_NULL} },
        { MP_QSTR_convexhull, MP_ARG_REQUIRED | MP_ARG_OBJ, {.u_obj = MP_OBJ_NULL} },
    };
    mp_arg_val_t args[MP_ARRAY_SIZE(allowed_args)];
    mp_arg_parse_all(n_args, pos_args, kw_args, MP_ARRAY_SIZE(allowed_args), allowed_args, args);

    ndarray_obj_t *contour_arr = ndarray_from_mp_obj(args[ARG_contour].u_obj, 0);
    if(!contour_arr || contour_arr->ndim != 2)
        mp_raise_ValueError(MP_ERROR_TEXT("contour must be Nx2 ndarray"));

    ndarray_obj_t *hull_arr = ndarray_from_mp_obj(args[ARG_convexhull].u_obj, 0);
    if(!hull_arr || hull_arr->ndim != 1)
        mp_raise_ValueError(MP_ERROR_TEXT("convexhull must be 1D ndarray of indices"));

    size_t rows = contour_arr->shape[ULAB_MAX_DIMS - 2];
    std::vector<Point> contour(rows);
    for(size_t r = 0; r < rows; r++)
    {
        mp_float_t x = ndarray_get_float_index(contour_arr->array, contour_arr->dtype, r * 2);
        mp_float_t y = ndarray_get_float_index(contour_arr->array, contour_arr->dtype, r * 2 + 1);
        contour[r] = Point((int)x, (int)y);
    }

    size_t hull_len = hull_arr->shape[ULAB_MAX_DIMS - 1];
    std::vector<int> hull(hull_len);
    for(size_t i = 0; i < hull_len; i++)
        hull[i] = (int)ndarray_get_float_index(hull_arr->array, hull_arr->dtype, i);

    std::vector<Vec4i> defects;
    try {
        convexityDefects(contour, hull, defects);
    } catch(Exception &e) {
        mp_raise_msg(&mp_type_Exception, MP_ERROR_TEXT(e.what()));
    }

    size_t shape[ULAB_MAX_DIMS];
    for(int i = 0; i < ULAB_MAX_DIMS - 2; i++)
        shape[i] = 0;
    shape[ULAB_MAX_DIMS - 2] = defects.size();
    shape[ULAB_MAX_DIMS - 1] = 4;

    ndarray_obj_t *result_arr = ndarray_new_dense_ndarray(2, shape, NDARRAY_FLOAT);
    mp_float_t *data = (mp_float_t *)result_arr->array;
    for(size_t i = 0; i < defects.size(); i++)
    {
        data[i * 4] = (float)defects[i][0];
        data[i * 4 + 1] = (float)defects[i][1];
        data[i * 4 + 2] = (float)defects[i][2];
        data[i * 4 + 3] = (float)defects[i][3];
    }
    return MP_OBJ_FROM_PTR(result_arr);
}
MP_DEFINE_CONST_FUN_OBJ_KW(cv2_imgproc_convexityDefects_obj, 1, cv2_imgproc_convexityDefects_fun);

STATIC mp_obj_t cv2_imgproc_matchShapes_fun(size_t n_args, const mp_obj_t *pos_args, mp_map_t *kw_args)
{
    enum { ARG_contour1, ARG_contour2, ARG_method, ARG_parameter };
    static const mp_arg_t allowed_args[] = {
        { MP_QSTR_contour1,  MP_ARG_REQUIRED | MP_ARG_OBJ, {.u_obj = MP_OBJ_NULL} },
        { MP_QSTR_contour2,  MP_ARG_REQUIRED | MP_ARG_OBJ, {.u_obj = MP_OBJ_NULL} },
        { MP_QSTR_method,    MP_ARG_REQUIRED | MP_ARG_INT, {.u_int = 0} },
        { MP_QSTR_parameter, MP_ARG_REQUIRED | MP_ARG_OBJ, {.u_obj = MP_OBJ_NULL} },
    };
    mp_arg_val_t args[MP_ARRAY_SIZE(allowed_args)];
    mp_arg_parse_all(n_args, pos_args, kw_args, MP_ARRAY_SIZE(allowed_args), allowed_args, args);

    ndarray_obj_t *c1_arr = ndarray_from_mp_obj(args[ARG_contour1].u_obj, 0);
    if(!c1_arr || c1_arr->ndim != 2)
        mp_raise_ValueError(MP_ERROR_TEXT("contour1 must be Nx2 ndarray"));

    ndarray_obj_t *c2_arr = ndarray_from_mp_obj(args[ARG_contour2].u_obj, 0);
    if(!c2_arr || c2_arr->ndim != 2)
        mp_raise_ValueError(MP_ERROR_TEXT("contour2 must be Nx2 ndarray"));

    size_t rows1 = c1_arr->shape[ULAB_MAX_DIMS - 2];
    std::vector<Point> contour1(rows1);
    for(size_t r = 0; r < rows1; r++)
    {
        mp_float_t x = ndarray_get_float_index(c1_arr->array, c1_arr->dtype, r * 2);
        mp_float_t y = ndarray_get_float_index(c1_arr->array, c1_arr->dtype, r * 2 + 1);
        contour1[r] = Point((int)x, (int)y);
    }

    size_t rows2 = c2_arr->shape[ULAB_MAX_DIMS - 2];
    std::vector<Point> contour2(rows2);
    for(size_t r = 0; r < rows2; r++)
    {
        mp_float_t x = ndarray_get_float_index(c2_arr->array, c2_arr->dtype, r * 2);
        mp_float_t y = ndarray_get_float_index(c2_arr->array, c2_arr->dtype, r * 2 + 1);
        contour2[r] = Point((int)x, (int)y);
    }

    int method = args[ARG_method].u_int;
    double parameter = mp_obj_get_float(args[ARG_parameter].u_obj);

    double result;
    try {
        result = matchShapes(contour1, contour2, method, parameter);
    } catch(Exception &e) {
        mp_raise_msg(&mp_type_Exception, MP_ERROR_TEXT(e.what()));
    }
    return mp_obj_new_float(result);
}
MP_DEFINE_CONST_FUN_OBJ_KW(cv2_imgproc_matchShapes_obj, 1, cv2_imgproc_matchShapes_fun);

STATIC mp_obj_t cv2_imgproc_isContourConvex_fun(size_t n_args, const mp_obj_t *pos_args, mp_map_t *kw_args)
{
    enum { ARG_contour };
    static const mp_arg_t allowed_args[] = {
        { MP_QSTR_contour, MP_ARG_REQUIRED | MP_ARG_OBJ, {.u_obj = MP_OBJ_NULL} },
    };
    mp_arg_val_t args[MP_ARRAY_SIZE(allowed_args)];
    mp_arg_parse_all(n_args, pos_args, kw_args, MP_ARRAY_SIZE(allowed_args), allowed_args, args);

    ndarray_obj_t *contour_arr = ndarray_from_mp_obj(args[ARG_contour].u_obj, 0);
    if(!contour_arr || contour_arr->ndim != 2)
        mp_raise_ValueError(MP_ERROR_TEXT("contour must be Nx2 ndarray"));

    size_t rows = contour_arr->shape[ULAB_MAX_DIMS - 2];
    std::vector<Point> contour(rows);
    for(size_t r = 0; r < rows; r++)
    {
        mp_float_t x = ndarray_get_float_index(contour_arr->array, contour_arr->dtype, r * 2);
        mp_float_t y = ndarray_get_float_index(contour_arr->array, contour_arr->dtype, r * 2 + 1);
        contour[r] = Point((int)x, (int)y);
    }

    bool result;
    try {
        result = isContourConvex(contour);
    } catch(Exception &e) {
        mp_raise_msg(&mp_type_Exception, MP_ERROR_TEXT(e.what()));
    }
    return mp_obj_new_bool(result);
}
MP_DEFINE_CONST_FUN_OBJ_KW(cv2_imgproc_isContourConvex_obj, 1, cv2_imgproc_isContourConvex_fun);

// ---- Segmentation / Flood Fill ----

STATIC mp_obj_t cv2_imgproc_watershed_fun(size_t n_args, const mp_obj_t *pos_args, mp_map_t *kw_args)
{
    enum { ARG_image, ARG_markers };
    static const mp_arg_t allowed_args[] = {
        { MP_QSTR_image,   MP_ARG_REQUIRED | MP_ARG_OBJ, {.u_obj = MP_OBJ_NULL} },
        { MP_QSTR_markers, MP_ARG_REQUIRED | MP_ARG_OBJ, {.u_obj = MP_OBJ_NULL} },
    };
    mp_arg_val_t args[MP_ARRAY_SIZE(allowed_args)];
    mp_arg_parse_all(n_args, pos_args, kw_args, MP_ARRAY_SIZE(allowed_args), allowed_args, args);

    Mat image = mp_obj_to_mat(args[ARG_image].u_obj);
    Mat markers = mp_obj_to_mat(args[ARG_markers].u_obj);
    Mat markers_s32;
    markers.convertTo(markers_s32, CV_32S);

    try {
        watershed(image, markers_s32);
    } catch(Exception &e) {
        mp_raise_msg(&mp_type_Exception, MP_ERROR_TEXT(e.what()));
    }
    // Copy result back in case markers type was changed
    if(markers_s32.data != markers.data)
        markers_s32.convertTo(markers, CV_32F);
    return mat_to_mp_obj(markers);
}
MP_DEFINE_CONST_FUN_OBJ_KW(cv2_imgproc_watershed_obj, 1, cv2_imgproc_watershed_fun);

STATIC mp_obj_t cv2_imgproc_distanceTransform_fun(size_t n_args, const mp_obj_t *pos_args, mp_map_t *kw_args)
{
    enum { ARG_src, ARG_distanceType, ARG_maskSize, ARG_dst, ARG_dstType, ARG_labelType };
    static const mp_arg_t allowed_args[] = {
        { MP_QSTR_src,          MP_ARG_REQUIRED | MP_ARG_OBJ, {.u_obj = MP_OBJ_NULL} },
        { MP_QSTR_distanceType, MP_ARG_REQUIRED | MP_ARG_INT, {.u_int = cv::DIST_L2} },
        { MP_QSTR_maskSize,     MP_ARG_REQUIRED | MP_ARG_INT, {.u_int = 3} },
        { MP_QSTR_dst,          MP_ARG_OBJ, {.u_obj = mp_const_none} },
        { MP_QSTR_dstType,      MP_ARG_INT, {.u_int = CV_32F} },
        { MP_QSTR_labelType,    MP_ARG_INT, {.u_int = cv::DIST_LABEL_CCOMP} },
    };
    mp_arg_val_t args[MP_ARRAY_SIZE(allowed_args)];
    mp_arg_parse_all(n_args, pos_args, kw_args, MP_ARRAY_SIZE(allowed_args), allowed_args, args);

    Mat src = mp_obj_to_mat(args[ARG_src].u_obj);
    int distanceType = args[ARG_distanceType].u_int;
    int maskSize = args[ARG_maskSize].u_int;
    Mat dst = mp_obj_to_mat(args[ARG_dst].u_obj);
    int dstType = args[ARG_dstType].u_int;
    int labelType = args[ARG_labelType].u_int;

    try {
        distanceTransform(src, dst, distanceType, maskSize, dstType);
        (void)labelType;
    } catch(Exception &e) {
        mp_raise_msg(&mp_type_Exception, MP_ERROR_TEXT(e.what()));
    }
    return mat_to_mp_obj(dst);
}
MP_DEFINE_CONST_FUN_OBJ_KW(cv2_imgproc_distanceTransform_obj, 1, cv2_imgproc_distanceTransform_fun);

STATIC mp_obj_t cv2_imgproc_floodFill_fun(size_t n_args, const mp_obj_t *pos_args, mp_map_t *kw_args)
{
    enum { ARG_image, ARG_mask, ARG_seedPoint, ARG_newVal, ARG_loDiff, ARG_upDiff, ARG_flags };
    static const mp_arg_t allowed_args[] = {
        { MP_QSTR_image,     MP_ARG_REQUIRED | MP_ARG_OBJ, {.u_obj = MP_OBJ_NULL} },
        { MP_QSTR_mask,      MP_ARG_OBJ, {.u_obj = mp_const_none} },
        { MP_QSTR_seedPoint, MP_ARG_REQUIRED | MP_ARG_OBJ, {.u_obj = MP_OBJ_NULL} },
        { MP_QSTR_newVal,    MP_ARG_REQUIRED | MP_ARG_OBJ, {.u_obj = MP_OBJ_NULL} },
        { MP_QSTR_loDiff,    MP_ARG_OBJ, {.u_obj = mp_const_none} },
        { MP_QSTR_upDiff,    MP_ARG_OBJ, {.u_obj = mp_const_none} },
        { MP_QSTR_flags,     MP_ARG_INT, {.u_int = 4} },
    };
    mp_arg_val_t args[MP_ARRAY_SIZE(allowed_args)];
    mp_arg_parse_all(n_args, pos_args, kw_args, MP_ARRAY_SIZE(allowed_args), allowed_args, args);

    Mat image = mp_obj_to_mat(args[ARG_image].u_obj);
    Mat mask = mp_obj_to_mat(args[ARG_mask].u_obj);
    Point seedPoint = mp_obj_to_point(args[ARG_seedPoint].u_obj);
    Scalar newVal = mp_obj_to_scalar(args[ARG_newVal].u_obj);
    Scalar loDiff = args[ARG_loDiff].u_obj == mp_const_none ? Scalar() : mp_obj_to_scalar(args[ARG_loDiff].u_obj);
    Scalar upDiff = args[ARG_upDiff].u_obj == mp_const_none ? Scalar() : mp_obj_to_scalar(args[ARG_upDiff].u_obj);
    int flags = args[ARG_flags].u_int;

    Rect rect;
    try {
        floodFill(image, mask.empty() ? noArray() : (InputOutputArray)mask, seedPoint, newVal, &rect, loDiff, upDiff, flags);
    } catch(Exception &e) {
        mp_raise_msg(&mp_type_Exception, MP_ERROR_TEXT(e.what()));
    }

    mp_obj_t r_arr[] = { mp_obj_new_int(rect.x), mp_obj_new_int(rect.y), mp_obj_new_int(rect.width), mp_obj_new_int(rect.height) };
    mp_obj_t result[2];
    result[0] = mat_to_mp_obj(image);
    result[1] = mp_obj_new_tuple(4, r_arr);
    return mp_obj_new_tuple(2, result);
}
MP_DEFINE_CONST_FUN_OBJ_KW(cv2_imgproc_floodFill_obj, 1, cv2_imgproc_floodFill_fun);

STATIC mp_obj_t cv2_imgproc_grabCut_fun(size_t n_args, const mp_obj_t *pos_args, mp_map_t *kw_args)
{
    enum { ARG_img, ARG_mask, ARG_rect, ARG_bgdModel, ARG_fgdModel, ARG_iterCount, ARG_mode };
    static const mp_arg_t allowed_args[] = {
        { MP_QSTR_img,        MP_ARG_REQUIRED | MP_ARG_OBJ, {.u_obj = MP_OBJ_NULL} },
        { MP_QSTR_mask,       MP_ARG_REQUIRED | MP_ARG_OBJ, {.u_obj = MP_OBJ_NULL} },
        { MP_QSTR_rect,       MP_ARG_REQUIRED | MP_ARG_OBJ, {.u_obj = MP_OBJ_NULL} },
        { MP_QSTR_bgdModel,   MP_ARG_REQUIRED | MP_ARG_OBJ, {.u_obj = MP_OBJ_NULL} },
        { MP_QSTR_fgdModel,   MP_ARG_REQUIRED | MP_ARG_OBJ, {.u_obj = MP_OBJ_NULL} },
        { MP_QSTR_iterCount,  MP_ARG_REQUIRED | MP_ARG_INT, {.u_int = 5} },
        { MP_QSTR_mode,       MP_ARG_INT, {.u_int = cv::GC_INIT_WITH_RECT} },
    };
    mp_arg_val_t args[MP_ARRAY_SIZE(allowed_args)];
    mp_arg_parse_all(n_args, pos_args, kw_args, MP_ARRAY_SIZE(allowed_args), allowed_args, args);

    Mat img = mp_obj_to_mat(args[ARG_img].u_obj);
    Mat mask = mp_obj_to_mat(args[ARG_mask].u_obj);
    Mat bgdModel = mp_obj_to_mat(args[ARG_bgdModel].u_obj);
    Mat fgdModel = mp_obj_to_mat(args[ARG_fgdModel].u_obj);
    Mat bgd64, fgd64;
    bgdModel.convertTo(bgd64, CV_64F);
    fgdModel.convertTo(fgd64, CV_64F);
    int iterCount = args[ARG_iterCount].u_int;
    int mode = args[ARG_mode].u_int;

    mp_obj_t rect_obj = args[ARG_rect].u_obj;
    size_t rlen;
    mp_obj_t *ritems;
    mp_obj_get_array(rect_obj, &rlen, &ritems);
    if(rlen < 4)
        mp_raise_ValueError(MP_ERROR_TEXT("rect must be (x, y, w, h)"));

    Rect rect(mp_obj_get_int(ritems[0]), mp_obj_get_int(ritems[1]),
              mp_obj_get_int(ritems[2]), mp_obj_get_int(ritems[3]));

    try {
        grabCut(img, mask, rect, bgd64, fgd64, iterCount, mode);
    } catch(Exception &e) {
        mp_raise_msg(&mp_type_Exception, MP_ERROR_TEXT(e.what()));
    }

    mp_obj_t result[3];
    result[0] = mat_to_mp_obj(mask);
    result[1] = mat_to_mp_obj(bgd64);
    result[2] = mat_to_mp_obj(fgd64);
    return mp_obj_new_tuple(3, result);
}
MP_DEFINE_CONST_FUN_OBJ_KW(cv2_imgproc_grabCut_obj, 1, cv2_imgproc_grabCut_fun);

STATIC mp_obj_t cv2_imgproc_connectedComponents_fun(size_t n_args, const mp_obj_t *pos_args, mp_map_t *kw_args)
{
    enum { ARG_image, ARG_labels, ARG_connectivity, ARG_ltype };
    static const mp_arg_t allowed_args[] = {
        { MP_QSTR_image,        MP_ARG_REQUIRED | MP_ARG_OBJ, {.u_obj = MP_OBJ_NULL} },
        { MP_QSTR_labels,       MP_ARG_OBJ, {.u_obj = mp_const_none} },
        { MP_QSTR_connectivity, MP_ARG_INT, {.u_int = 8} },
        { MP_QSTR_ltype,        MP_ARG_INT, {.u_int = CV_32S} },
    };
    mp_arg_val_t args[MP_ARRAY_SIZE(allowed_args)];
    mp_arg_parse_all(n_args, pos_args, kw_args, MP_ARRAY_SIZE(allowed_args), allowed_args, args);

    Mat image = mp_obj_to_mat(args[ARG_image].u_obj);
    Mat labels = mp_obj_to_mat(args[ARG_labels].u_obj);
    int connectivity = args[ARG_connectivity].u_int;
    int ltype = args[ARG_ltype].u_int;

    int nlabels;
    try {
        nlabels = connectedComponents(image, labels, connectivity, ltype);
    } catch(Exception &e) {
        mp_raise_msg(&mp_type_Exception, MP_ERROR_TEXT(e.what()));
    }

    mp_obj_t result[2];
    result[0] = mp_obj_new_int(nlabels);
    result[1] = mat_to_mp_obj(labels);
    return mp_obj_new_tuple(2, result);
}
MP_DEFINE_CONST_FUN_OBJ_KW(cv2_imgproc_connectedComponents_obj, 1, cv2_imgproc_connectedComponents_fun);
