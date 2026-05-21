#include "highgui.h"
extern "C" {
#include "py/runtime.h"
}
#include "cv_upy_macros.h"
#include <unistd.h>
#include <sys/select.h>
#include <errno.h>

// waitKey/waitKeyEx: reads from stdin (fd 0) with optional timeout.
// On K230 MicroPython, stdin is the UART REPL. Returns -1 on timeout
// or if stdin is not available. Ctrl+C (ASCII 3) maps to ESC (27).
STATIC mp_obj_t cv2_highgui_waitKey_fun(size_t n_args, const mp_obj_t *pos_args, mp_map_t *kw_args)
{
    enum { ARG_delay };
    static const mp_arg_t allowed_args[] = {
        { MP_QSTR_delay, MP_ARG_INT, {.u_int = 0} },
    };
    mp_arg_val_t args[MP_ARRAY_SIZE(allowed_args)];
    mp_arg_parse_all(n_args, pos_args, kw_args, MP_ARRAY_SIZE(allowed_args), allowed_args, args);

    int delay_ms = args[ARG_delay].u_int;
    int key = -1;

    if(delay_ms < 0)
        delay_ms = 0;

    fd_set readfds;
    FD_ZERO(&readfds);
    FD_SET(0, &readfds);

    struct timeval tv;
    struct timeval *ptv = NULL;
    if(delay_ms > 0)
    {
        tv.tv_sec = delay_ms / 1000;
        tv.tv_usec = (delay_ms % 1000) * 1000;
        ptv = &tv;
    }

    int ret = select(1, &readfds, NULL, NULL, ptv);
    if(ret > 0 && FD_ISSET(0, &readfds))
    {
        char c;
        if(read(0, &c, 1) > 0)
            key = (unsigned char)c;
    }
    // ret==0: timeout → key=-1
    // ret<0: error (e.g. no terminal) → key=-1

    if(key == 3)        // Ctrl+C
        key = 27;       // ESC

    return mp_obj_new_int(key);
}
MP_DEFINE_CONST_FUN_OBJ_KW(cv2_highgui_waitKey_obj, 0, cv2_highgui_waitKey_fun);

STATIC mp_obj_t cv2_highgui_waitKeyEx_fun(size_t n_args, const mp_obj_t *pos_args, mp_map_t *kw_args)
{
    return cv2_highgui_waitKey_fun(n_args, pos_args, kw_args);
}
MP_DEFINE_CONST_FUN_OBJ_KW(cv2_highgui_waitKeyEx_obj, 0, cv2_highgui_waitKeyEx_fun);