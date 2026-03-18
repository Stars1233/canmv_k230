#ifndef FUZZ_TEST_STUB_PY_RUNTIME_H_
#define FUZZ_TEST_STUB_PY_RUNTIME_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

#define NORETURN __attribute__((noreturn))
#define MP_ERROR_TEXT(x) (x)

typedef const char *mp_rom_error_text_t;

extern int mp_type_OSError;
void mp_raise_msg(const void *exc_type, mp_rom_error_text_t msg) NORETURN;

#ifdef __cplusplus
}
#endif

#endif
