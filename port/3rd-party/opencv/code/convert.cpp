#include "convert.h"
#include "numpy.h"
extern "C" {
#include "ndarray.h"
#include "ulab.h"
#include "py/runtime.h"
}
#include <opencv2/core/mat.hpp>
#include <string.h>

using namespace cv;

uint8_t ndarray_type_to_mat_depth(uint8_t dtype)
{
    switch(dtype) {
        case NDARRAY_UINT8:  return CV_8U;
        case NDARRAY_INT8:   return CV_8S;
        case NDARRAY_UINT16: return CV_16U;
        case NDARRAY_INT16:  return CV_16S;
        case NDARRAY_FLOAT:  return CV_32F;
        default: return CV_8U;
    }
}

uint8_t mat_depth_to_ndarray_type(int depth)
{
    switch(depth) {
        case CV_8U:  return NDARRAY_UINT8;
        case CV_8S:  return NDARRAY_INT8;
        case CV_16U: return NDARRAY_UINT16;
        case CV_16S: return NDARRAY_INT16;
        case CV_32S: return NDARRAY_FLOAT;  // ulab lacks INT32; float can represent all int32
        case CV_32F: return NDARRAY_FLOAT;
        case CV_64F: return NDARRAY_FLOAT;  // ulab lacks FLOAT64; truncated to system float
        default: return NDARRAY_UINT8;
    }
}

// NOTE: callers must check for nullptr return value (empty/invalid Mat)
ndarray_obj_t *mat_to_ndarray(Mat &mat)
{
    if(mat.empty())
        return nullptr;

    UMatData* u = mat.u;
    if(!u)
    {
        Mat temp;
        temp.allocator = GetNumpyAllocator();
        mat.copyTo(temp);
        u = temp.u;
        if(!u)
            return nullptr;
        ndarray_obj_t *ndarray = (ndarray_obj_t *)u->userdata;
        return ndarray;
    }

    if(u->userdata)
    {
        ndarray_obj_t *ndarray = (ndarray_obj_t *)u->userdata;
        return ndarray;
    }

    Mat temp;
    temp.allocator = GetNumpyAllocator();
    mat.copyTo(temp);
    u = temp.u;
    if(!u || !u->userdata)
        return nullptr;

    ndarray_obj_t *ndarray = (ndarray_obj_t *)u->userdata;
    return ndarray;
}

mp_obj_t mat_to_mp_obj(Mat &mat)
{
    if(mat.empty())
        return mp_const_none;

    ndarray_obj_t *ndarray = mat_to_ndarray(mat);
    if(!ndarray)
        return mp_const_none;

    return MP_OBJ_FROM_PTR(ndarray);
}

Mat ndarray_to_mat(ndarray_obj_t *ndarray)
{
    if(!ndarray || !ndarray->array)
        return Mat();

    uint8_t dtype = ndarray->dtype;
    int type = ndarray_type_to_mat_depth(dtype);
    uint8_t ndim = ndarray->ndim;

    size_t _sizes[ULAB_MAX_DIMS];
    size_t _strides[ULAB_MAX_DIMS];
    bool needcopy = false;

    for(uint8_t i = 0; i < ndarray->ndim; i++)
    {
        _sizes[i] = ndarray->shape[ULAB_MAX_DIMS - ndarray->ndim + i];
        _strides[i] = ndarray->strides[ULAB_MAX_DIMS - ndarray->ndim + i];
    }

    bool ismultichannel = false;
    int channels = 1;

    // Check for contiguous layout
    size_t expected_stride = ndarray->itemsize;
    for(int i = ndim - 1; i >= 0; i--)
    {
        if(_strides[i] != (int32_t)expected_stride)
        {
            needcopy = true;
            break;
        }
        expected_stride *= _sizes[i];
    }

    // If 3D, treat last dim as channels
    if(ndim == 3 && _sizes[ndim-1] <= 4)
    {
        ismultichannel = true;
        channels = _sizes[ndim-1];
        type = CV_MAKETYPE(CV_MAT_DEPTH(type), channels);
    }

    int dims = ndim;
    if(ismultichannel)
        dims--;

    int _type = type;
    std::vector<int> sz(dims);
    for(int i = 0; i < dims; i++)
        sz[i] = (int)_sizes[i];

    ndarray_obj_t *src_ndarray = ndarray;

    if(needcopy)
    {
        ndarray = ndarray_copy_view_convert_type(src_ndarray, dtype);
        if(!ndarray)
            return Mat();
        for(uint8_t i = 0; i < ndim; i++)
        {
            _strides[i] = ndarray->strides[ULAB_MAX_DIMS - ndim + i];
        }
    }

    size_t step[ULAB_MAX_DIMS];
    for(int i = 0; i < dims; i++)
    {
        step[i] = ndarray->strides[ULAB_MAX_DIMS - ndim + i];
    }

    void *data = (void *)ndarray->array;

    Mat mat = Mat(dims, sz.data(), _type, data, (size_t*)&_strides[0]);
    mat.u = NumpyAllocator_AllocateNdarray(ndarray, dims, sz.data(), _type, step);
    mat.addref();
    mat.allocator = GetNumpyAllocator();

    return mat;
}

Mat mp_obj_to_mat(mp_obj_t obj)
{
    if(obj == mp_const_none)
        return Mat();

    ndarray_obj_t *ndarray = ndarray_from_mp_obj(obj, 0);
    if(!ndarray)
        return Mat();

    return ndarray_to_mat(ndarray);
}

Size mp_obj_to_size(mp_obj_t obj)
{
    if(obj == mp_const_none)
    {
        mp_raise_TypeError(MP_ERROR_TEXT("size cannot be None"));
    }

    // Accept tuple or list: (w, h) or [w, h]
    if(mp_obj_is_type(obj, &mp_type_tuple) || mp_obj_is_type(obj, &mp_type_list))
    {
        size_t len;
        mp_obj_t *items;
        mp_obj_get_array(obj, &len, &items);
        if(len < 2)
            mp_raise_ValueError(MP_ERROR_TEXT("size requires 2 elements"));
        return Size(mp_obj_get_int(items[0]), mp_obj_get_int(items[1]));
    }

    // Accept ndarray of 2 elements
    if(mp_obj_is_type(obj, &ulab_ndarray_type))
    {
        ndarray_obj_t *ndarray = (ndarray_obj_t *)MP_OBJ_TO_PTR(obj);
        if(ndarray->ndim != 1 || ndarray->len < 2)
            mp_raise_ValueError(MP_ERROR_TEXT("Size requires 2 elements"));
        mp_float_t v0 = ndarray_get_float_index(ndarray->array, ndarray->dtype, 0);
        mp_float_t v1 = ndarray_get_float_index(ndarray->array, ndarray->dtype, 1);
        return Size((int)v0, (int)v1);
    }

    mp_raise_TypeError(MP_ERROR_TEXT("Expected tuple, list, or ndarray for size"));
    return Size();
}

Scalar mp_obj_to_scalar(mp_obj_t obj)
{
    if(obj == mp_const_none)
        return Scalar::all(0);

    // Accept tuple or list: (b, g, r) or [b, g, r]
    if(mp_obj_is_type(obj, &mp_type_tuple) || mp_obj_is_type(obj, &mp_type_list))
    {
        size_t len;
        mp_obj_t *items;
        mp_obj_get_array(obj, &len, &items);
        Scalar s = Scalar::all(0);
        for(size_t i = 0; i < len && i < 4; i++)
            s[i] = mp_obj_get_float(items[i]);
        return s;
    }

    // Accept ndarray
    if(mp_obj_is_type(obj, &ulab_ndarray_type))
    {
        ndarray_obj_t *ndarray = (ndarray_obj_t *)MP_OBJ_TO_PTR(obj);
        if(ndarray->ndim != 1)
            mp_raise_ValueError(MP_ERROR_TEXT("Scalar must be 1D array"));
        Scalar s;
        for(size_t i = 0; i < ndarray->len && i < 4; i++)
            s[i] = ndarray_get_float_index(ndarray->array, ndarray->dtype, i);
        return s;
    }

    mp_raise_TypeError(MP_ERROR_TEXT("Expected tuple, list, or ndarray for scalar"));
    return Scalar();
}

Point mp_obj_to_point(mp_obj_t obj)
{
    if(obj == mp_const_none)
    {
        mp_raise_TypeError(MP_ERROR_TEXT("point cannot be None"));
    }

    // Accept tuple or list: (x, y) or [x, y]
    if(mp_obj_is_type(obj, &mp_type_tuple) || mp_obj_is_type(obj, &mp_type_list))
    {
        size_t len;
        mp_obj_t *items;
        mp_obj_get_array(obj, &len, &items);
        if(len < 2)
            mp_raise_ValueError(MP_ERROR_TEXT("point requires 2 elements"));
        return Point(mp_obj_get_int(items[0]), mp_obj_get_int(items[1]));
    }

    // Accept ndarray of 2 elements
    if(mp_obj_is_type(obj, &ulab_ndarray_type))
    {
        ndarray_obj_t *ndarray = (ndarray_obj_t *)MP_OBJ_TO_PTR(obj);
        if(ndarray->ndim != 1 || ndarray->len < 2)
            mp_raise_ValueError(MP_ERROR_TEXT("Point requires 2 elements"));
        mp_float_t v0 = ndarray_get_float_index(ndarray->array, ndarray->dtype, 0);
        mp_float_t v1 = ndarray_get_float_index(ndarray->array, ndarray->dtype, 1);
        return Point((int)v0, (int)v1);
    }

    mp_raise_TypeError(MP_ERROR_TEXT("Expected tuple, list, or ndarray for point"));
    return Point();
}
