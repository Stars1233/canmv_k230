#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "py/runtime.h"

#include "modmachine.h"

STATIC mp_obj_t machine_led_make_new(const mp_obj_type_t* type, size_t n_args, size_t n_kw, const mp_obj_t* args)
{
    mp_raise_msg_varg(&mp_type_RuntimeError,
                      MP_ERROR_TEXT("machine.LED is deprecated. Please use the 'neopixel' module instead.\n"
                                    "Refer to: https://docs.micropython.org/en/latest/library/neopixel.html"));

    return mp_const_none;
}

MP_DEFINE_CONST_OBJ_TYPE(machine_led_type, MP_QSTR_LED, MP_TYPE_FLAG_NONE, make_new, machine_led_make_new);
