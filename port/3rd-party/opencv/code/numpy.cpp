#include "numpy.h"
extern "C" {
#include "py/runtime.h"
#include "ndarray.h"
#include "ulab.h"
}
#include <opencv2/core/mat.hpp>
#include <new>

extern "C" {
#include "py/gc.h"
}

using namespace cv;

class NumpyAllocator : public MatAllocator
{
public:
    NumpyAllocator() { stdAllocator = Mat::getStdAllocator(); }
    ~NumpyAllocator() {}

    UMatData* allocate(ndarray_obj_t* ndarray, int dims, const int* sizes, int type, size_t* step) const
    {
        UMatData* u = new UMatData(this);
        u->data = u->origdata = (uchar*)ndarray->array;
        u->userdata = ndarray;

        if(ndarray)
        {
            for(int i = 0; i < dims - 1; i++)
                step[i] = ndarray->strides[ULAB_MAX_DIMS - ndarray->ndim + i];
            step[dims-1] = CV_ELEM_SIZE(type);
            u->size = sizes[0] * step[0];
        }
        else
        {
            for(int i = 0; i < dims; i++)
                step[i] = 0;
            u->size = 0;
        }
        return u;
    }

    UMatData* allocate(int dims0, const int* sizes, int type,
                       void* data0, size_t* step, AccessFlag flags,
                       UMatUsageFlags usageFlags) const override
    {
        if(data0)
        {
            CV_Assert(0 && "NumpyAllocator: data0 should be NULL in allocate");
        }

        uint8_t ndim;
        size_t _sizes[ULAB_MAX_DIMS];
        uint8_t typenum = NDARRAY_UINT8;

        int depth = CV_MAT_DEPTH(type);
        int cn = CV_MAT_CN(type);

        if(cn > 1)
        {
            ndim = dims0 + 1;
        }
        else
        {
            ndim = dims0;
        }

        for(int i = 0; i < ndim; i++)
        {
            _sizes[ULAB_MAX_DIMS - ndim + i] = (i == ndim - 1 && cn > 1) ? cn : sizes[i];
        }

        switch(depth)
        {
            case CV_8U:  typenum = NDARRAY_UINT8;  break;
            case CV_8S:  typenum = NDARRAY_INT8;   break;
            case CV_16U: typenum = NDARRAY_UINT16; break;
            case CV_16S: typenum = NDARRAY_INT16;  break;
            case CV_32S: typenum = NDARRAY_FLOAT;  break;
            case CV_32F: typenum = NDARRAY_FLOAT;  break;
            case CV_64F: typenum = NDARRAY_FLOAT;  break;
            default:
                CV_Error(Error::StsUnsupportedFormat, "Unsupported Mat depth");
        }

        ndarray_obj_t* ndarray = ndarray_new_dense_ndarray(ndim, _sizes, typenum);
        if(!ndarray)
        {
            gc_collect();
            ndarray = ndarray_new_dense_ndarray(ndim, _sizes, typenum);
        }
        if(!ndarray)
            return nullptr;

        return allocate(ndarray, dims0, sizes, type, step);
    }

    bool allocate(UMatData* u, AccessFlag accessFlags, UMatUsageFlags usageFlags) const override
    {
        (void)u; (void)accessFlags; (void)usageFlags;
        return false;
    }

    void deallocate(UMatData* u) const override
    {
        if(!u)
            return;
        if(u->refcount == 0)
        {
            u->data = nullptr;
            u->origdata = nullptr;
            u->userdata = nullptr;
            delete u;
        }
    }

private:
    const MatAllocator* stdAllocator;
};

static NumpyAllocator g_numpyAllocator;

MatAllocator* GetNumpyAllocator()
{
    return &g_numpyAllocator;
}

UMatData* NumpyAllocator_AllocateNdarray(ndarray_obj_t* ndarray, int dims, const int* sizes, int type, size_t* step)
{
    return g_numpyAllocator.allocate(ndarray, dims, sizes, type, step);
}
