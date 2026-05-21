#ifndef CV_UPY_IMGPROC_H
#define CV_UPY_IMGPROC_H

#ifdef __cplusplus
extern "C" {
#endif
#include "py/obj.h"
#ifdef __cplusplus
}
#endif

extern const mp_obj_fun_builtin_var_t cv2_imgproc_Canny_obj;
extern const mp_obj_fun_builtin_var_t cv2_imgproc_GaussianBlur_obj;
extern const mp_obj_fun_builtin_var_t cv2_imgproc_blur_obj;
extern const mp_obj_fun_builtin_var_t cv2_imgproc_medianBlur_obj;
extern const mp_obj_fun_builtin_var_t cv2_imgproc_bilateralFilter_obj;
extern const mp_obj_fun_builtin_var_t cv2_imgproc_boxFilter_obj;
extern const mp_obj_fun_builtin_var_t cv2_imgproc_filter2D_obj;
extern const mp_obj_fun_builtin_var_t cv2_imgproc_Sobel_obj;
extern const mp_obj_fun_builtin_var_t cv2_imgproc_Scharr_obj;
extern const mp_obj_fun_builtin_var_t cv2_imgproc_Laplacian_obj;
extern const mp_obj_fun_builtin_var_t cv2_imgproc_erode_obj;
extern const mp_obj_fun_builtin_var_t cv2_imgproc_dilate_obj;
extern const mp_obj_fun_builtin_var_t cv2_imgproc_morphologyEx_obj;
extern const mp_obj_fun_builtin_var_t cv2_imgproc_getStructuringElement_obj;
extern const mp_obj_fun_builtin_var_t cv2_imgproc_threshold_obj;
extern const mp_obj_fun_builtin_var_t cv2_imgproc_adaptiveThreshold_obj;
extern const mp_obj_fun_builtin_var_t cv2_imgproc_cvtColor_obj;
extern const mp_obj_fun_builtin_var_t cv2_imgproc_resize_obj;
extern const mp_obj_fun_builtin_var_t cv2_imgproc_warpAffine_obj;
extern const mp_obj_fun_builtin_var_t cv2_imgproc_warpPerspective_obj;
extern const mp_obj_fun_builtin_var_t cv2_imgproc_getRotationMatrix2D_obj;
extern const mp_obj_fun_builtin_var_t cv2_imgproc_line_obj;
extern const mp_obj_fun_builtin_var_t cv2_imgproc_rectangle_obj;
extern const mp_obj_fun_builtin_var_t cv2_imgproc_circle_obj;
extern const mp_obj_fun_builtin_var_t cv2_imgproc_ellipse_obj;
extern const mp_obj_fun_builtin_var_t cv2_imgproc_putText_obj;
extern const mp_obj_fun_builtin_var_t cv2_imgproc_arrowedLine_obj;
extern const mp_obj_fun_builtin_var_t cv2_imgproc_drawMarker_obj;
extern const mp_obj_fun_builtin_var_t cv2_imgproc_fillPoly_obj;
extern const mp_obj_fun_builtin_var_t cv2_imgproc_findContours_obj;
extern const mp_obj_fun_builtin_var_t cv2_imgproc_drawContours_obj;
extern const mp_obj_fun_builtin_var_t cv2_imgproc_contourArea_obj;
extern const mp_obj_fun_builtin_var_t cv2_imgproc_arcLength_obj;
extern const mp_obj_fun_builtin_var_t cv2_imgproc_boundingRect_obj;
extern const mp_obj_fun_builtin_var_t cv2_imgproc_approxPolyDP_obj;
extern const mp_obj_fun_builtin_var_t cv2_imgproc_convexHull_obj;
extern const mp_obj_fun_builtin_var_t cv2_imgproc_minAreaRect_obj;
extern const mp_obj_fun_builtin_var_t cv2_imgproc_minEnclosingCircle_obj;
extern const mp_obj_fun_builtin_var_t cv2_imgproc_matchTemplate_obj;
extern const mp_obj_fun_builtin_var_t cv2_imgproc_moments_obj;
extern const mp_obj_fun_builtin_var_t cv2_imgproc_HoughLines_obj;
extern const mp_obj_fun_builtin_var_t cv2_imgproc_HoughLinesP_obj;
extern const mp_obj_fun_builtin_var_t cv2_imgproc_HoughCircles_obj;
extern const mp_obj_fun_builtin_var_t cv2_imgproc_equalizeHist_obj;

// Constants
#define OPENCV_IMGPROC_CONSTANTS \
    { MP_ROM_QSTR(MP_QSTR_MORPH_ERODE),     MP_ROM_INT(0) }, \
    { MP_ROM_QSTR(MP_QSTR_MORPH_DILATE),    MP_ROM_INT(1) }, \
    { MP_ROM_QSTR(MP_QSTR_MORPH_OPEN),      MP_ROM_INT(2) }, \
    { MP_ROM_QSTR(MP_QSTR_MORPH_CLOSE),     MP_ROM_INT(3) }, \
    { MP_ROM_QSTR(MP_QSTR_MORPH_GRADIENT),  MP_ROM_INT(4) }, \
    { MP_ROM_QSTR(MP_QSTR_MORPH_TOPHAT),    MP_ROM_INT(5) }, \
    { MP_ROM_QSTR(MP_QSTR_MORPH_BLACKHAT),  MP_ROM_INT(6) }, \
    { MP_ROM_QSTR(MP_QSTR_MORPH_HITMISS),   MP_ROM_INT(7) }, \
    { MP_ROM_QSTR(MP_QSTR_MORPH_RECT),      MP_ROM_INT(0) }, \
    { MP_ROM_QSTR(MP_QSTR_MORPH_CROSS),     MP_ROM_INT(1) }, \
    { MP_ROM_QSTR(MP_QSTR_MORPH_ELLIPSE),   MP_ROM_INT(2) }, \
    { MP_ROM_QSTR(MP_QSTR_THRESH_BINARY),       MP_ROM_INT(0) }, \
    { MP_ROM_QSTR(MP_QSTR_THRESH_BINARY_INV),   MP_ROM_INT(1) }, \
    { MP_ROM_QSTR(MP_QSTR_THRESH_TRUNC),        MP_ROM_INT(2) }, \
    { MP_ROM_QSTR(MP_QSTR_THRESH_TOZERO),       MP_ROM_INT(3) }, \
    { MP_ROM_QSTR(MP_QSTR_THRESH_TOZERO_INV),   MP_ROM_INT(4) }, \
    { MP_ROM_QSTR(MP_QSTR_THRESH_OTSU),         MP_ROM_INT(8) }, \
    { MP_ROM_QSTR(MP_QSTR_THRESH_TRIANGLE),     MP_ROM_INT(16) }, \
    { MP_ROM_QSTR(MP_QSTR_ADAPTIVE_THRESH_MEAN_C),     MP_ROM_INT(0) }, \
    { MP_ROM_QSTR(MP_QSTR_ADAPTIVE_THRESH_GAUSSIAN_C), MP_ROM_INT(1) }, \
    { MP_ROM_QSTR(MP_QSTR_BORDER_CONSTANT),    MP_ROM_INT(0) }, \
    { MP_ROM_QSTR(MP_QSTR_BORDER_REPLICATE),   MP_ROM_INT(1) }, \
    { MP_ROM_QSTR(MP_QSTR_BORDER_REFLECT),     MP_ROM_INT(2) }, \
    { MP_ROM_QSTR(MP_QSTR_BORDER_WRAP),        MP_ROM_INT(3) }, \
    { MP_ROM_QSTR(MP_QSTR_BORDER_REFLECT_101), MP_ROM_INT(4) }, \
    { MP_ROM_QSTR(MP_QSTR_BORDER_DEFAULT),     MP_ROM_INT(4) }, \
    { MP_ROM_QSTR(MP_QSTR_BORDER_TRANSPARENT), MP_ROM_INT(5) }, \
    { MP_ROM_QSTR(MP_QSTR_BORDER_ISOLATED),    MP_ROM_INT(16) }, \
    { MP_ROM_QSTR(MP_QSTR_FONT_HERSHEY_SIMPLEX),        MP_ROM_INT(0) }, \
    { MP_ROM_QSTR(MP_QSTR_FONT_HERSHEY_PLAIN),          MP_ROM_INT(1) }, \
    { MP_ROM_QSTR(MP_QSTR_FONT_HERSHEY_DUPLEX),         MP_ROM_INT(2) }, \
    { MP_ROM_QSTR(MP_QSTR_FONT_HERSHEY_COMPLEX),        MP_ROM_INT(3) }, \
    { MP_ROM_QSTR(MP_QSTR_FONT_HERSHEY_TRIPLEX),        MP_ROM_INT(4) }, \
    { MP_ROM_QSTR(MP_QSTR_FONT_HERSHEY_COMPLEX_SMALL),  MP_ROM_INT(5) }, \
    { MP_ROM_QSTR(MP_QSTR_FONT_HERSHEY_SCRIPT_SIMPLEX), MP_ROM_INT(6) }, \
    { MP_ROM_QSTR(MP_QSTR_FONT_HERSHEY_SCRIPT_COMPLEX), MP_ROM_INT(7) }, \
    { MP_ROM_QSTR(MP_QSTR_FONT_ITALIC),                 MP_ROM_INT(16) }, \
    { MP_ROM_QSTR(MP_QSTR_LINE_4),  MP_ROM_INT(4) }, \
    { MP_ROM_QSTR(MP_QSTR_LINE_8),  MP_ROM_INT(8) }, \
    { MP_ROM_QSTR(MP_QSTR_LINE_AA), MP_ROM_INT(16) }, \
    { MP_ROM_QSTR(MP_QSTR_FILLED),  MP_ROM_INT(-1) }, \
    { MP_ROM_QSTR(MP_QSTR_RETR_EXTERNAL),  MP_ROM_INT(0) }, \
    { MP_ROM_QSTR(MP_QSTR_RETR_LIST),      MP_ROM_INT(1) }, \
    { MP_ROM_QSTR(MP_QSTR_RETR_CCOMP),     MP_ROM_INT(2) }, \
    { MP_ROM_QSTR(MP_QSTR_RETR_TREE),      MP_ROM_INT(3) }, \
    { MP_ROM_QSTR(MP_QSTR_RETR_FLOODFILL), MP_ROM_INT(4) }, \
    { MP_ROM_QSTR(MP_QSTR_CHAIN_APPROX_NONE),         MP_ROM_INT(1) }, \
    { MP_ROM_QSTR(MP_QSTR_CHAIN_APPROX_SIMPLE),       MP_ROM_INT(2) }, \
    { MP_ROM_QSTR(MP_QSTR_CHAIN_APPROX_TC89_L1),      MP_ROM_INT(3) }, \
    { MP_ROM_QSTR(MP_QSTR_CHAIN_APPROX_TC89_KCOS),    MP_ROM_INT(4) }, \
    { MP_ROM_QSTR(MP_QSTR_CONTOURS_MATCH_I1),  MP_ROM_INT(1) }, \
    { MP_ROM_QSTR(MP_QSTR_CONTOURS_MATCH_I2),  MP_ROM_INT(2) }, \
    { MP_ROM_QSTR(MP_QSTR_CONTOURS_MATCH_I3),  MP_ROM_INT(3) }, \
    { MP_ROM_QSTR(MP_QSTR_CONNECTED_4),  MP_ROM_INT(4) }, \
    { MP_ROM_QSTR(MP_QSTR_CONNECTED_8),  MP_ROM_INT(8) }, \
    { MP_ROM_QSTR(MP_QSTR_CCL_DEFAULT),  MP_ROM_INT(-1) }, \
    { MP_ROM_QSTR(MP_QSTR_CCL_WU),       MP_ROM_INT(0) }, \
    { MP_ROM_QSTR(MP_QSTR_CCL_GRANA),    MP_ROM_INT(1) }, \
    { MP_ROM_QSTR(MP_QSTR_CCL_BOLELLI),  MP_ROM_INT(2) }, \
    { MP_ROM_QSTR(MP_QSTR_CCL_SAUF),     MP_ROM_INT(3) }, \
    { MP_ROM_QSTR(MP_QSTR_CCL_BBDT),     MP_ROM_INT(4) }, \
    { MP_ROM_QSTR(MP_QSTR_CCL_SPAGHETTI),MP_ROM_INT(5) }, \
    { MP_ROM_QSTR(MP_QSTR_DIST_L1),      MP_ROM_INT(1) }, \
    { MP_ROM_QSTR(MP_QSTR_DIST_L2),      MP_ROM_INT(2) }, \
    { MP_ROM_QSTR(MP_QSTR_DIST_C),       MP_ROM_INT(3) }, \
    { MP_ROM_QSTR(MP_QSTR_DIST_L12),     MP_ROM_INT(4) }, \
    { MP_ROM_QSTR(MP_QSTR_DIST_FAIR),    MP_ROM_INT(5) }, \
    { MP_ROM_QSTR(MP_QSTR_DIST_WELSCH),  MP_ROM_INT(6) }, \
    { MP_ROM_QSTR(MP_QSTR_DIST_HUBER),   MP_ROM_INT(7) }, \
    { MP_ROM_QSTR(MP_QSTR_TM_SQDIFF),        MP_ROM_INT(0) }, \
    { MP_ROM_QSTR(MP_QSTR_TM_SQDIFF_NORMED), MP_ROM_INT(1) }, \
    { MP_ROM_QSTR(MP_QSTR_TM_CCORR),         MP_ROM_INT(2) }, \
    { MP_ROM_QSTR(MP_QSTR_TM_CCORR_NORMED),  MP_ROM_INT(3) }, \
    { MP_ROM_QSTR(MP_QSTR_TM_CCOEFF),        MP_ROM_INT(4) }, \
    { MP_ROM_QSTR(MP_QSTR_TM_CCOEFF_NORMED), MP_ROM_INT(5) }, \
    { MP_ROM_QSTR(MP_QSTR_MARKER_CROSS),  MP_ROM_INT(0) }, \
    { MP_ROM_QSTR(MP_QSTR_MARKER_TILTED_CROSS), MP_ROM_INT(1) }, \
    { MP_ROM_QSTR(MP_QSTR_MARKER_STAR),   MP_ROM_INT(2) }, \
    { MP_ROM_QSTR(MP_QSTR_MARKER_DIAMOND),MP_ROM_INT(3) }, \
    { MP_ROM_QSTR(MP_QSTR_MARKER_SQUARE), MP_ROM_INT(4) }, \
    { MP_ROM_QSTR(MP_QSTR_MARKER_TRIANGLE_UP), MP_ROM_INT(5) }, \
    { MP_ROM_QSTR(MP_QSTR_MARKER_TRIANGLE_DOWN), MP_ROM_INT(6) },

// Color conversion constants
#define OPENCV_COLOR_CONVERSION_CONSTANTS \
    { MP_ROM_QSTR(MP_QSTR_COLOR_BGR2GRAY),  MP_ROM_INT(6) }, \
    { MP_ROM_QSTR(MP_QSTR_COLOR_RGB2GRAY),  MP_ROM_INT(7) }, \
    { MP_ROM_QSTR(MP_QSTR_COLOR_GRAY2BGR),  MP_ROM_INT(8) }, \
    { MP_ROM_QSTR(MP_QSTR_COLOR_GRAY2RGB),  MP_ROM_INT(8) }, \
    { MP_ROM_QSTR(MP_QSTR_COLOR_BGR2RGB),   MP_ROM_INT(4) }, \
    { MP_ROM_QSTR(MP_QSTR_COLOR_RGB2BGR),   MP_ROM_INT(4) }, \
    { MP_ROM_QSTR(MP_QSTR_COLOR_BGR2HSV),   MP_ROM_INT(40) }, \
    { MP_ROM_QSTR(MP_QSTR_COLOR_HSV2BGR),   MP_ROM_INT(54) }, \
    { MP_ROM_QSTR(MP_QSTR_COLOR_RGB2HSV),   MP_ROM_INT(41) }, \
    { MP_ROM_QSTR(MP_QSTR_COLOR_HSV2RGB),   MP_ROM_INT(55) }, \
    { MP_ROM_QSTR(MP_QSTR_COLOR_BGR2YUV),   MP_ROM_INT(82) }, \
    { MP_ROM_QSTR(MP_QSTR_COLOR_YUV2BGR),   MP_ROM_INT(84) }, \
    { MP_ROM_QSTR(MP_QSTR_COLOR_BGR2YCR_CB), MP_ROM_INT(36) }, \
    { MP_ROM_QSTR(MP_QSTR_COLOR_YCR_CB2BGR), MP_ROM_INT(38) }, \
    { MP_ROM_QSTR(MP_QSTR_COLOR_BGR2LAB),   MP_ROM_INT(44) }, \
    { MP_ROM_QSTR(MP_QSTR_COLOR_LAB2BGR),   MP_ROM_INT(56) }, \
    { MP_ROM_QSTR(MP_QSTR_COLOR_BGR2LUV),   MP_ROM_INT(50) }, \
    { MP_ROM_QSTR(MP_QSTR_COLOR_LUV2BGR),   MP_ROM_INT(58) }, \
    { MP_ROM_QSTR(MP_QSTR_COLOR_BGR2XYZ),   MP_ROM_INT(32) }, \
    { MP_ROM_QSTR(MP_QSTR_COLOR_XYZ2BGR),   MP_ROM_INT(34) }, \
    { MP_ROM_QSTR(MP_QSTR_COLOR_BGR2HLS),   MP_ROM_INT(52) }, \
    { MP_ROM_QSTR(MP_QSTR_COLOR_HLS2BGR),   MP_ROM_INT(60) }, \
    { MP_ROM_QSTR(MP_QSTR_COLOR_BGRA2BGR),  MP_ROM_INT(45) }, \
    { MP_ROM_QSTR(MP_QSTR_COLOR_BGR2BGRA),  MP_ROM_INT(43) },

// Rotate flag constants
#define OPENCV_CORE_CONSTANTS \
    { MP_ROM_QSTR(MP_QSTR_ROTATE_90_CLOCKWISE),         MP_ROM_INT(0) }, \
    { MP_ROM_QSTR(MP_QSTR_ROTATE_180),                  MP_ROM_INT(1) }, \
    { MP_ROM_QSTR(MP_QSTR_ROTATE_90_COUNTERCLOCKWISE),  MP_ROM_INT(2) },

#define OPENCV_IMGPROC_GLOBALS \
    { MP_ROM_QSTR(MP_QSTR_Canny),              MP_ROM_PTR(&cv2_imgproc_Canny_obj) }, \
    { MP_ROM_QSTR(MP_QSTR_GaussianBlur),       MP_ROM_PTR(&cv2_imgproc_GaussianBlur_obj) }, \
    { MP_ROM_QSTR(MP_QSTR_blur),               MP_ROM_PTR(&cv2_imgproc_blur_obj) }, \
    { MP_ROM_QSTR(MP_QSTR_medianBlur),         MP_ROM_PTR(&cv2_imgproc_medianBlur_obj) }, \
    { MP_ROM_QSTR(MP_QSTR_bilateralFilter),    MP_ROM_PTR(&cv2_imgproc_bilateralFilter_obj) }, \
    { MP_ROM_QSTR(MP_QSTR_boxFilter),          MP_ROM_PTR(&cv2_imgproc_boxFilter_obj) }, \
    { MP_ROM_QSTR(MP_QSTR_filter2D),           MP_ROM_PTR(&cv2_imgproc_filter2D_obj) }, \
    { MP_ROM_QSTR(MP_QSTR_Sobel),              MP_ROM_PTR(&cv2_imgproc_Sobel_obj) }, \
    { MP_ROM_QSTR(MP_QSTR_Scharr),             MP_ROM_PTR(&cv2_imgproc_Scharr_obj) }, \
    { MP_ROM_QSTR(MP_QSTR_Laplacian),          MP_ROM_PTR(&cv2_imgproc_Laplacian_obj) }, \
    { MP_ROM_QSTR(MP_QSTR_erode),              MP_ROM_PTR(&cv2_imgproc_erode_obj) }, \
    { MP_ROM_QSTR(MP_QSTR_dilate),             MP_ROM_PTR(&cv2_imgproc_dilate_obj) }, \
    { MP_ROM_QSTR(MP_QSTR_morphologyEx),       MP_ROM_PTR(&cv2_imgproc_morphologyEx_obj) }, \
    { MP_ROM_QSTR(MP_QSTR_getStructuringElement), MP_ROM_PTR(&cv2_imgproc_getStructuringElement_obj) }, \
    { MP_ROM_QSTR(MP_QSTR_threshold),          MP_ROM_PTR(&cv2_imgproc_threshold_obj) }, \
    { MP_ROM_QSTR(MP_QSTR_adaptiveThreshold),  MP_ROM_PTR(&cv2_imgproc_adaptiveThreshold_obj) }, \
    { MP_ROM_QSTR(MP_QSTR_cvtColor),           MP_ROM_PTR(&cv2_imgproc_cvtColor_obj) }, \
    { MP_ROM_QSTR(MP_QSTR_resize),             MP_ROM_PTR(&cv2_imgproc_resize_obj) }, \
    { MP_ROM_QSTR(MP_QSTR_warpAffine),         MP_ROM_PTR(&cv2_imgproc_warpAffine_obj) }, \
    { MP_ROM_QSTR(MP_QSTR_warpPerspective),    MP_ROM_PTR(&cv2_imgproc_warpPerspective_obj) }, \
    { MP_ROM_QSTR(MP_QSTR_getRotationMatrix2D),MP_ROM_PTR(&cv2_imgproc_getRotationMatrix2D_obj) }, \
    { MP_ROM_QSTR(MP_QSTR_line),               MP_ROM_PTR(&cv2_imgproc_line_obj) }, \
    { MP_ROM_QSTR(MP_QSTR_rectangle),          MP_ROM_PTR(&cv2_imgproc_rectangle_obj) }, \
    { MP_ROM_QSTR(MP_QSTR_circle),             MP_ROM_PTR(&cv2_imgproc_circle_obj) }, \
    { MP_ROM_QSTR(MP_QSTR_ellipse),            MP_ROM_PTR(&cv2_imgproc_ellipse_obj) }, \
    { MP_ROM_QSTR(MP_QSTR_putText),            MP_ROM_PTR(&cv2_imgproc_putText_obj) }, \
    { MP_ROM_QSTR(MP_QSTR_arrowedLine),        MP_ROM_PTR(&cv2_imgproc_arrowedLine_obj) }, \
    { MP_ROM_QSTR(MP_QSTR_drawMarker),         MP_ROM_PTR(&cv2_imgproc_drawMarker_obj) }, \
    { MP_ROM_QSTR(MP_QSTR_fillPoly),           MP_ROM_PTR(&cv2_imgproc_fillPoly_obj) }, \
    { MP_ROM_QSTR(MP_QSTR_findContours),       MP_ROM_PTR(&cv2_imgproc_findContours_obj) }, \
    { MP_ROM_QSTR(MP_QSTR_drawContours),       MP_ROM_PTR(&cv2_imgproc_drawContours_obj) }, \
    { MP_ROM_QSTR(MP_QSTR_contourArea),        MP_ROM_PTR(&cv2_imgproc_contourArea_obj) }, \
    { MP_ROM_QSTR(MP_QSTR_arcLength),          MP_ROM_PTR(&cv2_imgproc_arcLength_obj) }, \
    { MP_ROM_QSTR(MP_QSTR_boundingRect),       MP_ROM_PTR(&cv2_imgproc_boundingRect_obj) }, \
    { MP_ROM_QSTR(MP_QSTR_approxPolyDP),       MP_ROM_PTR(&cv2_imgproc_approxPolyDP_obj) }, \
    { MP_ROM_QSTR(MP_QSTR_convexHull),         MP_ROM_PTR(&cv2_imgproc_convexHull_obj) }, \
    { MP_ROM_QSTR(MP_QSTR_minAreaRect),        MP_ROM_PTR(&cv2_imgproc_minAreaRect_obj) }, \
    { MP_ROM_QSTR(MP_QSTR_minEnclosingCircle), MP_ROM_PTR(&cv2_imgproc_minEnclosingCircle_obj) }, \
    { MP_ROM_QSTR(MP_QSTR_matchTemplate),      MP_ROM_PTR(&cv2_imgproc_matchTemplate_obj) }, \
    { MP_ROM_QSTR(MP_QSTR_moments),            MP_ROM_PTR(&cv2_imgproc_moments_obj) }, \
    { MP_ROM_QSTR(MP_QSTR_HoughLines),         MP_ROM_PTR(&cv2_imgproc_HoughLines_obj) }, \
    { MP_ROM_QSTR(MP_QSTR_HoughLinesP),        MP_ROM_PTR(&cv2_imgproc_HoughLinesP_obj) }, \
    { MP_ROM_QSTR(MP_QSTR_HoughCircles),       MP_ROM_PTR(&cv2_imgproc_HoughCircles_obj) }, \
    { MP_ROM_QSTR(MP_QSTR_equalizeHist),       MP_ROM_PTR(&cv2_imgproc_equalizeHist_obj) }, \
    { MP_ROM_QSTR(MP_QSTR_CV_8U),              MP_ROM_INT(0) }, \

#endif // CV_UPY_IMGPROC_H
