#ifndef CV_UPY_CORE_H
#define CV_UPY_CORE_H

#ifdef __cplusplus
extern "C" {
#endif
#include "py/obj.h"
#ifdef __cplusplus
}
#endif

extern const mp_obj_fun_builtin_var_t cv2_core_inRange_obj;
extern const mp_obj_fun_builtin_var_t cv2_core_convertScaleAbs_obj;
extern const mp_obj_fun_builtin_var_t cv2_core_minMaxLoc_obj;
extern const mp_obj_fun_builtin_var_t cv2_core_LUT_obj;
extern const mp_obj_fun_builtin_var_t cv2_core_countNonZero_obj;
extern const mp_obj_fun_builtin_var_t cv2_core_flip_obj;
extern const mp_obj_fun_builtin_var_t cv2_core_rotate_obj;
extern const mp_obj_fun_builtin_var_t cv2_core_transpose_obj;

#define OPENCV_CORE_GLOBALS \
    { MP_ROM_QSTR(MP_QSTR_inRange),          MP_ROM_PTR(&cv2_core_inRange_obj) }, \
    { MP_ROM_QSTR(MP_QSTR_convertScaleAbs),  MP_ROM_PTR(&cv2_core_convertScaleAbs_obj) }, \
    { MP_ROM_QSTR(MP_QSTR_minMaxLoc),        MP_ROM_PTR(&cv2_core_minMaxLoc_obj) }, \
    { MP_ROM_QSTR(MP_QSTR_LUT),              MP_ROM_PTR(&cv2_core_LUT_obj) }, \
    { MP_ROM_QSTR(MP_QSTR_countNonZero),     MP_ROM_PTR(&cv2_core_countNonZero_obj) }, \
    { MP_ROM_QSTR(MP_QSTR_flip),             MP_ROM_PTR(&cv2_core_flip_obj) }, \
    { MP_ROM_QSTR(MP_QSTR_rotate),           MP_ROM_PTR(&cv2_core_rotate_obj) }, \
    { MP_ROM_QSTR(MP_QSTR_transpose),        MP_ROM_PTR(&cv2_core_transpose_obj) },

#endif // CV_UPY_CORE_H
