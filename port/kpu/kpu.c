#include "nncase_wrap.h"
#include "nncase_type.h"
#include "kpu.h"
#include "py/obj.h"
#include "py/runtime.h"
#include "py/stream.h"
#include "py/builtin.h"
#include <stdio.h>
#include <string.h>
#include "ndarray.h"
#include "ulab.h"
#include "py/binary.h"
#include "ulab_tools.h"
#include <stdlib.h>
#include <sys/stat.h>

#include "hal_rvv_ops.h"

// 数据结构
typedef int mpy_datatype_t;

enum mpy_datatype
{
    dt_boolean = 0,
    dt_utf8char = 1,
    dt_int8 = 2,
    dt_int16 = 3,
    dt_int32 = 4,
    dt_int64 = 5,
    dt_uint8 = 6,
    dt_uint16 = 7,
    dt_uint32 = 8,
    dt_uint64 = 9,
    dt_float16 = 10,
    dt_float32 = 11,
    dt_float64 = 12,
    dt_bfloat16 = 13,
    dt_pointer = 14,
    dt_valuetype = 15,
};

const char* dtype_enum_str[] = {
    "dt_boolean",
    "dt_utf8char",
    "dt_int8",
    "dt_int16",
    "dt_int32",
    "dt_int64",
    "dt_uint8",
    "dt_uint16",
    "dt_uint32",
    "dt_uint64",
    "dt_float16",
    "dt_float32",
    "dt_float64",
    "dt_bfloat16",
    "dt_pointer",
    "dt_valuetype"
};

mp_obj_t get_dtype_str(mpy_datatype_t dt) {
    const char* str = dtype_enum_str[dt];
    mp_obj_t mp_str = mp_obj_new_str(str, strlen(str));
    return mp_str; 
}

// datatype
//| # Auto-generated CanMV stub docs. Edit the signatures/docstrings here.
//| module: datatype
//| """CanMV datatype module."""

STATIC const mp_rom_map_elem_t mp_datatype_locals_dict_table[] = {
    { MP_ROM_QSTR(MP_QSTR_dt_boolean), MP_ROM_INT(0) },
    { MP_ROM_QSTR(MP_QSTR_dt_utf8char), MP_ROM_INT(1) },
    { MP_ROM_QSTR(MP_QSTR_dt_int8), MP_ROM_INT(2) },
    { MP_ROM_QSTR(MP_QSTR_dt_int16), MP_ROM_INT(3) },
    { MP_ROM_QSTR(MP_QSTR_dt_int32), MP_ROM_INT(4) },
    { MP_ROM_QSTR(MP_QSTR_dt_int64), MP_ROM_INT(5) },
    { MP_ROM_QSTR(MP_QSTR_dt_uint8), MP_ROM_INT(6) },
    { MP_ROM_QSTR(MP_QSTR_dt_uint16), MP_ROM_INT(7) },
    { MP_ROM_QSTR(MP_QSTR_dt_uint32), MP_ROM_INT(8) },
    { MP_ROM_QSTR(MP_QSTR_dt_uint64), MP_ROM_INT(9) },
    { MP_ROM_QSTR(MP_QSTR_dt_float16), MP_ROM_INT(10) },
    { MP_ROM_QSTR(MP_QSTR_dt_float32), MP_ROM_INT(11) },
    { MP_ROM_QSTR(MP_QSTR_dt_float64), MP_ROM_INT(12) },
    { MP_ROM_QSTR(MP_QSTR_dt_bfloat16), MP_ROM_INT(13) },
    { MP_ROM_QSTR(MP_QSTR_dt_pointer), MP_ROM_INT(14) },
    { MP_ROM_QSTR(MP_QSTR_dt_valuetype), MP_ROM_INT(15) },
};

STATIC MP_DEFINE_CONST_DICT(mp_datatype_locals_dict, mp_datatype_locals_dict_table);

const mp_obj_module_t mp_module_datatype = {
    .base = { &mp_type_module },
    .globals = (mp_obj_dict_t *)&mp_datatype_locals_dict,
};

MP_REGISTER_EXTENSIBLE_MODULE(MP_QSTR_datatype, mp_module_datatype);

MP_DEFINE_CONST_OBJ_TYPE(
    datatype_type,
    MP_QSTR_datatype,
    MP_TYPE_FLAG_INSTANCE_TYPE,
    locals_dict, &mp_datatype_locals_dict
);
// 实现各个功能函数

// init
STATIC mp_obj_t mp_kpu_make_new(const mp_obj_type_t* type, size_t n_args, size_t n_kw, const mp_obj_t* args) {
    mp_arg_check_num(n_args, n_kw, 0, 0, false);
    kpu_obj_t *self = m_new_obj_with_finaliser(kpu_obj_t);
    self->interp = Kpu_create();
    self->base.type = &kpu_type;
    return MP_OBJ_FROM_PTR(self);
}

STATIC mp_obj_t mp_kpu_destroy(mp_obj_t self_in) {
    kpu_obj_t *self = MP_OBJ_TO_PTR(self_in);
    if (self->interp != NULL) {
        Kpu_destroy(self->interp);
        self->interp = NULL;
    }
    return mp_const_none;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_1(kpu_destroy_obj, mp_kpu_destroy);

// load model
STATIC mp_obj_t mp_kpu_load_kmodel(mp_obj_t self_in, mp_obj_t filename_in) {
    
    if (!mp_obj_is_str(filename_in)) {
        mp_buffer_info_t bufferinfo;
        mp_get_buffer_raise(filename_in, &bufferinfo, MP_BUFFER_READ);
        kpu_obj_t *self = MP_OBJ_TO_PTR(self_in);
        bool flag = Kpu_load_kmodel_buffer(self->interp, (char* )bufferinfo.buf, bufferinfo.len);
        if(!flag)
            mp_raise_msg(&mp_type_EOFError, MP_ERROR_TEXT("KPU load model failed from buffer."));
    }
    else{
        kpu_obj_t *self = MP_OBJ_TO_PTR(self_in);
        self->base.type = &kpu_type;
        size_t len = 0;
        const char *filename = mp_obj_str_get_data(filename_in, &len);
        if (access(filename, R_OK) != 0)
            mp_raise_msg(&mp_type_OSError, MP_ERROR_TEXT("Kmodel file not exist."));
        bool flag = Kpu_load_kmodel_path(self->interp, filename);
        if(!flag)
            mp_raise_msg(&mp_type_EOFError, MP_ERROR_TEXT("KPU load model failed from path."));
    }
    
    return mp_const_none;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_2(kpu_load_kmodel_obj, mp_kpu_load_kmodel);


// kmodel run
STATIC mp_obj_t mp_kpu_run(mp_obj_t self_in) {
    kpu_obj_t *self = MP_OBJ_TO_PTR(self_in);
    bool flag = Kpu_run(self->interp);
    if(!flag)
        mp_raise_msg(&mp_type_RuntimeError, MP_ERROR_TEXT("KPU run failed."));
    return mp_const_none;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_1(kpu_run_obj, mp_kpu_run);


// set input tensor
STATIC mp_obj_t mp_kpu_set_input_tensor(mp_obj_t self_in, mp_obj_t index_in, mp_obj_t tensor_in) {
    kpu_obj_t *self = MP_OBJ_TO_PTR(self_in);
    size_t index = mp_obj_get_int(index_in);
    mp_runtime_tensor_obj_t *tensor = MP_OBJ_TO_PTR(tensor_in);
    bool flag = Kpu_set_input_tensor(self->interp, index, tensor->r_tensor);
    if(!flag)
        mp_raise_msg(&mp_type_ValueError, MP_ERROR_TEXT("KPU set input tensor failed."));
    return mp_const_none;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_3(kpu_set_input_tensor_obj, mp_kpu_set_input_tensor);

STATIC mp_obj_t mp_kpu_get_input_tensor(mp_obj_t self_in, mp_obj_t index_in) {
    kpu_obj_t *self = MP_OBJ_TO_PTR(self_in);
    size_t index = mp_obj_get_int(index_in);
    mp_runtime_tensor_obj_t *tensor = m_new_obj_with_finaliser(mp_runtime_tensor_obj_t);
    tensor->r_tensor = Kpu_get_input_tensor(self->interp, index);
    tensor->base.type = &rt_type;
    return MP_OBJ_TO_PTR(tensor);
}
STATIC MP_DEFINE_CONST_FUN_OBJ_2(kpu_get_input_tensor_obj, mp_kpu_get_input_tensor);

// set output tensor
STATIC mp_obj_t mp_kpu_set_output_tensor(mp_obj_t self_in, mp_obj_t index_in, mp_obj_t tensor_in) {
    kpu_obj_t *self = MP_OBJ_TO_PTR(self_in);
    size_t index = mp_obj_get_int(index_in);
    mp_runtime_tensor_obj_t *tensor = MP_OBJ_TO_PTR(tensor_in);
    bool flag = Kpu_set_output_tensor(self->interp, index, tensor->r_tensor);
    if(!flag)
        mp_raise_msg(&mp_type_ValueError, MP_ERROR_TEXT("KPU set output tensor failed."));
    return mp_const_none;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_3(kpu_set_output_tensor_obj, mp_kpu_set_output_tensor);

// get output tensor
STATIC mp_obj_t mp_kpu_get_output_tensor(mp_obj_t self_in, mp_obj_t index_in) {
    kpu_obj_t *self = MP_OBJ_TO_PTR(self_in);
    size_t index = mp_obj_get_int(index_in);
    mp_runtime_tensor_obj_t *tensor = m_new_obj_with_finaliser(mp_runtime_tensor_obj_t);
    tensor->r_tensor = Kpu_get_output_tensor(self->interp, index);
    tensor->base.type = &rt_type;
    return MP_OBJ_TO_PTR(tensor);
}
STATIC MP_DEFINE_CONST_FUN_OBJ_2(kpu_get_output_tensor_obj, mp_kpu_get_output_tensor);

// get input size
STATIC mp_obj_t mp_kpu_get_input_size(mp_obj_t self_in) {
    kpu_obj_t *self = MP_OBJ_TO_PTR(self_in);
    size_t input_size = (size_t)Kpu_inputs_size(self->interp);
    return mp_obj_new_int(input_size);
}
STATIC MP_DEFINE_CONST_FUN_OBJ_1(kpu_get_input_size_obj, mp_kpu_get_input_size);

// get output size
STATIC mp_obj_t mp_kpu_get_output_size(mp_obj_t self_in) {
    kpu_obj_t *self = MP_OBJ_TO_PTR(self_in);
    size_t output_size = (size_t)Kpu_outputs_size(self->interp);
    return mp_obj_new_int(output_size);
}
STATIC MP_DEFINE_CONST_FUN_OBJ_1(kpu_get_output_size_obj, mp_kpu_get_output_size);

// get input desc
STATIC mp_obj_t mp_kpu_get_input_desc(mp_obj_t self_in, mp_obj_t index_in) {   
    kpu_obj_t *self = MP_OBJ_TO_PTR(self_in);
    size_t index = mp_obj_get_int(index_in);
    
    struct tensor_desc data = Kpu_get_input_desc(self->interp, index);

    mp_obj_t dict = mp_obj_new_dict(3);
    mp_obj_dict_store(dict, MP_OBJ_NEW_QSTR(MP_QSTR_tensor_datatype), get_dtype_str(data.datatype));
    mp_obj_dict_store(dict, MP_OBJ_NEW_QSTR(MP_QSTR_tensor_start), mp_obj_new_int(data.start)); 
    mp_obj_dict_store(dict, MP_OBJ_NEW_QSTR(MP_QSTR_tensor_size), mp_obj_new_int(data.size));
    return dict;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_2(kpu_get_input_desc_obj, mp_kpu_get_input_desc);

// get output desc
STATIC mp_obj_t mp_kpu_get_output_desc(mp_obj_t self_in, mp_obj_t index_in) {

    kpu_obj_t *self = MP_OBJ_TO_PTR(self_in);
    size_t index = mp_obj_get_int(index_in);
    
    struct tensor_desc data = Kpu_get_output_desc(self->interp, index);

    mp_obj_t dict = mp_obj_new_dict(3);
    mp_obj_dict_store(dict, MP_OBJ_NEW_QSTR(MP_QSTR_tensor_datatype), get_dtype_str(data.datatype));
    mp_obj_dict_store(dict, MP_OBJ_NEW_QSTR(MP_QSTR_tensor_start), mp_obj_new_int(data.start)); 
    mp_obj_dict_store(dict, MP_OBJ_NEW_QSTR(MP_QSTR_tensor_size), mp_obj_new_int(data.size));
    return dict;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_2(kpu_get_output_desc_obj, mp_kpu_get_output_desc);

// set dict
//| # Auto-generated CanMV stub docs. Edit the signatures/docstrings here.
//| module: nncase_runtime
//| class kpu:
//|     """nncase_runtime.kpu object."""
//|     def __init__(self) -> None:
//|         """Create a nncase_runtime.kpu object."""
//|     def get_input_tensor(self, index: Any, /) -> Any:
//|         """Return input tensor for nncase_runtime.kpu."""
//|     def get_output_tensor(self, index: Any, /) -> Any:
//|         """Return output tensor for nncase_runtime.kpu."""
//|     def inputs_desc(self, index: Any, /) -> Any:
//|         """Perform inputs desc for nncase_runtime.kpu."""
//|     def inputs_size(self, /) -> Any:
//|         """Perform inputs size for nncase_runtime.kpu."""
//|     def load_kmodel(self, filename: Any, /) -> Any:
//|         """Load kmodel for nncase_runtime.kpu."""
//|     def outputs_desc(self, index: Any, /) -> Any:
//|         """Perform outputs desc for nncase_runtime.kpu."""
//|     def outputs_size(self, /) -> Any:
//|         """Perform outputs size for nncase_runtime.kpu."""
//|     def run(self, /) -> Any:
//|         """Run nncase_runtime.kpu."""
//|     def set_input_tensor(self, index: Any, tensor: Any, /) -> Any:
//|         """Set input tensor for nncase_runtime.kpu."""
//|     def set_output_tensor(self, index: Any, tensor: Any, /) -> Any:
//|         """Set output tensor for nncase_runtime.kpu."""

//| # Auto-generated CanMV stub docs. Edit the signatures/docstrings here.
//| module: kpu
//| """CanMV kpu module."""
//| def __del__(obj: Any, /) -> Any:
//|     """Perform del for kpu."""
//| def get_input_tensor(obj: Any, index: Any, /) -> Any:
//|     """Return input tensor for kpu."""
//| def get_output_tensor(obj: Any, index: Any, /) -> Any:
//|     """Return output tensor for kpu."""
//| def inputs_desc(obj: Any, index: Any, /) -> Any:
//|     """Perform inputs desc for kpu."""
//| def inputs_size(obj: Any, /) -> Any:
//|     """Perform inputs size for kpu."""
//| def load_kmodel(obj: Any, filename: Any, /) -> Any:
//|     """Load kmodel for kpu."""
//| def outputs_desc(obj: Any, index: Any, /) -> Any:
//|     """Perform outputs desc for kpu."""
//| def outputs_size(obj: Any, /) -> Any:
//|     """Perform outputs size for kpu."""
//| def run(obj: Any, /) -> Any:
//|     """Run kpu."""
//| def set_input_tensor(obj: Any, index: Any, tensor: Any, /) -> Any:
//|     """Set input tensor for kpu."""
//| def set_output_tensor(obj: Any, index: Any, tensor: Any, /) -> Any:
//|     """Set output tensor for kpu."""

STATIC const mp_rom_map_elem_t kpu_locals_dict_table[] = {
    { MP_ROM_QSTR(MP_QSTR___name__), MP_ROM_QSTR(MP_QSTR_kpu) },
    { MP_ROM_QSTR(MP_QSTR___del__), MP_ROM_PTR(&kpu_destroy_obj) },
    { MP_ROM_QSTR(MP_QSTR_load_kmodel), MP_ROM_PTR(&kpu_load_kmodel_obj) },
    { MP_ROM_QSTR(MP_QSTR_run), MP_ROM_PTR(&kpu_run_obj) },
    { MP_ROM_QSTR(MP_QSTR_get_input_tensor), MP_ROM_PTR(&kpu_get_input_tensor_obj) },
    { MP_ROM_QSTR(MP_QSTR_set_input_tensor), MP_ROM_PTR(&kpu_set_input_tensor_obj) },
    { MP_ROM_QSTR(MP_QSTR_get_output_tensor), MP_ROM_PTR(&kpu_get_output_tensor_obj) },
    { MP_ROM_QSTR(MP_QSTR_set_output_tensor), MP_ROM_PTR(&kpu_set_output_tensor_obj) },
    { MP_ROM_QSTR(MP_QSTR_inputs_size), MP_ROM_PTR(&kpu_get_input_size_obj) },
    { MP_ROM_QSTR(MP_QSTR_outputs_size), MP_ROM_PTR(&kpu_get_output_size_obj) },
    { MP_ROM_QSTR(MP_QSTR_inputs_desc), MP_ROM_PTR(&kpu_get_input_desc_obj) },
    { MP_ROM_QSTR(MP_QSTR_outputs_desc), MP_ROM_PTR(&kpu_get_output_desc_obj) },
};

STATIC MP_DEFINE_CONST_DICT(kpu_locals_dict, kpu_locals_dict_table);
const mp_obj_module_t mp_module_kpu = {
    .base = { &mp_type_module },
    .globals = (mp_obj_dict_t *)&kpu_locals_dict,
};

MP_REGISTER_EXTENSIBLE_MODULE(MP_QSTR_kpu, mp_module_kpu);

MP_DEFINE_CONST_OBJ_TYPE(
    kpu_type,
    MP_QSTR_kpu,
    MP_TYPE_FLAG_NONE,
    make_new, mp_kpu_make_new,
    locals_dict, &kpu_locals_dict
    );


STATIC mp_obj_t mp_to_numpy(mp_obj_t self_in) {

    mp_runtime_tensor_obj_t *self = MP_OBJ_TO_PTR(self_in);
    rt_to_ndarray_info info;
    to_numpy(self->r_tensor, &info);
    size_t *mp_shape = m_new(size_t, ULAB_MAX_DIMS);
    int32_t *mp_stride = m_new(int32_t, ULAB_MAX_DIMS);
    size_t size_bytes = ulab_binary_get_size(info.dtype_);
    for(int i=0; i<info.ndim_; i++) {
        mp_shape[ULAB_MAX_DIMS - 1-i] = (size_t)info.shape_[info.ndim_ - 1 - i];
        mp_stride[ULAB_MAX_DIMS - 1-i] = (int32_t)info.strides_[info.ndim_ - 1 - i]*size_bytes;
    }
    ndarray_obj_t *result = ndarray_new_ndarray(info.ndim_, mp_shape, mp_stride, info.dtype_);
    hal_rvv_memcpy((void *)result->origin, (void *)info.data_, result->len * result->itemsize);
    
    free(info.data_);
    info.data_ = NULL;
    m_del(size_t, mp_shape, ULAB_MAX_DIMS);
    m_del(int32_t, mp_stride, ULAB_MAX_DIMS);
    return MP_OBJ_FROM_PTR(result);
}
STATIC MP_DEFINE_CONST_FUN_OBJ_1(mp_to_numpy_obj, mp_to_numpy);

static mp_obj_t mp_runtime_tensor_release(mp_obj_t runtime_tensor_obj) {
    mp_runtime_tensor_obj_t *self = MP_OBJ_TO_PTR(runtime_tensor_obj);
    runtime_tensor_release(self->r_tensor);
    return mp_const_none;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_1(mp_runtime_tensor_del_obj, mp_runtime_tensor_release);
//| # Auto-generated CanMV stub docs. Edit the signatures/docstrings here.
//| module: runtime_tensor
//| """CanMV runtime_tensor module."""
//| def __del__(runtime_tensor: Any, /) -> Any:
//|     """Perform del for runtime_tensor."""
//| def to_numpy(obj: Any, /) -> Any:
//|     """Convert runtime_tensor to numpy."""


STATIC const mp_rom_map_elem_t mp_rt_dict_table[] = {
    { MP_ROM_QSTR(MP_QSTR___name__), MP_ROM_QSTR(MP_QSTR_runtime_tensor) },
    { MP_ROM_QSTR(MP_QSTR___del__), MP_ROM_PTR(&mp_runtime_tensor_del_obj) },
    { MP_ROM_QSTR(MP_QSTR_to_numpy), MP_ROM_PTR(&mp_to_numpy_obj) },
};

STATIC MP_DEFINE_CONST_DICT(mp_rt_dict, mp_rt_dict_table);

const mp_obj_module_t mp_module_rt = {
    .base = { &mp_type_module },
    .globals = (mp_obj_dict_t *)&mp_rt_dict,
};

MP_REGISTER_EXTENSIBLE_MODULE(MP_QSTR_runtime_tensor, mp_module_rt);

MP_DEFINE_CONST_OBJ_TYPE(
    rt_type,
    MP_QSTR_runtime_tensor,
    MP_TYPE_FLAG_NONE,
    locals_dict, &mp_rt_dict
);






