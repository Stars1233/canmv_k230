// Override MP_DEFINE_CONST_FUN_OBJ_KW to force external linkage in C++
// and use C++20-compatible fully-designated initializers.
// The original MicroPython macro mixes positional and designated
// initializers, which C++20 does not allow.
#undef MP_DEFINE_CONST_FUN_OBJ_KW
#define MP_DEFINE_CONST_FUN_OBJ_KW(obj_name, n_args_min, fun_name) \
    extern const mp_obj_fun_builtin_var_t obj_name = { \
        .base = {.type = &mp_type_fun_builtin_var}, \
        .sig = MP_OBJ_FUN_MAKE_SIG(n_args_min, MP_OBJ_FUN_ARGS_MAX, true), \
        .fun = {.kw = fun_name} \
    }
