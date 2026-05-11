#ifndef FUZZ_TEST_STUB_PY_OBJ_H_
#define FUZZ_TEST_STUB_PY_OBJ_H_

#include <stddef.h>
#include <stdint.h>

typedef void *mp_obj_t;
typedef uintptr_t mp_uint_t;
typedef intptr_t mp_int_t;
typedef uint16_t qstr;

typedef struct _mp_obj_base_t {
    const void *type;
} mp_obj_base_t;

typedef struct _mp_obj_type_t {
    mp_obj_base_t base;
} mp_obj_type_t;

typedef struct _mp_obj_dict_t {
    mp_obj_base_t base;
} mp_obj_dict_t;

typedef struct _mp_obj_module_t {
    mp_obj_base_t base;
    mp_obj_dict_t *globals;
} mp_obj_module_t;

typedef struct _mp_rom_map_elem_t {
    uintptr_t key;
    uintptr_t value;
} mp_rom_map_elem_t;

typedef struct _mp_obj_list_t {
    mp_obj_base_t base;
    size_t len;
    mp_obj_t *items;
} mp_obj_list_t;

#define MP_ROM_QSTR(q) ((uintptr_t)(q))
#define MP_ROM_PTR(p) ((uintptr_t)(p))
#define MP_ROM_INT(i) ((uintptr_t)(i))
#define MP_OBJ_TO_PTR(o) ((void *)(o))
#define MP_OBJ_FROM_PTR(p) ((mp_obj_t)(p))
#define MP_ARRAY_SIZE(a) (sizeof(a) / sizeof((a)[0]))

#endif
