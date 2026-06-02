#ifndef CV_UPY_IMGCODECS_H
#define CV_UPY_IMGCODECS_H

#ifdef __cplusplus
extern "C" {
#endif
#include "py/obj.h"
#ifdef __cplusplus
}
#endif

extern const mp_obj_fun_builtin_var_t cv2_imgcodecs_imread_obj;
extern const mp_obj_fun_builtin_var_t cv2_imgcodecs_imwrite_obj;
extern const mp_obj_fun_builtin_var_t cv2_imgcodecs_imdecode_obj;
extern const mp_obj_fun_builtin_var_t cv2_imgcodecs_imencode_obj;

// imread flags
#define OPENCV_IMGCODECS_CONSTANTS \
    { MP_ROM_QSTR(MP_QSTR_IMREAD_UNCHANGED),  MP_ROM_INT(-1) }, \
    { MP_ROM_QSTR(MP_QSTR_IMREAD_GRAYSCALE),  MP_ROM_INT(0) }, \
    { MP_ROM_QSTR(MP_QSTR_IMREAD_COLOR),      MP_ROM_INT(1) }, \
    { MP_ROM_QSTR(MP_QSTR_IMREAD_ANYDEPTH),   MP_ROM_INT(2) }, \
    { MP_ROM_QSTR(MP_QSTR_IMREAD_ANYCOLOR),   MP_ROM_INT(4) }, \
    { MP_ROM_QSTR(MP_QSTR_IMREAD_IGNORE_ORIENTATION), MP_ROM_INT(128) },

#define OPENCV_IMGCODECS_GLOBALS \
    { MP_ROM_QSTR(MP_QSTR_imread),   MP_ROM_PTR(&cv2_imgcodecs_imread_obj) }, \
    { MP_ROM_QSTR(MP_QSTR_imwrite),  MP_ROM_PTR(&cv2_imgcodecs_imwrite_obj) }, \
    { MP_ROM_QSTR(MP_QSTR_imdecode), MP_ROM_PTR(&cv2_imgcodecs_imdecode_obj) }, \
    { MP_ROM_QSTR(MP_QSTR_imencode), MP_ROM_PTR(&cv2_imgcodecs_imencode_obj) },

#endif // CV_UPY_IMGCODECS_H
