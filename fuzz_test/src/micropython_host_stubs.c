#include <stdlib.h>
#include <stdarg.h>
#include <stdint.h>
#include <string.h>

#include "py/runtime.h"

int mp_type_OSError = 0;
int mp_type_ValueError = 0;
int mp_type_RuntimeError = 0;
int mp_type_TypeError = 0;
int mp_type_MemoryError = 0;
mp_obj_t mp_const_none = NULL;
mp_obj_t mp_const_true = (mp_obj_t)(uintptr_t)1;
mp_obj_t mp_const_false = NULL;

void mp_raise_msg(const void *exc_type, mp_rom_error_text_t msg) {
    (void) exc_type;
    (void) msg;
    abort();
}

void mp_raise_msg_varg(const void *exc_type, mp_rom_error_text_t fmt, ...) {
    (void) exc_type;
    (void) fmt;
    abort();
}

int mp_obj_get_int(mp_obj_t obj) {
    return (int)(intptr_t)obj;
}

float mp_obj_get_float(mp_obj_t obj) {
    return (float)(intptr_t)obj;
}

mp_obj_t mp_obj_new_int(int value) {
    return (mp_obj_t)(intptr_t)value;
}

mp_obj_t mp_obj_new_bool(int value) {
    return value ? mp_const_true : mp_const_false;
}

mp_obj_t mp_obj_new_float(float value) {
    return (mp_obj_t)(intptr_t)value;
}

mp_obj_t mp_obj_new_str(const char *data, size_t len) {
    char *copy = (char *)malloc(len + 1U);
    if (copy == NULL) {
        abort();
    }
    memcpy(copy, data, len);
    copy[len] = '\0';
    return (mp_obj_t)copy;
}

mp_obj_t mp_obj_new_tuple(size_t n, const mp_obj_t *items) {
    (void)n;
    return (mp_obj_t)items;
}

mp_obj_t mp_obj_new_list(size_t n, mp_obj_t *items) {
    mp_obj_list_t *list = (mp_obj_list_t *)calloc(1U, sizeof(*list));
    if (list == NULL) {
        abort();
    }
    list->len = n;
    list->items = items;
    return (mp_obj_t)list;
}

void mp_obj_list_append(mp_obj_t list_obj, mp_obj_t item) {
    mp_obj_list_t *list = (mp_obj_list_t *)list_obj;
    if (list == NULL) {
        abort();
    }
    mp_obj_t *items = (mp_obj_t *)realloc(list->items, (list->len + 1U) * sizeof(*items));
    if (items == NULL) {
        abort();
    }
    items[list->len++] = item;
    list->items = items;
}
