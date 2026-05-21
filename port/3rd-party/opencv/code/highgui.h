#ifndef CV_UPY_HIGHGUI_H
#define CV_UPY_HIGHGUI_H

#ifdef __cplusplus
extern "C" {
#endif
#include "py/obj.h"
#ifdef __cplusplus
}
#endif

extern const mp_obj_fun_builtin_var_t cv2_highgui_waitKey_obj;
extern const mp_obj_fun_builtin_var_t cv2_highgui_waitKeyEx_obj;

#define OPENCV_HIGHGUI_GLOBALS \
    { MP_ROM_QSTR(MP_QSTR_waitKey),   MP_ROM_PTR(&cv2_highgui_waitKey_obj) }, \
    { MP_ROM_QSTR(MP_QSTR_waitKeyEx), MP_ROM_PTR(&cv2_highgui_waitKeyEx_obj) },

#endif // CV_UPY_HIGHGUI_H
