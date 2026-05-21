#ifndef CV_UPY_NUMPY_H
#define CV_UPY_NUMPY_H

#include <opencv2/core/mat.hpp>

#ifdef __cplusplus
extern "C" {
#endif
#include "ndarray.h"
#ifdef __cplusplus
}
#endif

cv::MatAllocator* GetNumpyAllocator();

// Helper to call NumpyAllocator's ndarray-aware allocate through base pointer
cv::UMatData* NumpyAllocator_AllocateNdarray(ndarray_obj_t* ndarray, int dims, const int* sizes, int type, size_t* step);

#endif // CV_UPY_NUMPY_H
