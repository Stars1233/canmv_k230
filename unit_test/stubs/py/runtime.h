#ifndef UNIT_TEST_STUB_PY_RUNTIME_H_
#define UNIT_TEST_STUB_PY_RUNTIME_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include "py/obj.h"

#define NORETURN __attribute__((noreturn))
#define MP_ERROR_TEXT(x) (x)

typedef const char *mp_rom_error_text_t;

extern int mp_type_OSError;
extern int mp_type_ValueError;
extern int mp_type_RuntimeError;
extern int mp_type_TypeError;
extern mp_obj_t mp_const_none;
extern mp_obj_t mp_const_true;
extern mp_obj_t mp_const_false;

void mp_raise_msg(const void *exc_type, mp_rom_error_text_t msg) NORETURN;
void mp_raise_msg_varg(const void *exc_type, mp_rom_error_text_t fmt, ...) NORETURN;
int mp_obj_get_int(mp_obj_t obj);
float mp_obj_get_float(mp_obj_t obj);
mp_obj_t mp_obj_new_int(int value);
mp_obj_t mp_obj_new_bool(int value);
mp_obj_t mp_obj_new_float(float value);
mp_obj_t mp_obj_new_str(const char *data, size_t len);
mp_obj_t mp_obj_new_tuple(size_t n, const mp_obj_t *items);

#ifdef __cplusplus
}
#endif

#endif
