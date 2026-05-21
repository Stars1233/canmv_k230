extern "C" {
#include "py/obj.h"
#include "py/runtime.h"
}

#include "core.h"
#include "imgproc.h"
#include "imgcodecs.h"
#include "highgui.h"

STATIC const mp_rom_map_elem_t cv2_module_globals_table[] = {
    { MP_ROM_QSTR(MP_QSTR___name__), MP_ROM_QSTR(MP_QSTR_cv2) },

    OPENCV_CORE_GLOBALS
    OPENCV_CORE_CONSTANTS
    OPENCV_IMGPROC_GLOBALS
    OPENCV_IMGPROC_CONSTANTS
    OPENCV_COLOR_CONVERSION_CONSTANTS
    OPENCV_IMGCODECS_GLOBALS
    OPENCV_IMGCODECS_CONSTANTS
    OPENCV_HIGHGUI_GLOBALS
};

STATIC MP_DEFINE_CONST_DICT(cv2_module_globals, cv2_module_globals_table);

extern const mp_obj_module_t cv2_module = {
    .base = { &mp_type_module },
    .globals = (mp_obj_dict_t *)&cv2_module_globals,
};

MP_REGISTER_MODULE(MP_QSTR_cv2, cv2_module);
