#include <stdlib.h>

#include "py/runtime.h"

int mp_type_OSError = 0;

void mp_raise_msg(const void *exc_type, mp_rom_error_text_t msg) {
    (void) exc_type;
    (void) msg;
    abort();
}
