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
extern const mp_obj_fun_builtin_var_t cv2_core_add_obj;
extern const mp_obj_fun_builtin_var_t cv2_core_subtract_obj;
extern const mp_obj_fun_builtin_var_t cv2_core_multiply_obj;
extern const mp_obj_fun_builtin_var_t cv2_core_divide_obj;
extern const mp_obj_fun_builtin_var_t cv2_core_addWeighted_obj;
extern const mp_obj_fun_builtin_var_t cv2_core_absdiff_obj;
extern const mp_obj_fun_builtin_var_t cv2_core_bitwise_and_obj;
extern const mp_obj_fun_builtin_var_t cv2_core_bitwise_or_obj;
extern const mp_obj_fun_builtin_var_t cv2_core_bitwise_xor_obj;
extern const mp_obj_fun_builtin_var_t cv2_core_bitwise_not_obj;
extern const mp_obj_fun_builtin_var_t cv2_core_split_obj;
extern const mp_obj_fun_builtin_var_t cv2_core_merge_obj;
extern const mp_obj_fun_builtin_var_t cv2_core_mean_obj;
extern const mp_obj_fun_builtin_var_t cv2_core_normalize_obj;
extern const mp_obj_fun_builtin_var_t cv2_core_compare_obj;

#define OPENCV_CORE_GLOBALS \
    { MP_ROM_QSTR(MP_QSTR_inRange),          MP_ROM_PTR(&cv2_core_inRange_obj) }, \
    { MP_ROM_QSTR(MP_QSTR_convertScaleAbs),  MP_ROM_PTR(&cv2_core_convertScaleAbs_obj) }, \
    { MP_ROM_QSTR(MP_QSTR_minMaxLoc),        MP_ROM_PTR(&cv2_core_minMaxLoc_obj) }, \
    { MP_ROM_QSTR(MP_QSTR_LUT),              MP_ROM_PTR(&cv2_core_LUT_obj) }, \
    { MP_ROM_QSTR(MP_QSTR_countNonZero),     MP_ROM_PTR(&cv2_core_countNonZero_obj) }, \
    { MP_ROM_QSTR(MP_QSTR_flip),             MP_ROM_PTR(&cv2_core_flip_obj) }, \
    { MP_ROM_QSTR(MP_QSTR_rotate),           MP_ROM_PTR(&cv2_core_rotate_obj) }, \
    { MP_ROM_QSTR(MP_QSTR_transpose),        MP_ROM_PTR(&cv2_core_transpose_obj) }, \
    { MP_ROM_QSTR(MP_QSTR_add),              MP_ROM_PTR(&cv2_core_add_obj) }, \
    { MP_ROM_QSTR(MP_QSTR_subtract),         MP_ROM_PTR(&cv2_core_subtract_obj) }, \
    { MP_ROM_QSTR(MP_QSTR_multiply),         MP_ROM_PTR(&cv2_core_multiply_obj) }, \
    { MP_ROM_QSTR(MP_QSTR_divide),           MP_ROM_PTR(&cv2_core_divide_obj) }, \
    { MP_ROM_QSTR(MP_QSTR_addWeighted),      MP_ROM_PTR(&cv2_core_addWeighted_obj) }, \
    { MP_ROM_QSTR(MP_QSTR_absdiff),          MP_ROM_PTR(&cv2_core_absdiff_obj) }, \
    { MP_ROM_QSTR(MP_QSTR_bitwise_and),      MP_ROM_PTR(&cv2_core_bitwise_and_obj) }, \
    { MP_ROM_QSTR(MP_QSTR_bitwise_or),       MP_ROM_PTR(&cv2_core_bitwise_or_obj) }, \
    { MP_ROM_QSTR(MP_QSTR_bitwise_xor),      MP_ROM_PTR(&cv2_core_bitwise_xor_obj) }, \
    { MP_ROM_QSTR(MP_QSTR_bitwise_not),      MP_ROM_PTR(&cv2_core_bitwise_not_obj) }, \
    { MP_ROM_QSTR(MP_QSTR_split),            MP_ROM_PTR(&cv2_core_split_obj) }, \
    { MP_ROM_QSTR(MP_QSTR_merge),            MP_ROM_PTR(&cv2_core_merge_obj) }, \
    { MP_ROM_QSTR(MP_QSTR_mean),             MP_ROM_PTR(&cv2_core_mean_obj) }, \
    { MP_ROM_QSTR(MP_QSTR_normalize),        MP_ROM_PTR(&cv2_core_normalize_obj) }, \
    { MP_ROM_QSTR(MP_QSTR_compare),          MP_ROM_PTR(&cv2_core_compare_obj) },

#endif // CV_UPY_CORE_H
