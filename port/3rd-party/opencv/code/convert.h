#ifndef CV_UPY_CONVERT_H
#define CV_UPY_CONVERT_H

#include <opencv2/core/mat.hpp>

#ifdef __cplusplus
extern "C" {
#endif
#include "py/runtime.h"
#include "ndarray.h"
#include "ulab.h"
#ifdef __cplusplus
}
#endif

cv::Mat mp_obj_to_mat(mp_obj_t obj);
mp_obj_t mat_to_mp_obj(cv::Mat &mat);
ndarray_obj_t *mat_to_ndarray(cv::Mat &mat);
cv::Mat ndarray_to_mat(ndarray_obj_t *ndarray);

cv::Size mp_obj_to_size(mp_obj_t obj);
cv::Scalar mp_obj_to_scalar(mp_obj_t obj);
cv::Point mp_obj_to_point(mp_obj_t obj);

uint8_t ndarray_type_to_mat_depth(uint8_t dtype);
uint8_t mat_depth_to_ndarray_type(int depth);

#endif // CV_UPY_CONVERT_H
