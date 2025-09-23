/* Copyright (c) 2023, Canaan Bright Sight Co., Ltd
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 * 1. Redistributions of source code must retain the above copyright
 * notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 * notice, this list of conditions and the following disclaimer in the
 * documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND
 * CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES,
 * INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
 * BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
 * NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include "py/compile.h"
#include "py/runtime.h"
#include "py/binary.h"
#include "py/obj.h"
#include "ndarray.h"
#include "ulab.h"
#include "cv_lite.h"
#include "cv_lite_type.h"
#include "cv_lite_wrap.h"

//*****************************for cv*****************************
STATIC mp_obj_t cv_lite_grayscale_find_blobs(size_t n_args, const mp_obj_t *args)
{
    mp_obj_list_t *frame_size_mp = MP_OBJ_TO_PTR(args[0]);

    ndarray_obj_t *data = MP_ROM_PTR(args[1]); //hwc
    uint8_t* img_data = data->array;

    int threshold_min = mp_obj_get_int(args[2]);
    int threshold_max = mp_obj_get_int(args[3]);

    FrameCHWSize frame_shape;
    frame_shape.height = mp_obj_get_int(frame_size_mp->items[0]);
    frame_shape.width = mp_obj_get_int(frame_size_mp->items[1]);
    frame_shape.channel = mp_obj_get_int(1);
    int min_area = mp_obj_get_int(args[4]);
    int kernel_size = mp_obj_get_int(args[5]);

    int ret_num;
    int* ret=grayscale_find_blobs(frame_shape,img_data,threshold_min,threshold_max,min_area,kernel_size,&ret_num);

    // mp_obj_list_t *int_array = mp_obj_new_list(0, NULL);

    // for (int i = 0; i < ret_num; i++)
    // {
    //     mp_obj_list_append(int_array, mp_obj_new_int(ret[i*4+0]));
    //     mp_obj_list_append(int_array, mp_obj_new_int(ret[i*4+1]));
    //     mp_obj_list_append(int_array, mp_obj_new_int(ret[i*4+2]));
    //     mp_obj_list_append(int_array, mp_obj_new_int(ret[i*4+3]));
    // }
    // free(ret);
    // return MP_OBJ_FROM_PTR(int_array);

    mp_obj_t *items = m_new(mp_obj_t, ret_num * 4);
    for (int i = 0; i < ret_num; ++i) {
        items[i * 4 + 0] = mp_obj_new_int(ret[i * 4 + 0]);
        items[i * 4 + 1] = mp_obj_new_int(ret[i * 4 + 1]);
        items[i * 4 + 2] = mp_obj_new_int(ret[i * 4 + 2]);
        items[i * 4 + 3] = mp_obj_new_int(ret[i * 4 + 3]);
    }
    mp_obj_t list_obj = mp_obj_new_list(ret_num * 4, items);
    free(ret);
    return list_obj;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_VAR_BETWEEN(cv_lite_grayscale_find_blobs_obj, 6, 6, cv_lite_grayscale_find_blobs);

STATIC mp_obj_t cv_lite_rgb888_find_blobs(size_t n_args, const mp_obj_t *args)
{
    mp_obj_list_t *frame_size_mp = MP_OBJ_TO_PTR(args[0]);

    ndarray_obj_t *data = MP_ROM_PTR(args[1]); //hwc
    uint8_t* img_data = data->array;

    mp_obj_list_t* threshold_list = MP_OBJ_TO_PTR(args[2]);

    FrameCHWSize frame_shape;
    frame_shape.height = mp_obj_get_int(frame_size_mp->items[0]);
    frame_shape.width = mp_obj_get_int(frame_size_mp->items[1]);
    frame_shape.channel = mp_obj_get_int(3);
    int threshold[6];
    threshold[0] = mp_obj_get_int(threshold_list->items[0]);
    threshold[1] = mp_obj_get_int(threshold_list->items[1]);
    threshold[2] = mp_obj_get_int(threshold_list->items[2]);
    threshold[3] = mp_obj_get_int(threshold_list->items[3]);
    threshold[4] = mp_obj_get_int(threshold_list->items[4]);
    threshold[5] = mp_obj_get_int(threshold_list->items[5]);

    int min_area = mp_obj_get_int(args[3]);
    int kernel_size = mp_obj_get_int(args[4]);

    int ret_num;
    int* ret=rgb888_find_blobs(frame_shape,img_data,threshold,min_area,kernel_size,&ret_num);

    // mp_obj_list_t *int_array = mp_obj_new_list(0, NULL);

    // for (int i = 0; i < ret_num; i++)
    // {
    //     mp_obj_list_append(int_array, mp_obj_new_int(ret[i*4+0]));
    //     mp_obj_list_append(int_array, mp_obj_new_int(ret[i*4+1]));
    //     mp_obj_list_append(int_array, mp_obj_new_int(ret[i*4+2]));
    //     mp_obj_list_append(int_array, mp_obj_new_int(ret[i*4+3]));
    // }
    // free(ret);
    // return MP_OBJ_FROM_PTR(int_array);

    mp_obj_t *items = m_new(mp_obj_t, ret_num * 4);
    for (int i = 0; i < ret_num; ++i) {
        items[i * 4 + 0] = mp_obj_new_int(ret[i * 4 + 0]);
        items[i * 4 + 1] = mp_obj_new_int(ret[i * 4 + 1]);
        items[i * 4 + 2] = mp_obj_new_int(ret[i * 4 + 2]);
        items[i * 4 + 3] = mp_obj_new_int(ret[i * 4 + 3]);
    }
    mp_obj_t list_obj = mp_obj_new_list(ret_num * 4, items);
    free(ret);
    return list_obj;

}
STATIC MP_DEFINE_CONST_FUN_OBJ_VAR_BETWEEN(cv_lite_rgb888_find_blobs_obj, 5, 5, cv_lite_rgb888_find_blobs);

STATIC mp_obj_t cv_lite_grayscale_find_circles(size_t n_args, const mp_obj_t *args)
{
    // 参数解析
    mp_obj_list_t *frame_size_mp = MP_OBJ_TO_PTR(args[0]);
    ndarray_obj_t *data = MP_ROM_PTR(args[1]); // hwc
    uint8_t *img_data = data->array;

    int dp        = mp_obj_get_int(args[2]);
    int minDist   = mp_obj_get_int(args[3]);
    int param1    = mp_obj_get_int(args[4]);
    int param2    = mp_obj_get_int(args[5]);
    int minRadius = mp_obj_get_int(args[6]);
    int maxRadius = mp_obj_get_int(args[7]);

    // 构造图像尺寸
    FrameCHWSize frame_shape;
    frame_shape.height = mp_obj_get_int(frame_size_mp->items[0]);
    frame_shape.width  = mp_obj_get_int(frame_size_mp->items[1]);
    frame_shape.channel = 1;

    // 调用底层圆检测函数
    int ret_num;
    int *ret = grayscale_find_circles(
        frame_shape, img_data, dp, minDist, param1, param2, minRadius, maxRadius, &ret_num
    );

    // 返回 Python 列表，每个圆返回 x, y, r
    // mp_obj_list_t *int_array = mp_obj_new_list(0, NULL);
    // for (int i = 0; i < ret_num; ++i) {
    //     mp_obj_list_append(int_array, mp_obj_new_int(ret[i * 3 + 0])); // x
    //     mp_obj_list_append(int_array, mp_obj_new_int(ret[i * 3 + 1])); // y
    //     mp_obj_list_append(int_array, mp_obj_new_int(ret[i * 3 + 2])); // r
    // }

    // free(ret);
    // return MP_OBJ_FROM_PTR(int_array);
    mp_obj_t *items = m_new(mp_obj_t, ret_num * 3);
    for (int i = 0; i < ret_num; ++i) {
        items[i * 3 + 0] = mp_obj_new_int(ret[i * 3 + 0]);
        items[i * 3 + 1] = mp_obj_new_int(ret[i * 3 + 1]);
        items[i * 3 + 2] = mp_obj_new_int(ret[i * 3 + 2]);
    }
    mp_obj_t list_obj = mp_obj_new_list(ret_num * 3, items);
    free(ret);
    return list_obj;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_VAR_BETWEEN(cv_lite_grayscale_find_circles_obj, 8, 8, cv_lite_grayscale_find_circles);

STATIC mp_obj_t cv_lite_rgb888_find_circles(size_t n_args, const mp_obj_t *args)
{
    // 参数解析
    mp_obj_list_t *frame_size_mp = MP_OBJ_TO_PTR(args[0]);
    ndarray_obj_t *data = MP_ROM_PTR(args[1]); // hwc
    uint8_t *img_data = data->array;

    int dp        = mp_obj_get_int(args[2]);
    int minDist   = mp_obj_get_int(args[3]);
    int param1    = mp_obj_get_int(args[4]);
    int param2    = mp_obj_get_int(args[5]);
    int minRadius = mp_obj_get_int(args[6]);
    int maxRadius = mp_obj_get_int(args[7]);

    // 构造图像尺寸
    FrameCHWSize frame_shape;
    frame_shape.height = mp_obj_get_int(frame_size_mp->items[0]);
    frame_shape.width  = mp_obj_get_int(frame_size_mp->items[1]);
    frame_shape.channel = 3;

    // 调用底层圆检测函数
    int ret_num;
    int *ret = rgb888_find_circles(
        frame_shape, img_data, dp, minDist, param1, param2, minRadius, maxRadius, &ret_num
    );

    // 返回 Python 列表，每个圆返回 x, y, r
    // mp_obj_list_t *int_array = mp_obj_new_list(0, NULL);
    // for (int i = 0; i < ret_num; ++i) {
    //     mp_obj_list_append(int_array, mp_obj_new_int(ret[i * 3 + 0])); // x
    //     mp_obj_list_append(int_array, mp_obj_new_int(ret[i * 3 + 1])); // y
    //     mp_obj_list_append(int_array, mp_obj_new_int(ret[i * 3 + 2])); // r
    // }

    // free(ret);
    // return MP_OBJ_FROM_PTR(int_array);
    mp_obj_t *items = m_new(mp_obj_t, ret_num * 3);
    for (int i = 0; i < ret_num; ++i) {
        items[i * 3 + 0] = mp_obj_new_int(ret[i * 3 + 0]);
        items[i * 3 + 1] = mp_obj_new_int(ret[i * 3 + 1]);
        items[i * 3 + 2] = mp_obj_new_int(ret[i * 3 + 2]);
    }
    mp_obj_t list_obj = mp_obj_new_list(ret_num * 3, items);
    free(ret);
    return list_obj;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_VAR_BETWEEN(cv_lite_rgb888_find_circles_obj, 8, 8, cv_lite_rgb888_find_circles);

STATIC mp_obj_t cv_lite_grayscale_find_rectangles(size_t n_args, const mp_obj_t *args) 
{
    // 参数解析
    mp_obj_list_t *frame_size_mp = MP_OBJ_TO_PTR(args[0]);  // [height, width]
    ndarray_obj_t *data = MP_ROM_PTR(args[1]);              // 图像数据，HWC 或 HW1
    uint8_t *img_data = data->array;

    FrameCHWSize frame_shape;
    frame_shape.height = mp_obj_get_int(frame_size_mp->items[0]);
    frame_shape.width  = mp_obj_get_int(frame_size_mp->items[1]);
    frame_shape.channel = 1; // 灰度图

    // 解析剩余参数
    int canny_thresh1         = mp_obj_get_int(args[2]);
    int canny_thresh2         = mp_obj_get_int(args[3]);
    float approx_eps_ratio   = mp_obj_get_float(args[4]);
    float area_min_ratio     = mp_obj_get_float(args[5]);
    float max_angle_cos      = mp_obj_get_float(args[6]);
    int gaussian_blur_size    = mp_obj_get_int(args[7]);

    // 调用 C++ 检测函数
    int ret_num = 0;
    int *ret = grayscale_find_rectangles(
        frame_shape, img_data,
        canny_thresh1, canny_thresh2,
        approx_eps_ratio,
        area_min_ratio,
        max_angle_cos,
        gaussian_blur_size,
        &ret_num
    );

    // 构造返回 Python 列表 [x0, y0, w0, h0, x1, y1, w1, h1, ...]
    // mp_obj_list_t *int_array = mp_obj_new_list(0, NULL);
    // for (int i = 0; i < ret_num; ++i) {
    //     mp_obj_list_append(int_array, mp_obj_new_int(ret[i * 4 + 0])); // x
    //     mp_obj_list_append(int_array, mp_obj_new_int(ret[i * 4 + 1])); // y
    //     mp_obj_list_append(int_array, mp_obj_new_int(ret[i * 4 + 2])); // w
    //     mp_obj_list_append(int_array, mp_obj_new_int(ret[i * 4 + 3])); // h
    // }

    // free(ret);
    // return MP_OBJ_FROM_PTR(int_array);
    mp_obj_t *items = m_new(mp_obj_t, ret_num * 4);
    for (int i = 0; i < ret_num; ++i) {
        items[i * 4 + 0] = mp_obj_new_int(ret[i * 4 + 0]);
        items[i * 4 + 1] = mp_obj_new_int(ret[i * 4 + 1]);
        items[i * 4 + 2] = mp_obj_new_int(ret[i * 4 + 2]);
        items[i * 4 + 3] = mp_obj_new_int(ret[i * 4 + 3]);
    }
    mp_obj_t list_obj = mp_obj_new_list(ret_num * 4, items);
    free(ret);
    return list_obj;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_VAR_BETWEEN(cv_lite_grayscale_find_rectangles_obj, 8, 8, cv_lite_grayscale_find_rectangles);

STATIC mp_obj_t cv_lite_grayscale_find_rectangles_with_corners(size_t n_args, const mp_obj_t *args) 
{
    // 参数解析
    mp_obj_list_t *frame_size_mp = MP_OBJ_TO_PTR(args[0]);  // [height, width]
    ndarray_obj_t *data = MP_ROM_PTR(args[1]);              // 图像数据，HWC 或 HW1
    uint8_t *img_data = data->array;

    FrameCHWSize frame_shape;
    frame_shape.height = mp_obj_get_int(frame_size_mp->items[0]);
    frame_shape.width  = mp_obj_get_int(frame_size_mp->items[1]);
    frame_shape.channel = 1; // 灰度图

    // 解析剩余参数
    int canny_thresh1         = mp_obj_get_int(args[2]);
    int canny_thresh2         = mp_obj_get_int(args[3]);
    float approx_eps_ratio   = mp_obj_get_float(args[4]);
    float area_min_ratio     = mp_obj_get_float(args[5]);
    float max_angle_cos      = mp_obj_get_float(args[6]);
    int gaussian_blur_size    = mp_obj_get_int(args[7]);

    // 调用 C++ 检测函数
    int ret_num = 0;
    int *ret = grayscale_find_rectangles_with_corners(
        frame_shape, img_data,
        canny_thresh1, canny_thresh2,
        approx_eps_ratio,
        area_min_ratio,
        max_angle_cos,
        gaussian_blur_size,
        &ret_num
    );
    mp_obj_list_t *results_mp_list = mp_obj_new_list(0, NULL);

    for (int i = 0; i < ret_num; i++)
    {
        mp_obj_list_t *results_mp_list_rect = mp_obj_new_list(0, NULL);
        mp_obj_list_append(results_mp_list_rect, mp_obj_new_int(ret[i * 12 + 0]));
        mp_obj_list_append(results_mp_list_rect, mp_obj_new_int(ret[i * 12 + 1]));
        mp_obj_list_append(results_mp_list_rect, mp_obj_new_int(ret[i * 12 + 2]));
        mp_obj_list_append(results_mp_list_rect, mp_obj_new_int(ret[i * 12 + 3]));
        for (int j = 0; j < 4; j++)
        {
            mp_obj_list_append(results_mp_list_rect, mp_obj_new_int(ret[i * 12 + j * 2 + 4]));
            mp_obj_list_append(results_mp_list_rect, mp_obj_new_int(ret[i * 12 + j * 2 + 5]));
        }
        mp_obj_list_append(results_mp_list, results_mp_list_rect);
    }
    free(ret);
    return MP_OBJ_FROM_PTR(results_mp_list);
}
STATIC MP_DEFINE_CONST_FUN_OBJ_VAR_BETWEEN(cv_lite_grayscale_find_rectangles_with_corners_obj, 8, 8, cv_lite_grayscale_find_rectangles_with_corners);

STATIC mp_obj_t cv_lite_rgb888_find_rectangles_with_corners(size_t n_args, const mp_obj_t *args) 
{
    // 参数解析
    mp_obj_list_t *frame_size_mp = MP_OBJ_TO_PTR(args[0]);  // [height, width]
    ndarray_obj_t *data = MP_ROM_PTR(args[1]);              // 图像数据，HWC 或 HW1
    uint8_t *img_data = data->array;

    FrameCHWSize frame_shape;
    frame_shape.height = mp_obj_get_int(frame_size_mp->items[0]);
    frame_shape.width  = mp_obj_get_int(frame_size_mp->items[1]);
    frame_shape.channel = 3; // RGB888

    // 解析剩余参数
    int canny_thresh1         = mp_obj_get_int(args[2]);
    int canny_thresh2         = mp_obj_get_int(args[3]);
    float approx_eps_ratio   = mp_obj_get_float(args[4]);
    float area_min_ratio     = mp_obj_get_float(args[5]);
    float max_angle_cos      = mp_obj_get_float(args[6]);
    int gaussian_blur_size    = mp_obj_get_int(args[7]);

    // 调用 C++ 检测函数
    int ret_num = 0;
    int *ret = rgb888_find_rectangles_with_corners(
        frame_shape, img_data,
        canny_thresh1, canny_thresh2,
        approx_eps_ratio,
        area_min_ratio,
        max_angle_cos,
        gaussian_blur_size,
        &ret_num
    );
    mp_obj_list_t *results_mp_list = mp_obj_new_list(0, NULL);

    for (int i = 0; i < ret_num; i++)
    {
        mp_obj_list_t *results_mp_list_rect = mp_obj_new_list(0, NULL);
        mp_obj_list_append(results_mp_list_rect, mp_obj_new_int(ret[i * 12 + 0]));
        mp_obj_list_append(results_mp_list_rect, mp_obj_new_int(ret[i * 12 + 1]));
        mp_obj_list_append(results_mp_list_rect, mp_obj_new_int(ret[i * 12 + 2]));
        mp_obj_list_append(results_mp_list_rect, mp_obj_new_int(ret[i * 12 + 3]));
        for (int j = 0; j < 4; j++)
        {
            mp_obj_list_append(results_mp_list_rect, mp_obj_new_int(ret[i * 12 + j * 2 + 4]));
            mp_obj_list_append(results_mp_list_rect, mp_obj_new_int(ret[i * 12 + j * 2 + 5]));
        }
        mp_obj_list_append(results_mp_list, results_mp_list_rect);
    }
    free(ret);
    return MP_OBJ_FROM_PTR(results_mp_list);
}
STATIC MP_DEFINE_CONST_FUN_OBJ_VAR_BETWEEN(cv_lite_rgb888_find_rectangles_with_corners_obj, 8, 8, cv_lite_rgb888_find_rectangles_with_corners);

STATIC mp_obj_t cv_lite_rgb888_find_rectangles(size_t n_args, const mp_obj_t *args)
{
    // 参数解析
    mp_obj_list_t *frame_size_mp = MP_OBJ_TO_PTR(args[0]);  // [height, width]
    ndarray_obj_t *data = MP_ROM_PTR(args[1]);              // 图像数据，HWC
    uint8_t *img_data = data->array;

    FrameCHWSize frame_shape;
    frame_shape.height = mp_obj_get_int(frame_size_mp->items[0]);
    frame_shape.width  = mp_obj_get_int(frame_size_mp->items[1]);
    frame_shape.channel = 3; // RGB888

    // 解析剩余参数
    int canny_thresh1         = mp_obj_get_int(args[2]);
    int canny_thresh2         = mp_obj_get_int(args[3]);
    float approx_eps_ratio   = mp_obj_get_float(args[4]);
    float area_min_ratio     = mp_obj_get_float(args[5]);
    float max_angle_cos      = mp_obj_get_float(args[6]);
    int gaussian_blur_size    = mp_obj_get_int(args[7]);

    // 调用 C++ 检测函数
    int ret_num = 0;
    int *ret = rgb888_find_rectangles(
        frame_shape, img_data,
        canny_thresh1, canny_thresh2,
        approx_eps_ratio,
        area_min_ratio,
        max_angle_cos,
        gaussian_blur_size,
        &ret_num
    );

    // 构造返回 Python 列表 [x0, y0, w0, h0, x1, y1, w1, h1, ...]
    mp_obj_t *items = m_new(mp_obj_t, ret_num * 4);
    for (int i = 0; i < ret_num; ++i) {
        items[i * 4 + 0] = mp_obj_new_int(ret[i * 4 + 0]);
        items[i * 4 + 1] = mp_obj_new_int(ret[i * 4 + 1]);
        items[i * 4 + 2] = mp_obj_new_int(ret[i * 4 + 2]);
        items[i * 4 + 3] = mp_obj_new_int(ret[i * 4 + 3]);
    }
    mp_obj_t list_obj = mp_obj_new_list(ret_num * 4, items);
    free(ret);
    return list_obj;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_VAR_BETWEEN(cv_lite_rgb888_find_rectangles_obj, 8, 8, cv_lite_rgb888_find_rectangles);


STATIC mp_obj_t cv_lite_grayscale_find_lines_raw(size_t n_args, const mp_obj_t *args) {
    // 1. 参数解析
    mp_obj_list_t *frame_size_mp = MP_OBJ_TO_PTR(args[0]);  // [height, width]
    ndarray_obj_t *data = MP_ROM_PTR(args[1]);              // 图像数据
    uint8_t *img_data = data->array;

    FrameCHWSize frame_shape;
    frame_shape.height = mp_obj_get_int(frame_size_mp->items[0]);
    frame_shape.width  = mp_obj_get_int(frame_size_mp->items[1]);
    frame_shape.channel = 1;

    int x_stride     = mp_obj_get_int(args[2]);
    int y_stride     = mp_obj_get_int(args[3]);
    int sobel_thresh = mp_obj_get_int(args[4]);
    float rho_step   = mp_obj_get_float(args[5]);
    float theta_step = mp_obj_get_float(args[6]);
    int hough_thresh = mp_obj_get_int(args[7]);

    // 2. 调用 C 实现
    int ret_num = 0;
    int *ret = grayscale_find_lines_raw(frame_shape, img_data,
                                        x_stride, y_stride,
                                        sobel_thresh, rho_step, theta_step,
                                        hough_thresh, &ret_num);

    if (!ret || ret_num <= 0) return mp_obj_new_list(0, NULL);

    // 3. 转为 MicroPython list
    mp_obj_t *items = m_new(mp_obj_t, ret_num * 4);
    for (int i = 0; i < ret_num; ++i) {
        items[i * 4 + 0] = mp_obj_new_int(ret[i * 4 + 0]);
        items[i * 4 + 1] = mp_obj_new_int(ret[i * 4 + 1]);
        items[i * 4 + 2] = mp_obj_new_int(ret[i * 4 + 2]);
        items[i * 4 + 3] = mp_obj_new_int(ret[i * 4 + 3]);
    }
    mp_obj_t list_obj = mp_obj_new_list(ret_num * 4, items);
    free(ret);
    return list_obj;
}
// 注册为 MicroPython 接口
STATIC MP_DEFINE_CONST_FUN_OBJ_VAR_BETWEEN(cv_lite_grayscale_find_lines_raw_obj, 8, 8, cv_lite_grayscale_find_lines_raw);


STATIC mp_obj_t cv_lite_grayscale_find_lines(size_t n_args, const mp_obj_t *args) {
    // 参数解析
    mp_obj_list_t *frame_size_mp = MP_OBJ_TO_PTR(args[0]);  // [height, width]
    ndarray_obj_t *data = MP_ROM_PTR(args[1]);              // 图像数据
    uint8_t *img_data = data->array;

    FrameCHWSize frame_shape;
    frame_shape.height = mp_obj_get_int(frame_size_mp->items[0]);
    frame_shape.width  = mp_obj_get_int(frame_size_mp->items[1]);
    frame_shape.channel = 1;

    // 参数解析
    int canny_thresh1        = mp_obj_get_int(args[2]);
    int canny_thresh2        = mp_obj_get_int(args[3]);
    int gaussian_blur_size   = mp_obj_get_int(args[4]);
    float rho                = mp_obj_get_float(args[5]);
    float theta              = mp_obj_get_float(args[6]);
    int hough_thresh         = mp_obj_get_int(args[7]);
    float min_line_length    = mp_obj_get_float(args[8]);
    float max_line_gap       = mp_obj_get_float(args[9]);

    // 调用底层直线查找函数
    int ret_num = 0;
    int *ret = grayscale_find_lines(frame_shape, img_data,canny_thresh1, canny_thresh2,gaussian_blur_size,rho, theta,hough_thresh, min_line_length, max_line_gap,&ret_num);

    // // 构造返回列表 [x1, y1, x2, y2, x1, y1, x2, y2, ...]
    // mp_obj_list_t *int_array = mp_obj_new_list(0, NULL);
    // for (int i = 0; i < ret_num; ++i) {
    //     mp_obj_list_append(int_array, mp_obj_new_int(ret[i * 4 + 0]));
    //     mp_obj_list_append(int_array, mp_obj_new_int(ret[i * 4 + 1]));
    //     mp_obj_list_append(int_array, mp_obj_new_int(ret[i * 4 + 2]));
    //     mp_obj_list_append(int_array, mp_obj_new_int(ret[i * 4 + 3]));
    // }

    // free(ret);
    // return MP_OBJ_FROM_PTR(int_array);

    mp_obj_t *items = m_new(mp_obj_t, ret_num * 4);
    for (int i = 0; i < ret_num; ++i) {
        items[i * 4 + 0] = mp_obj_new_int(ret[i * 4 + 0]);
        items[i * 4 + 1] = mp_obj_new_int(ret[i * 4 + 1]);
        items[i * 4 + 2] = mp_obj_new_int(ret[i * 4 + 2]);
        items[i * 4 + 3] = mp_obj_new_int(ret[i * 4 + 3]);
    }
    mp_obj_t list_obj = mp_obj_new_list(ret_num * 4, items);
    free(ret);
    return list_obj;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_VAR_BETWEEN(cv_lite_grayscale_find_lines_obj, 10, 10, cv_lite_grayscale_find_lines);

STATIC mp_obj_t cv_lite_rgb888_find_lines(size_t n_args, const mp_obj_t *args) {
    // 参数解析
    mp_obj_list_t *frame_size_mp = MP_OBJ_TO_PTR(args[0]);  // [height, width]
    ndarray_obj_t *data = MP_ROM_PTR(args[1]);              // 图像数据
    uint8_t *img_data = data->array;

    FrameCHWSize frame_shape;
    frame_shape.height = mp_obj_get_int(frame_size_mp->items[0]);
    frame_shape.width  = mp_obj_get_int(frame_size_mp->items[1]);
    frame_shape.channel = 3;

    // 参数解析
    int canny_thresh1        = mp_obj_get_int(args[2]);
    int canny_thresh2        = mp_obj_get_int(args[3]);
    int gaussian_blur_size   = mp_obj_get_int(args[4]);
    float rho                = mp_obj_get_float(args[5]);
    float theta              = mp_obj_get_float(args[6]);
    int hough_thresh         = mp_obj_get_int(args[7]);
    float min_line_length    = mp_obj_get_float(args[8]);
    float max_line_gap       = mp_obj_get_float(args[9]);

    // 调用底层直线查找函数
    int ret_num = 0;
    int *ret = rgb888_find_lines(frame_shape, img_data,canny_thresh1, canny_thresh2,gaussian_blur_size,rho, theta,hough_thresh, min_line_length, max_line_gap,&ret_num);

    // 构造返回列表 [x1, y1, x2, y2, x1, y1, x2, y2, ...]
    // mp_obj_list_t *int_array = mp_obj_new_list(0, NULL);
    // for (int i = 0; i < ret_num; ++i) {
    //     mp_obj_list_append(int_array, mp_obj_new_int(ret[i * 4 + 0]));
    //     mp_obj_list_append(int_array, mp_obj_new_int(ret[i * 4 + 1]));
    //     mp_obj_list_append(int_array, mp_obj_new_int(ret[i * 4 + 2]));
    //     mp_obj_list_append(int_array, mp_obj_new_int(ret[i * 4 + 3]));
    // }

    // free(ret);
    // return MP_OBJ_FROM_PTR(int_array);
    mp_obj_t *items = m_new(mp_obj_t, ret_num * 4);
    for (int i = 0; i < ret_num; ++i) {
        items[i * 4 + 0] = mp_obj_new_int(ret[i * 4 + 0]);
        items[i * 4 + 1] = mp_obj_new_int(ret[i * 4 + 1]);
        items[i * 4 + 2] = mp_obj_new_int(ret[i * 4 + 2]);
        items[i * 4 + 3] = mp_obj_new_int(ret[i * 4 + 3]);
    }
    mp_obj_t list_obj = mp_obj_new_list(ret_num * 4, items);
    free(ret);
    return list_obj;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_VAR_BETWEEN(cv_lite_rgb888_find_lines_obj, 10, 10, cv_lite_rgb888_find_lines);

STATIC mp_obj_t cv_lite_grayscale_find_lines_sobel(size_t n_args, const mp_obj_t *args) {
    // 参数解析
    mp_obj_list_t *frame_size_mp = MP_OBJ_TO_PTR(args[0]);  // [height, width]
    ndarray_obj_t *data = MP_ROM_PTR(args[1]);              // 图像数据（灰度图）
    uint8_t *img_data = data->array;

    FrameCHWSize frame_shape;
    frame_shape.height = mp_obj_get_int(frame_size_mp->items[0]);
    frame_shape.width  = mp_obj_get_int(frame_size_mp->items[1]);
    frame_shape.channel = 1;

    // 参数解析
    int sobel_thresh         = mp_obj_get_int(args[2]);  // sobel边缘图阈值
    int gaussian_blur_size   = mp_obj_get_int(args[3]);
    float rho                = mp_obj_get_float(args[4]);
    float theta              = mp_obj_get_float(args[5]);
    int hough_thresh         = mp_obj_get_int(args[6]);
    float min_line_length    = mp_obj_get_float(args[7]);
    float max_line_gap       = mp_obj_get_float(args[8]);

    // 调用底层直线查找函数（使用Sobel替代Canny）
    int ret_num = 0;
    int *ret = grayscale_find_lines_sobel(frame_shape, img_data,
                                          sobel_thresh,
                                          gaussian_blur_size,
                                          rho, theta,
                                          hough_thresh,
                                          min_line_length, max_line_gap,
                                          &ret_num);

    // 构造返回列表 [x1, y1, x2, y2, ...]
    mp_obj_t *items = m_new(mp_obj_t, ret_num * 4);
    for (int i = 0; i < ret_num; ++i) {
        items[i * 4 + 0] = mp_obj_new_int(ret[i * 4 + 0]);
        items[i * 4 + 1] = mp_obj_new_int(ret[i * 4 + 1]);
        items[i * 4 + 2] = mp_obj_new_int(ret[i * 4 + 2]);
        items[i * 4 + 3] = mp_obj_new_int(ret[i * 4 + 3]);
    }
    mp_obj_t list_obj = mp_obj_new_list(ret_num * 4, items);
    free(ret);
    return list_obj;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_VAR_BETWEEN(cv_lite_grayscale_find_lines_sobel_obj, 9, 9, cv_lite_grayscale_find_lines_sobel);


STATIC mp_obj_t cv_lite_grayscale_find_lines_no_hough(size_t n_args, const mp_obj_t *args) {
    // 参数解析
    mp_obj_list_t *frame_size_mp = MP_OBJ_TO_PTR(args[0]);  // [height, width]
    ndarray_obj_t *data = MP_ROM_PTR(args[1]);              // 图像数据
    uint8_t *img_data = data->array;

    FrameCHWSize frame_shape;
    frame_shape.height = mp_obj_get_int(frame_size_mp->items[0]);
    frame_shape.width  = mp_obj_get_int(frame_size_mp->items[1]);
    frame_shape.channel = 1;

    // 参数解析
    int canny_thresh1        = mp_obj_get_int(args[2]);
    int canny_thresh2        = mp_obj_get_int(args[3]);
    int gaussian_blur_size   = mp_obj_get_int(args[4]);
    float min_contour_len    = mp_obj_get_float(args[5]);

    // 调用底层直线查找函数
    int ret_num = 0;
    int *ret = grayscale_find_lines_no_hough(frame_shape, img_data,canny_thresh1, canny_thresh2,gaussian_blur_size,min_contour_len, &ret_num);

    // // 构造返回列表 [x1, y1, x2, y2, x1, y1, x2, y2, ...]
    // mp_obj_list_t *int_array = mp_obj_new_list(0, NULL);
    // for (int i = 0; i < ret_num; ++i) {
    //     mp_obj_list_append(int_array, mp_obj_new_int(ret[i * 4 + 0]));
    //     mp_obj_list_append(int_array, mp_obj_new_int(ret[i * 4 + 1]));
    //     mp_obj_list_append(int_array, mp_obj_new_int(ret[i * 4 + 2]));
    //     mp_obj_list_append(int_array, mp_obj_new_int(ret[i * 4 + 3]));
    // }

    // free(ret);
    // return MP_OBJ_FROM_PTR(int_array);

    mp_obj_t *items = m_new(mp_obj_t, ret_num * 4);
    for (int i = 0; i < ret_num; ++i) {
        items[i * 4 + 0] = mp_obj_new_int(ret[i * 4 + 0]);
        items[i * 4 + 1] = mp_obj_new_int(ret[i * 4 + 1]);
        items[i * 4 + 2] = mp_obj_new_int(ret[i * 4 + 2]);
        items[i * 4 + 3] = mp_obj_new_int(ret[i * 4 + 3]);
    }
    mp_obj_t list_obj = mp_obj_new_list(ret_num * 4, items);
    free(ret);
    return list_obj;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_VAR_BETWEEN(cv_lite_grayscale_find_lines_no_hough_obj, 6, 6, cv_lite_grayscale_find_lines_no_hough);

STATIC mp_obj_t cv_lite_grayscale_find_edges(size_t n_args, const mp_obj_t *args)
{
    mp_obj_list_t *frame_size_mp = MP_OBJ_TO_PTR(args[0]);
    ndarray_obj_t *data = MP_ROM_PTR(args[1]); // hwc
    uint8_t *img_data = data->array;

    // 构造图像尺寸
    FrameCHWSize frame_shape;
    frame_shape.height = mp_obj_get_int(frame_size_mp->items[0]);
    frame_shape.width  = mp_obj_get_int(frame_size_mp->items[1]);
    frame_shape.channel = 1;

    // 阈值
    int threshold1 = mp_obj_get_int(args[2]);
    int threshold2 = mp_obj_get_int(args[3]);

    uint8_t* result=(uint8_t*)malloc(frame_shape.width*frame_shape.height*frame_shape.channel);
    // 处理：边缘检测
    grayscale_find_edges(frame_shape, img_data, threshold1, threshold2,result);

    // 构造返回的 numpy 对象（共享内存，不复制）
    size_t ndarray_shape[4];
    ndarray_shape[1] = frame_shape.height;
    ndarray_shape[2] = frame_shape.width;
    ndarray_shape[3] = frame_shape.channel;
    ndarray_obj_t *out = ndarray_new_ndarray(3, ndarray_shape, NULL, NDARRAY_UINT8);
    uint8_t *out_data = (uint8_t *)out->array;
    memcpy(out_data, result, frame_shape.width * frame_shape.height*frame_shape.channel); // 独立返回一份数据
    free(result);
    return MP_OBJ_FROM_PTR(out);
}
STATIC MP_DEFINE_CONST_FUN_OBJ_VAR_BETWEEN(cv_lite_grayscale_find_edges_obj, 4, 4, cv_lite_grayscale_find_edges);

STATIC mp_obj_t cv_lite_rgb888_find_edges(size_t n_args, const mp_obj_t *args)
{
    mp_obj_list_t *frame_size_mp = MP_OBJ_TO_PTR(args[0]);
    ndarray_obj_t *data = MP_ROM_PTR(args[1]); // hwc
    uint8_t *img_data = data->array;

    // 构造图像尺寸
    FrameCHWSize frame_shape;
    frame_shape.height = mp_obj_get_int(frame_size_mp->items[0]);
    frame_shape.width  = mp_obj_get_int(frame_size_mp->items[1]);
    frame_shape.channel = 3;

    // 阈值
    int threshold1 = mp_obj_get_int(args[2]);
    int threshold2 = mp_obj_get_int(args[3]);

    uint8_t* result=(uint8_t*)malloc(frame_shape.width*frame_shape.height);
    // 处理：边缘检测
    rgb888_find_edges(frame_shape, img_data, threshold1, threshold2,result);

    // 构造返回的 numpy 对象（共享内存，不复制）
    size_t ndarray_shape[4];
    ndarray_shape[1] = frame_shape.height;
    ndarray_shape[2] = frame_shape.width;
    ndarray_shape[3] = 1;
    ndarray_obj_t *out = ndarray_new_ndarray(3, ndarray_shape, NULL, NDARRAY_UINT8);
    uint8_t *out_data = (uint8_t *)out->array;
    memcpy(out_data, result, frame_shape.width * frame_shape.height); // 独立返回一份数据
    free(result);
    return MP_OBJ_FROM_PTR(out);
}
STATIC MP_DEFINE_CONST_FUN_OBJ_VAR_BETWEEN(cv_lite_rgb888_find_edges_obj, 4, 4, cv_lite_rgb888_find_edges);

STATIC mp_obj_t cv_lite_grayscale_threshold_binary(size_t n_args, const mp_obj_t *args)
{
    mp_obj_list_t *frame_size_mp = MP_OBJ_TO_PTR(args[0]);
    ndarray_obj_t *data = MP_ROM_PTR(args[1]); // hwc
    uint8_t *img_data = data->array;

    // 构造图像尺寸
    FrameCHWSize frame_shape;
    frame_shape.height = mp_obj_get_int(frame_size_mp->items[0]);
    frame_shape.width  = mp_obj_get_int(frame_size_mp->items[1]);
    frame_shape.channel = 1;
    
    // 阈值
    int thresh = mp_obj_get_int(args[2]);
    int maxval = mp_obj_get_int(args[3]);

    uint8_t* result=(uint8_t*)malloc(frame_shape.width*frame_shape.height);
    // 处理：二值化
    grayscale_threshold_binary(frame_shape, img_data, thresh, maxval,result);

    // 构造返回的 numpy 对象（共享内存，不复制）
    size_t ndarray_shape[4];

    ndarray_shape[1] = frame_shape.height;
    ndarray_shape[2] = frame_shape.width;
    ndarray_shape[3] = 1;
    ndarray_obj_t *out = ndarray_new_ndarray(3, ndarray_shape, NULL, NDARRAY_UINT8);
    uint8_t *out_data = (uint8_t *)out->array;
    memcpy(out_data, result, frame_shape.width * frame_shape.height); // 独立返回一份数据
    free(result);
    return MP_OBJ_FROM_PTR(out);
}
STATIC MP_DEFINE_CONST_FUN_OBJ_VAR_BETWEEN(cv_lite_grayscale_threshold_binary_obj, 4, 4, cv_lite_grayscale_threshold_binary);

STATIC mp_obj_t cv_lite_rgb888_threshold_binary(size_t n_args, const mp_obj_t *args)
{
    mp_obj_list_t *frame_size_mp = MP_OBJ_TO_PTR(args[0]);
    ndarray_obj_t *data = MP_ROM_PTR(args[1]); // hwc
    uint8_t *img_data = data->array;

    // 构造图像尺寸
    FrameCHWSize frame_shape;
    frame_shape.height = mp_obj_get_int(frame_size_mp->items[0]);
    frame_shape.width  = mp_obj_get_int(frame_size_mp->items[1]);
    frame_shape.channel = 3;
    
    // 阈值
    int thresh = mp_obj_get_int(args[2]);
    int maxval = mp_obj_get_int(args[3]);

    uint8_t* result=(uint8_t*)malloc(frame_shape.width*frame_shape.height);
    // 处理：二值化
    rgb888_threshold_binary(frame_shape, img_data, thresh, maxval,result);

    // 构造返回的 numpy 对象（共享内存，不复制）
    size_t ndarray_shape[4];

    ndarray_shape[1] = frame_shape.height;
    ndarray_shape[2] = frame_shape.width;
    ndarray_shape[3] = 1;
    ndarray_obj_t *out = ndarray_new_ndarray(3, ndarray_shape, NULL, NDARRAY_UINT8);
    uint8_t *out_data = (uint8_t *)out->array;
    memcpy(out_data, result, frame_shape.width * frame_shape.height); // 独立返回一份数据
    free(result);
    return MP_OBJ_FROM_PTR(out);
}
STATIC MP_DEFINE_CONST_FUN_OBJ_VAR_BETWEEN(cv_lite_rgb888_threshold_binary_obj, 4, 4, cv_lite_rgb888_threshold_binary);

STATIC mp_obj_t cv_lite_rgb888_white_balance_gray_world(size_t n_args, const mp_obj_t *args)
{
    mp_obj_list_t *frame_size_mp = MP_OBJ_TO_PTR(args[0]);
    ndarray_obj_t *data = MP_ROM_PTR(args[1]); // hwc
    uint8_t *img_data = data->array;

    // 构造图像尺寸
    FrameCHWSize frame_shape;
    frame_shape.height = mp_obj_get_int(frame_size_mp->items[0]);
    frame_shape.width  = mp_obj_get_int(frame_size_mp->items[1]);
    frame_shape.channel = 3;

    uint8_t* result=(uint8_t*)malloc(frame_shape.width*frame_shape.height*frame_shape.channel);
    // 处理：白平衡
    rgb888_white_balance_gray_world(frame_shape, img_data, result);
    // 构造返回的 numpy 对象（共享内存，不复制）
    size_t ndarray_shape[4];
    ndarray_shape[1] = frame_shape.height;
    ndarray_shape[2] = frame_shape.width;
    ndarray_shape[3] = frame_shape.channel;
    ndarray_obj_t *out = ndarray_new_ndarray(3, ndarray_shape, NULL, NDARRAY_UINT8);
    uint8_t *out_data = (uint8_t *)out->array;
    memcpy(out_data, result, frame_shape.width * frame_shape.height * frame_shape.channel); // 独立返回一份数据
    free(result);
    return MP_OBJ_FROM_PTR(out);
}
STATIC MP_DEFINE_CONST_FUN_OBJ_VAR_BETWEEN(cv_lite_rgb888_white_balance_gray_world_obj, 2, 2, cv_lite_rgb888_white_balance_gray_world);

STATIC mp_obj_t cv_lite_rgb888_white_balance_gray_world_fast(size_t n_args, const mp_obj_t *args)
{
    mp_obj_list_t *frame_size_mp = MP_OBJ_TO_PTR(args[0]);
    ndarray_obj_t *data = MP_ROM_PTR(args[1]); // hwc
    uint8_t *img_data = data->array;

    // 构造图像尺寸
    FrameCHWSize frame_shape;
    frame_shape.height = mp_obj_get_int(frame_size_mp->items[0]);
    frame_shape.width  = mp_obj_get_int(frame_size_mp->items[1]);
    frame_shape.channel = 3;

    uint8_t* result=(uint8_t*)malloc(frame_shape.width*frame_shape.height*frame_shape.channel);
    // 处理：白平衡
    rgb888_white_balance_gray_world_fast(frame_shape, img_data, result);
    // 构造返回的 numpy 对象（共享内存，不复制）
    size_t ndarray_shape[4];
    ndarray_shape[1] = frame_shape.height;
    ndarray_shape[2] = frame_shape.width;
    ndarray_shape[3] = frame_shape.channel;
    ndarray_obj_t *out = ndarray_new_ndarray(3, ndarray_shape, NULL, NDARRAY_UINT8);
    uint8_t *out_data = (uint8_t *)out->array;
    memcpy(out_data, result, frame_shape.width * frame_shape.height * frame_shape.channel); // 独立返回一份数据
    free(result);
    return MP_OBJ_FROM_PTR(out);
}
STATIC MP_DEFINE_CONST_FUN_OBJ_VAR_BETWEEN(cv_lite_rgb888_white_balance_gray_world_fast_obj, 2, 2, cv_lite_rgb888_white_balance_gray_world_fast);

STATIC mp_obj_t cv_lite_rgb888_white_balance_gray_world_fast_ex(size_t n_args, const mp_obj_t *args)
{
    // 参数解析：
    // args[0] - 图像尺寸 [H, W]
    // args[1] - ndarray 图像数据 (HWC, RGB888)
    // args[2] - gain_clip（可选，float）
    // args[3] - brightness_boost（可选，float）

    mp_obj_list_t *frame_size_mp = MP_OBJ_TO_PTR(args[0]);
    ndarray_obj_t *data = MP_ROM_PTR(args[1]);
    uint8_t *img_data = data->array;

    // 解析图像尺寸
    FrameCHWSize frame_shape;
    frame_shape.height = mp_obj_get_int(frame_size_mp->items[0]);
    frame_shape.width  = mp_obj_get_int(frame_size_mp->items[1]);
    frame_shape.channel = 3;

    // 默认参数
    float gain_clip = 2.5f;
    float brightness_boost = 1.05f;

    // 可选参数覆盖
    if (n_args >= 3) {
        gain_clip = mp_obj_get_float(args[2]);
    }
    if (n_args >= 4) {
        brightness_boost = mp_obj_get_float(args[3]);
    }

    // 分配输出图像内存
    uint8_t* result = (uint8_t*)malloc(frame_shape.width * frame_shape.height * frame_shape.channel);

    // 调用白平衡处理
    rgb888_white_balance_gray_world_fast_ex(frame_shape, img_data, result, gain_clip, brightness_boost);

    // 构造返回 ndarray
    size_t ndarray_shape[4];
    ndarray_shape[1] = frame_shape.height;
    ndarray_shape[2] = frame_shape.width;
    ndarray_shape[3] = frame_shape.channel;
    ndarray_obj_t *out = ndarray_new_ndarray(3, ndarray_shape, NULL, NDARRAY_UINT8);
    uint8_t *out_data = (uint8_t *)out->array;
    memcpy(out_data, result, frame_shape.width * frame_shape.height * frame_shape.channel);
    free(result);

    return MP_OBJ_FROM_PTR(out);
}
// 注册函数对象，支持 2~4 个参数
STATIC MP_DEFINE_CONST_FUN_OBJ_VAR_BETWEEN(cv_lite_rgb888_white_balance_gray_world_fast_ex_obj,2, 4, cv_lite_rgb888_white_balance_gray_world_fast_ex);


STATIC mp_obj_t cv_lite_rgb888_white_balance_white_patch(size_t n_args, const mp_obj_t *args)
{
    mp_obj_list_t *frame_size_mp = MP_OBJ_TO_PTR(args[0]);
    ndarray_obj_t *data = MP_ROM_PTR(args[1]); // hwc
    uint8_t *img_data = data->array;

    // 构造图像尺寸
    FrameCHWSize frame_shape;
    frame_shape.height = mp_obj_get_int(frame_size_mp->items[0]);
    frame_shape.width  = mp_obj_get_int(frame_size_mp->items[1]);
    frame_shape.channel = 3;

    uint8_t* result=(uint8_t*)malloc(frame_shape.width*frame_shape.height*frame_shape.channel);
    // 处理：白平衡
    rgb888_white_balance_white_patch(frame_shape, img_data, result);
    // 构造返回的 numpy 对象（共享内存，不复制）
    size_t ndarray_shape[4];
    ndarray_shape[1] = frame_shape.height;
    ndarray_shape[2] = frame_shape.width;
    ndarray_shape[3] = frame_shape.channel;
    ndarray_obj_t *out = ndarray_new_ndarray(3, ndarray_shape, NULL, NDARRAY_UINT8);
    uint8_t *out_data = (uint8_t *)out->array;
    memcpy(out_data, result, frame_shape.width * frame_shape.height * frame_shape.channel); // 独立返回一份数据
    free(result);
    return MP_OBJ_FROM_PTR(out);
}
STATIC MP_DEFINE_CONST_FUN_OBJ_VAR_BETWEEN(cv_lite_rgb888_white_balance_white_patch_obj, 2, 2, cv_lite_rgb888_white_balance_white_patch);

// MicroPython 封装接口
STATIC mp_obj_t cv_lite_rgb888_white_balance_white_patch_ex(size_t n_args, const mp_obj_t *args)
{
    // 参数 0: frame_size [height, width]
    mp_obj_list_t *frame_size_mp = MP_OBJ_TO_PTR(args[0]);
    ndarray_obj_t *data = MP_ROM_PTR(args[1]); // RGB888 图像数据
    uint8_t *img_data = data->array;

    // 构造图像尺寸
    FrameCHWSize frame_shape;
    frame_shape.height = mp_obj_get_int(frame_size_mp->items[0]);
    frame_shape.width  = mp_obj_get_int(frame_size_mp->items[1]);
    frame_shape.channel = 3;

    // 参数 2 ~ 4: 可选 top_percent, gain_clip, brightness_boost
    float top_percent = 5.0f;
    float gain_clip = 2.5f;
    float brightness_boost = 1.0f;

    if (n_args > 2) {
        top_percent = mp_obj_get_float(args[2]);
    }
    if (n_args > 3) {
        gain_clip = mp_obj_get_float(args[3]);
    }
    if (n_args > 4) {
        brightness_boost = mp_obj_get_float(args[4]);
    }

    // 输出图像缓冲区
    uint8_t* result = (uint8_t*)malloc(frame_shape.width * frame_shape.height * frame_shape.channel);
    if (!result) {
        mp_raise_msg(&mp_type_MemoryError, MP_ERROR_TEXT("unable to allocate result buffer"));
    }

    // 白平衡处理
    rgb888_white_balance_white_patch_ex(frame_shape, img_data, result,
                                        top_percent, gain_clip, brightness_boost);

    // 构造返回的 ndarray（uint8 HWC）
    size_t shape[4];
    shape[1] = frame_shape.height;
    shape[2] = frame_shape.width;
    shape[3] = frame_shape.channel;

    ndarray_obj_t *out = ndarray_new_ndarray(3, shape, NULL, NDARRAY_UINT8);
    memcpy(out->array, result, frame_shape.width * frame_shape.height * frame_shape.channel);
    free(result);

    return MP_OBJ_FROM_PTR(out);
}
// 支持 2~5 个参数：frame_shape, data, [top_percent], [gain_clip], [brightness_boost]
STATIC MP_DEFINE_CONST_FUN_OBJ_VAR_BETWEEN(cv_lite_rgb888_white_balance_white_patch_ex_obj, 2, 5, cv_lite_rgb888_white_balance_white_patch_ex);


STATIC mp_obj_t cv_lite_rgb888_white_balance_gray_world_adjustable(size_t n_args, const mp_obj_t *args)
{
    mp_obj_list_t *frame_size_mp = MP_OBJ_TO_PTR(args[0]);
    ndarray_obj_t *data = MP_ROM_PTR(args[1]); // hwc
    uint8_t *img_data = data->array;

    // 构造图像尺寸
    FrameCHWSize frame_shape;
    frame_shape.height = mp_obj_get_int(frame_size_mp->items[0]);
    frame_shape.width  = mp_obj_get_int(frame_size_mp->items[1]);
    frame_shape.channel = 3;

    float alpha = mp_obj_get_float(args[2]);

    uint8_t* result=(uint8_t*)malloc(frame_shape.width*frame_shape.height*frame_shape.channel);
    // 处理：白平衡
    rgb888_white_balance_gray_world_adjustable(frame_shape, img_data, alpha, result);
    // 构造返回的 numpy 对象（共享内存，不复制）
    size_t ndarray_shape[4];
    ndarray_shape[1] = frame_shape.height;
    ndarray_shape[2] = frame_shape.width;
    ndarray_shape[3] = frame_shape.channel;
    ndarray_obj_t *out = ndarray_new_ndarray(3, ndarray_shape, NULL, NDARRAY_UINT8);
    uint8_t *out_data = (uint8_t *)out->array;
    memcpy(out_data, result, frame_shape.width * frame_shape.height * frame_shape.channel); // 独立返回一份数据
    free(result);
    return MP_OBJ_FROM_PTR(out);
}
STATIC MP_DEFINE_CONST_FUN_OBJ_VAR_BETWEEN(cv_lite_rgb888_white_balance_gray_world_adjustable_obj, 3, 3, cv_lite_rgb888_white_balance_gray_world_adjustable);

STATIC mp_obj_t cv_lite_rgb888_adjust_exposure(size_t n_args, const mp_obj_t *args)
{
    mp_obj_list_t *frame_size_mp = MP_OBJ_TO_PTR(args[0]);
    ndarray_obj_t *data = MP_ROM_PTR(args[1]); // hwc
    uint8_t *img_data = data->array;


    // 构造图像尺寸
    FrameCHWSize frame_shape;
    frame_shape.height = mp_obj_get_int(frame_size_mp->items[0]);
    frame_shape.width  = mp_obj_get_int(frame_size_mp->items[1]);
    frame_shape.channel = 3;

    float exposure_gain = mp_obj_get_float(args[2]);
    uint8_t* result=(uint8_t*)malloc(frame_shape.width*frame_shape.height*frame_shape.channel);
    // 处理：曝光调整
    rgb888_adjust_exposure(frame_shape, img_data, exposure_gain, result);
    // 构造返回的 numpy 对象（共享内存，不复制）
    size_t ndarray_shape[4];
    ndarray_shape[1] = frame_shape.height;
    ndarray_shape[2] = frame_shape.width;
    ndarray_shape[3] = frame_shape.channel;
    ndarray_obj_t *out = ndarray_new_ndarray(3, ndarray_shape, NULL, NDARRAY_UINT8);
    uint8_t *out_data = (uint8_t *)out->array;
    memcpy(out_data, result, frame_shape.width * frame_shape.height * frame_shape.channel); // 独立返回一份数据
    free(result);
    return MP_OBJ_FROM_PTR(out);
}
STATIC MP_DEFINE_CONST_FUN_OBJ_VAR_BETWEEN(cv_lite_rgb888_adjust_exposure_obj, 3, 3, cv_lite_rgb888_adjust_exposure);

STATIC mp_obj_t cv_lite_rgb888_adjust_exposure_fast(size_t n_args, const mp_obj_t *args)
{
    mp_obj_list_t *frame_size_mp = MP_OBJ_TO_PTR(args[0]);
    ndarray_obj_t *data = MP_ROM_PTR(args[1]); // hwc
    uint8_t *img_data = data->array;


    // 构造图像尺寸
    FrameCHWSize frame_shape;
    frame_shape.height = mp_obj_get_int(frame_size_mp->items[0]);
    frame_shape.width  = mp_obj_get_int(frame_size_mp->items[1]);
    frame_shape.channel = 3;

    float exposure_gain = mp_obj_get_float(args[2]);
    uint8_t* result=(uint8_t*)malloc(frame_shape.width*frame_shape.height*frame_shape.channel);
    // 处理：曝光调整
    rgb888_adjust_exposure_fast(frame_shape, img_data, exposure_gain, result);
    // 构造返回的 numpy 对象（共享内存，不复制）
    size_t ndarray_shape[4];
    ndarray_shape[1] = frame_shape.height;
    ndarray_shape[2] = frame_shape.width;
    ndarray_shape[3] = frame_shape.channel;
    ndarray_obj_t *out = ndarray_new_ndarray(3, ndarray_shape, NULL, NDARRAY_UINT8);
    uint8_t *out_data = (uint8_t *)out->array;
    memcpy(out_data, result, frame_shape.width * frame_shape.height * frame_shape.channel); // 独立返回一份数据
    free(result);
    return MP_OBJ_FROM_PTR(out);
}
STATIC MP_DEFINE_CONST_FUN_OBJ_VAR_BETWEEN(cv_lite_rgb888_adjust_exposure_fast_obj, 3, 3, cv_lite_rgb888_adjust_exposure_fast);

STATIC mp_obj_t cv_lite_rgb888_denoise(size_t n_args, const mp_obj_t *args)
{
    mp_obj_list_t *frame_size_mp = MP_OBJ_TO_PTR(args[0]);
    ndarray_obj_t *data = MP_ROM_PTR(args[1]); // hwc
    uint8_t *img_data = data->array;

    // 构造图像尺寸
    FrameCHWSize frame_shape;
    frame_shape.height = mp_obj_get_int(frame_size_mp->items[0]);
    frame_shape.width  = mp_obj_get_int(frame_size_mp->items[1]);
    frame_shape.channel = 3;

    int method = mp_obj_get_int(args[2]);
    int strength = mp_obj_get_int(args[3]);
    uint8_t* result=(uint8_t*)malloc(frame_shape.width*frame_shape.height*frame_shape.channel);
    // 处理：曝光调整
    rgb888_denoise(frame_shape, img_data, method, strength, result);
    // 构造返回的 numpy 对象（共享内存，不复制）
    size_t ndarray_shape[4];
    ndarray_shape[1] = frame_shape.height;
    ndarray_shape[2] = frame_shape.width;
    ndarray_shape[3] = frame_shape.channel;
    ndarray_obj_t *out = ndarray_new_ndarray(3, ndarray_shape, NULL, NDARRAY_UINT8);
    uint8_t *out_data = (uint8_t *)out->array;
    memcpy(out_data, result, frame_shape.width * frame_shape.height * frame_shape.channel); // 独立返回一份数据
    free(result);
    return MP_OBJ_FROM_PTR(out);
}
STATIC MP_DEFINE_CONST_FUN_OBJ_VAR_BETWEEN(cv_lite_rgb888_denoise_obj, 4, 4, cv_lite_rgb888_denoise);

STATIC mp_obj_t cv_lite_rgb888_mean_blur(size_t n_args, const mp_obj_t *args)
{
    mp_obj_list_t *frame_size_mp = MP_OBJ_TO_PTR(args[0]);
    ndarray_obj_t *data = MP_ROM_PTR(args[1]); // hwc
    uint8_t *img_data = data->array;

    // 构造图像尺寸
    FrameCHWSize frame_shape;
    frame_shape.height = mp_obj_get_int(frame_size_mp->items[0]);
    frame_shape.width  = mp_obj_get_int(frame_size_mp->items[1]);
    frame_shape.channel = 3;
    int kernel_size = mp_obj_get_int(args[2]);
    uint8_t* result=(uint8_t*)malloc(frame_shape.width*frame_shape.height*frame_shape.channel);
    // 处理：曝光调整
    rgb888_mean_blur(frame_shape, img_data, kernel_size, result);
    // 构造返回的 numpy 对象（共享内存，不复制）
    size_t ndarray_shape[4];
    ndarray_shape[1] = frame_shape.height;
    ndarray_shape[2] = frame_shape.width;
    ndarray_shape[3] = frame_shape.channel;
    ndarray_obj_t *out = ndarray_new_ndarray(3, ndarray_shape, NULL, NDARRAY_UINT8);
    uint8_t *out_data = (uint8_t *)out->array;
    memcpy(out_data, result, frame_shape.width * frame_shape.height * frame_shape.channel); // 独立返回一份数据
    free(result);
    return MP_OBJ_FROM_PTR(out);
}
STATIC MP_DEFINE_CONST_FUN_OBJ_VAR_BETWEEN(cv_lite_rgb888_mean_blur_obj, 3, 3, cv_lite_rgb888_mean_blur);

STATIC mp_obj_t cv_lite_rgb888_mean_blur_fast(size_t n_args, const mp_obj_t *args)
{
    mp_obj_list_t *frame_size_mp = MP_OBJ_TO_PTR(args[0]);
    ndarray_obj_t *data = MP_ROM_PTR(args[1]); // hwc
    uint8_t *img_data = data->array;

    // 构造图像尺寸
    FrameCHWSize frame_shape;
    frame_shape.height = mp_obj_get_int(frame_size_mp->items[0]);
    frame_shape.width  = mp_obj_get_int(frame_size_mp->items[1]);
    frame_shape.channel = 3;
    int kernel_size = mp_obj_get_int(args[2]);
    uint8_t* result=(uint8_t*)malloc(frame_shape.width*frame_shape.height*frame_shape.channel);
    // 处理：曝光调整
    rgb888_mean_blur_fast(frame_shape, img_data, kernel_size, result);
    // 构造返回的 numpy 对象（共享内存，不复制）
    size_t ndarray_shape[4];
    ndarray_shape[1] = frame_shape.height;
    ndarray_shape[2] = frame_shape.width;
    ndarray_shape[3] = frame_shape.channel;
    ndarray_obj_t *out = ndarray_new_ndarray(3, ndarray_shape, NULL, NDARRAY_UINT8);
    uint8_t *out_data = (uint8_t *)out->array;
    memcpy(out_data, result, frame_shape.width * frame_shape.height * frame_shape.channel); // 独立返回一份数据
    free(result);
    return MP_OBJ_FROM_PTR(out);
}
STATIC MP_DEFINE_CONST_FUN_OBJ_VAR_BETWEEN(cv_lite_rgb888_mean_blur_fast_obj, 3, 3, cv_lite_rgb888_mean_blur_fast);

STATIC mp_obj_t cv_lite_rgb888_gaussian_blur_fast(size_t n_args, const mp_obj_t *args)
{
    mp_obj_list_t *frame_size_mp = MP_OBJ_TO_PTR(args[0]);
    ndarray_obj_t *data = MP_ROM_PTR(args[1]); // hwc
    uint8_t *img_data = data->array;

    // 构造图像尺寸
    FrameCHWSize frame_shape;
    frame_shape.height = mp_obj_get_int(frame_size_mp->items[0]);
    frame_shape.width  = mp_obj_get_int(frame_size_mp->items[1]);
    frame_shape.channel = 3;
    int kernel_size = mp_obj_get_int(args[2]);
    uint8_t* result=(uint8_t*)malloc(frame_shape.width*frame_shape.height*frame_shape.channel);
    rgb888_gaussian_blur_fast(frame_shape, img_data, kernel_size, result);
    // 构造返回的 numpy 对象（共享内存，不复制）
    size_t ndarray_shape[4];
    ndarray_shape[1] = frame_shape.height;
    ndarray_shape[2] = frame_shape.width;
    ndarray_shape[3] = frame_shape.channel;
    ndarray_obj_t *out = ndarray_new_ndarray(3, ndarray_shape, NULL, NDARRAY_UINT8);
    uint8_t *out_data = (uint8_t *)out->array;
    memcpy(out_data, result, frame_shape.width * frame_shape.height * frame_shape.channel); // 独立返回一份数据
    free(result);
    return MP_OBJ_FROM_PTR(out);
}
STATIC MP_DEFINE_CONST_FUN_OBJ_VAR_BETWEEN(cv_lite_rgb888_gaussian_blur_fast_obj, 3, 3, cv_lite_rgb888_gaussian_blur_fast);


STATIC mp_obj_t cv_lite_rgb888_denoise_fast(size_t n_args, const mp_obj_t *args)
{
    mp_obj_list_t *frame_size_mp = MP_OBJ_TO_PTR(args[0]);
    ndarray_obj_t *data = MP_ROM_PTR(args[1]); // hwc
    uint8_t *img_data = data->array;

    // 构造图像尺寸
    FrameCHWSize frame_shape;
    frame_shape.height = mp_obj_get_int(frame_size_mp->items[0]);
    frame_shape.width  = mp_obj_get_int(frame_size_mp->items[1]);
    frame_shape.channel = 3;

    int method = mp_obj_get_int(args[2]);
    int strength = mp_obj_get_int(args[3]);
    uint8_t* result=(uint8_t*)malloc(frame_shape.width*frame_shape.height*frame_shape.channel);
    // 处理：曝光调整
    rgb888_denoise_fast(frame_shape, img_data, method, strength, result);
    // 构造返回的 numpy 对象（共享内存，不复制）
    size_t ndarray_shape[4];
    ndarray_shape[1] = frame_shape.height;
    ndarray_shape[2] = frame_shape.width;
    ndarray_shape[3] = frame_shape.channel;
    ndarray_obj_t *out = ndarray_new_ndarray(4, ndarray_shape, NULL, NDARRAY_UINT8);
    uint8_t *out_data = (uint8_t *)out->array;
    memcpy(out_data, result, frame_shape.width * frame_shape.height * frame_shape.channel); // 独立返回一份数据
    free(result);
    return MP_OBJ_FROM_PTR(out);
}
STATIC MP_DEFINE_CONST_FUN_OBJ_VAR_BETWEEN(cv_lite_rgb888_denoise_fast_obj, 4, 4, cv_lite_rgb888_denoise_fast);

STATIC mp_obj_t cv_lite_rgb888_erode(size_t n_args, const mp_obj_t *args)
{
    mp_obj_list_t *frame_size_mp = MP_OBJ_TO_PTR(args[0]);
    ndarray_obj_t *data = MP_ROM_PTR(args[1]); // hwc
    uint8_t *img_data = data->array;

    // 图像尺寸
    FrameCHWSize frame_shape;
    frame_shape.height = mp_obj_get_int(frame_size_mp->items[0]);
    frame_shape.width  = mp_obj_get_int(frame_size_mp->items[1]);
    frame_shape.channel = 3;

    int kernel_size = mp_obj_get_int(args[2]);
    int iterations = mp_obj_get_int(args[3]);
    int threshold_value = mp_obj_get_int(args[4]);  // 第5个参数，支持自定义或 Otsu
    // 分配输出缓冲（单通道）
    uint8_t* result = (uint8_t*)malloc(frame_shape.width * frame_shape.height);
    // 执行腐蚀操作
    rgb888_erode(frame_shape, img_data, kernel_size, iterations, threshold_value, result);

    // 构造 ndarray（灰度图：2D 或 3D 单通道）
    size_t ndarray_shape[4];
    ndarray_shape[1] = frame_shape.height;
    ndarray_shape[2] = frame_shape.width;
    ndarray_shape[3] = 1;  // 单通道
    ndarray_obj_t *out = ndarray_new_ndarray(3, ndarray_shape, NULL, NDARRAY_UINT8);
    uint8_t *out_data = (uint8_t *)out->array;
    // 拷贝输出结果
    memcpy(out_data, result, frame_shape.width * frame_shape.height);
    free(result);
    return MP_OBJ_FROM_PTR(out);
}
STATIC MP_DEFINE_CONST_FUN_OBJ_VAR_BETWEEN(cv_lite_rgb888_erode_obj, 5, 5, cv_lite_rgb888_erode);

STATIC mp_obj_t cv_lite_rgb888_dilate(size_t n_args, const mp_obj_t *args)
{
    mp_obj_list_t *frame_size_mp = MP_OBJ_TO_PTR(args[0]);
    ndarray_obj_t *data = MP_ROM_PTR(args[1]); // hwc
    uint8_t *img_data = data->array;

    // 构造图像尺寸
    FrameCHWSize frame_shape;
    frame_shape.height = mp_obj_get_int(frame_size_mp->items[0]);
    frame_shape.width  = mp_obj_get_int(frame_size_mp->items[1]);
    frame_shape.channel = 3;
    
    int kernel_size = mp_obj_get_int(args[2]);
    int iterations = mp_obj_get_int(args[3]);
    int threshold_value = mp_obj_get_int(args[4]);  // 第5个参数，支持自定义或 Otsu
    uint8_t* result=(uint8_t*)malloc(frame_shape.width*frame_shape.height*frame_shape.channel);
    // 处理：曝光调整
    rgb888_dilate(frame_shape, img_data, kernel_size, iterations, threshold_value, result);
    // 构造返回的 numpy 对象（共享内存，不复制）
    size_t ndarray_shape[4];
    ndarray_shape[1] = frame_shape.height;
    ndarray_shape[2] = frame_shape.width;
    ndarray_shape[3] = 1;
    ndarray_obj_t *out = ndarray_new_ndarray(4, ndarray_shape, NULL, NDARRAY_UINT8);
    uint8_t *out_data = (uint8_t *)out->array;
    memcpy(out_data, result, frame_shape.width * frame_shape.height); // 独立返回一份数据
    free(result);
    return MP_OBJ_FROM_PTR(out);
}
STATIC MP_DEFINE_CONST_FUN_OBJ_VAR_BETWEEN(cv_lite_rgb888_dilate_obj, 5, 5, cv_lite_rgb888_dilate);


STATIC mp_obj_t cv_lite_rgb888_open(size_t n_args, const mp_obj_t *args)
{
    mp_obj_list_t *frame_size_mp = MP_OBJ_TO_PTR(args[0]);
    ndarray_obj_t *data = MP_ROM_PTR(args[1]); // hwc
    uint8_t *img_data = data->array;

    // 构造图像尺寸
    FrameCHWSize frame_shape;
    frame_shape.height = mp_obj_get_int(frame_size_mp->items[0]);
    frame_shape.width  = mp_obj_get_int(frame_size_mp->items[1]);
    frame_shape.channel = 3;
    
    int kernel_size = mp_obj_get_int(args[2]);
    int iterations = mp_obj_get_int(args[3]);
    int threshold_value = mp_obj_get_int(args[4]);  // 第5个参数，支持自定义或 Otsu
    uint8_t* result=(uint8_t*)malloc(frame_shape.width*frame_shape.height*frame_shape.channel);
    // 处理：曝光调整
    rgb888_open(frame_shape, img_data, kernel_size, iterations, threshold_value, result);
    // 构造返回的 numpy 对象（共享内存，不复制）
    size_t ndarray_shape[4];
    ndarray_shape[1] = frame_shape.height;
    ndarray_shape[2] = frame_shape.width;
    ndarray_shape[3] = 1;
    ndarray_obj_t *out = ndarray_new_ndarray(4, ndarray_shape, NULL, NDARRAY_UINT8);
    uint8_t *out_data = (uint8_t *)out->array;
    memcpy(out_data, result, frame_shape.width * frame_shape.height); // 独立返回一份数据
    free(result);
    return MP_OBJ_FROM_PTR(out);
}
STATIC MP_DEFINE_CONST_FUN_OBJ_VAR_BETWEEN(cv_lite_rgb888_open_obj, 5, 5, cv_lite_rgb888_open);

STATIC mp_obj_t cv_lite_rgb888_close(size_t n_args, const mp_obj_t *args)
{
    mp_obj_list_t *frame_size_mp = MP_OBJ_TO_PTR(args[0]);
    ndarray_obj_t *data = MP_ROM_PTR(args[1]); // hwc
    uint8_t *img_data = data->array;

    // 构造图像尺寸
    FrameCHWSize frame_shape;
    frame_shape.height = mp_obj_get_int(frame_size_mp->items[0]);
    frame_shape.width  = mp_obj_get_int(frame_size_mp->items[1]);
    frame_shape.channel = 3;
    
    int kernel_size = mp_obj_get_int(args[2]);
    int iterations = mp_obj_get_int(args[3]);
    int threshold_value = mp_obj_get_int(args[4]);  // 第5个参数，支持自定义或 Otsu

    uint8_t* result=(uint8_t*)malloc(frame_shape.width*frame_shape.height);
    // 处理：曝光调整
    rgb888_close(frame_shape, img_data, kernel_size, iterations,threshold_value,  result);
    // 构造返回的 numpy 对象（共享内存，不复制）
    size_t ndarray_shape[4];
    ndarray_shape[1] = frame_shape.height;
    ndarray_shape[2] = frame_shape.width;
    ndarray_shape[3] = 1;
    ndarray_obj_t *out = ndarray_new_ndarray(4, ndarray_shape, NULL, NDARRAY_UINT8);
    uint8_t *out_data = (uint8_t *)out->array;
    memcpy(out_data, result, frame_shape.width * frame_shape.height); // 独立返回一份数据
    free(result);
    return MP_OBJ_FROM_PTR(out);
}
STATIC MP_DEFINE_CONST_FUN_OBJ_VAR_BETWEEN(cv_lite_rgb888_close_obj, 5, 5, cv_lite_rgb888_close);

STATIC mp_obj_t cv_lite_rgb888_gradient(size_t n_args, const mp_obj_t *args)
{
    mp_obj_list_t *frame_size_mp = MP_OBJ_TO_PTR(args[0]);
    ndarray_obj_t *data = MP_ROM_PTR(args[1]); // hwc
    uint8_t *img_data = data->array;
    // 构造图像尺寸
    FrameCHWSize frame_shape;
    frame_shape.height = mp_obj_get_int(frame_size_mp->items[0]);
    frame_shape.width  = mp_obj_get_int(frame_size_mp->items[1]);
    frame_shape.channel = 3;
    
    int kernel_size = mp_obj_get_int(args[2]);
    int iterations = mp_obj_get_int(args[3]);
    int threshold_value = mp_obj_get_int(args[4]);  // 第5个参数，支持自定义或 Otsu
    uint8_t* result=(uint8_t*)malloc(frame_shape.width*frame_shape.height);
    // 处理：曝光调整
    rgb888_gradient(frame_shape, img_data, kernel_size, iterations, threshold_value, result);
    // 构造返回的 numpy 对象（共享内存，不复制）
    size_t ndarray_shape[4];
    ndarray_shape[1] = frame_shape.height;
    ndarray_shape[2] = frame_shape.width;
    ndarray_shape[3] = frame_shape.channel;
    ndarray_obj_t *out = ndarray_new_ndarray(4, ndarray_shape, NULL, NDARRAY_UINT8);
    uint8_t *out_data = (uint8_t *)out->array;
    memcpy(out_data, result, frame_shape.width * frame_shape.height); // 独立返回一份数据
    free(result);
    return MP_OBJ_FROM_PTR(out);
}
STATIC MP_DEFINE_CONST_FUN_OBJ_VAR_BETWEEN(cv_lite_rgb888_gradient_obj, 5, 5, cv_lite_rgb888_gradient);

STATIC mp_obj_t cv_lite_rgb888_tophat(size_t n_args, const mp_obj_t *args)
{
    mp_obj_list_t *frame_size_mp = MP_OBJ_TO_PTR(args[0]);
    ndarray_obj_t *data = MP_ROM_PTR(args[1]); // hwc
    uint8_t *img_data = data->array;
    // 构造图像尺寸
    FrameCHWSize frame_shape;
    frame_shape.height = mp_obj_get_int(frame_size_mp->items[0]);
    frame_shape.width  = mp_obj_get_int(frame_size_mp->items[1]);
    frame_shape.channel = 3;
    
    int kernel_size = mp_obj_get_int(args[2]);
    int iterations = mp_obj_get_int(args[3]);
    int threshold_value = mp_obj_get_int(args[4]);  // 第5个参数，支持自定义或 Otsu
    uint8_t* result=(uint8_t*)malloc(frame_shape.width*frame_shape.height);
    // 处理：曝光调整
    rgb888_tophat(frame_shape, img_data, kernel_size, iterations, threshold_value, result);
    // 构造返回的 numpy 对象（共享内存，不复制）
    size_t ndarray_shape[4];
    ndarray_shape[1] = frame_shape.height;
    ndarray_shape[2] = frame_shape.width;
    ndarray_shape[3] = frame_shape.channel;
    ndarray_obj_t *out = ndarray_new_ndarray(4, ndarray_shape, NULL, NDARRAY_UINT8);
    uint8_t *out_data = (uint8_t *)out->array;
    memcpy(out_data, result, frame_shape.width * frame_shape.height); // 独立返回一份数据
    free(result);
    return MP_OBJ_FROM_PTR(out);
}
STATIC MP_DEFINE_CONST_FUN_OBJ_VAR_BETWEEN(cv_lite_rgb888_tophat_obj, 5, 5, cv_lite_rgb888_tophat);

STATIC mp_obj_t cv_lite_rgb888_blackhat(size_t n_args, const mp_obj_t *args)
{
    mp_obj_list_t *frame_size_mp = MP_OBJ_TO_PTR(args[0]);
    ndarray_obj_t *data = MP_ROM_PTR(args[1]); // hwc
    uint8_t *img_data = data->array;
    // 构造图像尺寸
    FrameCHWSize frame_shape;
    frame_shape.height = mp_obj_get_int(frame_size_mp->items[0]);
    frame_shape.width  = mp_obj_get_int(frame_size_mp->items[1]);
    frame_shape.channel = 3;
    
    int kernel_size = mp_obj_get_int(args[2]);
    int iterations = mp_obj_get_int(args[3]);
    int threshold_value = mp_obj_get_int(args[4]);  // 第5个参数，支持自定义或 Otsu
    uint8_t* result=(uint8_t*)malloc(frame_shape.width*frame_shape.height);
    // 处理：曝光调整
    rgb888_blackhat(frame_shape, img_data, kernel_size, iterations, threshold_value, result);
    // 构造返回的 numpy 对象（共享内存，不复制）
    size_t ndarray_shape[4];
    ndarray_shape[1] = frame_shape.height;
    ndarray_shape[2] = frame_shape.width;
    ndarray_shape[3] = frame_shape.channel;
    ndarray_obj_t *out = ndarray_new_ndarray(4, ndarray_shape, NULL, NDARRAY_UINT8);
    uint8_t *out_data = (uint8_t *)out->array;
    memcpy(out_data, result, frame_shape.width * frame_shape.height); // 独立返回一份数据
    free(result);
    return MP_OBJ_FROM_PTR(out);
}
STATIC MP_DEFINE_CONST_FUN_OBJ_VAR_BETWEEN(cv_lite_rgb888_blackhat_obj, 5, 5, cv_lite_rgb888_blackhat);

STATIC mp_obj_t cv_lite_rgb888_calc_histogram(size_t n_args, const mp_obj_t *args)
{
    // 解析输入参数
    mp_obj_list_t *frame_size_mp = MP_OBJ_TO_PTR(args[0]);
    ndarray_obj_t *data = MP_ROM_PTR(args[1]); // HW3
    uint8_t *img_data = data->array;

    // 构造图像尺寸
    FrameCHWSize frame_shape;
    frame_shape.height = mp_obj_get_int(frame_size_mp->items[0]);
    frame_shape.width  = mp_obj_get_int(frame_size_mp->items[1]);
    frame_shape.channel = 3;

    // 分配直方图缓冲区 [3][256]，使用 uint32_t 避免溢出
    uint32_t hist_buf[3][256] = {0};

    // 假设 rgb888_calc_histogram 支持 uint32_t 输出
    rgb888_calc_histogram(frame_shape, img_data, (uint32_t *)hist_buf);

    // 转换为 list of list 返回
    mp_obj_t outer_list = mp_obj_new_list(0, NULL);

    for (int c = 0; c < 3; c++) {
        mp_obj_t inner_list = mp_obj_new_list(256, NULL);
        for (int i = 0; i < 256; i++) {
            // 使用 mp_obj_new_int_from_uint 创建 Python int
            ((mp_obj_list_t *)MP_OBJ_TO_PTR(inner_list))->items[i] = mp_obj_new_int_from_uint(hist_buf[c][i]);
        }
        mp_obj_list_append(outer_list, inner_list);
    }

    return outer_list;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_VAR_BETWEEN(cv_lite_rgb888_calc_histogram_obj, 2, 2, cv_lite_rgb888_calc_histogram);

STATIC mp_obj_t cv_lite_grayscale_find_corners(size_t n_args, const mp_obj_t *args)
{
    // 解析输入参数
    mp_obj_list_t *frame_size_mp = MP_OBJ_TO_PTR(args[0]);
    ndarray_obj_t *data = MP_ROM_PTR(args[1]); // HW3
    uint8_t *img_data = data->array;

    // 构造图像尺寸
    FrameCHWSize frame_shape;
    frame_shape.height = mp_obj_get_int(frame_size_mp->items[0]);
    frame_shape.width  = mp_obj_get_int(frame_size_mp->items[1]);
    frame_shape.channel = 1;
    int maxCorners = mp_obj_get_int(args[2]);
    double qualityLevel = mp_obj_get_float(args[3]);
    double minDistance = mp_obj_get_float(args[4]);
    // 调用 C++ 检测函数
    int ret_num = 0;
    int *ret = grayscale_find_corners(frame_shape, img_data, maxCorners, qualityLevel, minDistance, &ret_num);

    // 构造返回 Python 列表 [x0, y0, x1, y1, ...]
    mp_obj_t *items = m_new(mp_obj_t, ret_num * 2);
    for (int i = 0; i < ret_num; ++i) {
        items[i * 2 + 0] = mp_obj_new_int(ret[i * 2 + 0]);
        items[i * 2 + 1] = mp_obj_new_int(ret[i * 2 + 1]);
    }
    mp_obj_t list_obj = mp_obj_new_list(ret_num * 2, items);
    free(ret);
    return list_obj;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_VAR_BETWEEN(cv_lite_grayscale_find_corners_obj, 5, 5, cv_lite_grayscale_find_corners);

STATIC mp_obj_t cv_lite_rgb888_find_corners(size_t n_args, const mp_obj_t *args)
{
    // 解析输入参数
    mp_obj_list_t *frame_size_mp = MP_OBJ_TO_PTR(args[0]);
    ndarray_obj_t *data = MP_ROM_PTR(args[1]); // HW3
    uint8_t *img_data = data->array;

    // 构造图像尺寸
    FrameCHWSize frame_shape;
    frame_shape.height = mp_obj_get_int(frame_size_mp->items[0]);
    frame_shape.width  = mp_obj_get_int(frame_size_mp->items[1]);
    frame_shape.channel = 3;
    int maxCorners = mp_obj_get_int(args[2]);
    double qualityLevel = mp_obj_get_float(args[3]);
    double minDistance = mp_obj_get_float(args[4]);
    // 调用 C++ 检测函数
    int ret_num = 0;
    int *ret = rgb888_find_corners(frame_shape, img_data, maxCorners, qualityLevel, minDistance, &ret_num);

    // 构造返回 Python 列表 [x0, y0, x1, y1, ...]
    mp_obj_t *items = m_new(mp_obj_t, ret_num * 2);
    for (int i = 0; i < ret_num; ++i) {
        items[i * 2 + 0] = mp_obj_new_int(ret[i * 2 + 0]);
        items[i * 2 + 1] = mp_obj_new_int(ret[i * 2 + 1]);
    }
    mp_obj_t list_obj = mp_obj_new_list(ret_num * 2, items);
    free(ret);
    return list_obj;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_VAR_BETWEEN(cv_lite_rgb888_find_corners_obj, 5, 5, cv_lite_rgb888_find_corners);

STATIC mp_obj_t cv_lite_rgb888_find_corners_fast(size_t n_args, const mp_obj_t *args)
{
    // 解析输入参数
    mp_obj_list_t *frame_size_mp = MP_OBJ_TO_PTR(args[0]);
    ndarray_obj_t *data = MP_ROM_PTR(args[1]); // HW3
    uint8_t *img_data = data->array;

    // 构造图像尺寸
    FrameCHWSize frame_shape;
    frame_shape.height = mp_obj_get_int(frame_size_mp->items[0]);
    frame_shape.width  = mp_obj_get_int(frame_size_mp->items[1]);
    frame_shape.channel = 3;
    int maxCorners = mp_obj_get_int(args[2]);
    float qualityLevel = mp_obj_get_float(args[3]);
    float minDistance = mp_obj_get_float(args[4]);
    // 调用 C++ 检测函数
    int ret_num = 0;
    int *ret = rgb888_find_corners_fast(frame_shape, img_data, maxCorners, qualityLevel, minDistance, &ret_num);
    // 构造返回 Python 列表 [x0, y0, x1, y1, ...]
    mp_obj_t *items = m_new(mp_obj_t, ret_num * 2);
    for (int i = 0; i < ret_num; ++i) {
        items[i * 2 + 0] = mp_obj_new_int(ret[i * 2 + 0]);
        items[i * 2 + 1] = mp_obj_new_int(ret[i * 2 + 1]);
    }
    mp_obj_t list_obj = mp_obj_new_list(ret_num * 2, items);
    free(ret);
    return list_obj;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_VAR_BETWEEN(cv_lite_rgb888_find_corners_fast_obj, 5, 5, cv_lite_rgb888_find_corners_fast);

STATIC mp_obj_t cv_lite_save_image(size_t n_args, const mp_obj_t *args)
{
    // 解析输入参数
    const char* save_path = mp_obj_str_get_str(args[0]);
    mp_obj_list_t *frame_size_mp = MP_OBJ_TO_PTR(args[1]);
    ndarray_obj_t *data = MP_ROM_PTR(args[2]); // HW3
    uint8_t *img_data = data->array;
    // 构造图像尺寸
    FrameCHWSize frame_shape;
    frame_shape.height = mp_obj_get_int(frame_size_mp->items[0]);
    frame_shape.width  = mp_obj_get_int(frame_size_mp->items[1]);
    frame_shape.channel = 3;
    // 调用 C++ 检测函数
    save_image(save_path,frame_shape,img_data);
    return mp_const_none;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_VAR_BETWEEN(cv_lite_save_image_obj, 3, 3, cv_lite_save_image);

STATIC mp_obj_t cv_lite_load_image(size_t n_args, const mp_obj_t *args)
{
    // 解析输入参数
    const char* file_path = mp_obj_str_get_str(args[0]);

    // 调用C++读取函数（假设返回BGR或RGB格式，8UC3）
    FrameCHWSize frame_shape;
    uint8_t* img_data = load_image(file_path, &frame_shape); 
    // ⚠️ 这里 read_image 要能分配内存并返回指针，同时写入宽高通道

    // 构造返回的 numpy 对象（共享内存，不复制）
    size_t ndarray_shape[4];
    ndarray_shape[0] = 1;
    ndarray_shape[1] = frame_shape.height;
    ndarray_shape[2] = frame_shape.width;
    ndarray_shape[3] = frame_shape.channel;
    ndarray_obj_t *out = ndarray_new_ndarray(4, ndarray_shape, NULL, NDARRAY_UINT8);
    uint8_t *out_data = (uint8_t *)out->array;
    memcpy(out_data, img_data, frame_shape.width * frame_shape.height * frame_shape.channel); // 独立返回一份数据
    free(img_data);
    return MP_OBJ_FROM_PTR(out);
}
STATIC MP_DEFINE_CONST_FUN_OBJ_VAR_BETWEEN(cv_lite_load_image_obj, 1, 1, cv_lite_load_image);


STATIC mp_obj_t cv_lite_rgb888_undistort(size_t n_args, const mp_obj_t *args)
{
    mp_obj_list_t *frame_size_mp = MP_OBJ_TO_PTR(args[0]);
    ndarray_obj_t *data = MP_ROM_PTR(args[1]); // hwc
    uint8_t *img_data = data->array;

    // 构造图像尺寸
    FrameCHWSize frame_shape;
    frame_shape.height = mp_obj_get_int(frame_size_mp->items[0]);
    frame_shape.width  = mp_obj_get_int(frame_size_mp->items[1]);
    frame_shape.channel = 3;

    mp_obj_list_t *camera_matrix_mp = MP_OBJ_TO_PTR(args[2]);
    mp_obj_list_t *dist_coeffs_mp = MP_OBJ_TO_PTR(args[3]);
    int dist_len = mp_obj_get_int(args[4]);

    // camera_matrix_mp 和 dist_coeffs_mp 是传进来的 mp_obj_t
    mp_obj_t *camera_matrix_items;
    size_t camera_matrix_len;
    mp_obj_get_array(camera_matrix_mp, &camera_matrix_len, &camera_matrix_items);

    // 分配 float 缓冲
    float camera_matrix[9];
    for (size_t i = 0; i < camera_matrix_len && i < 9; ++i) {
        camera_matrix[i] = mp_obj_get_float(camera_matrix_items[i]);
    }

    // 解析 dist_coeffs
    mp_obj_t *dist_coeff_items;
    size_t dist_coeff_len;
    mp_obj_get_array(dist_coeffs_mp, &dist_coeff_len, &dist_coeff_items);

    float dist_coeffs[dist_coeff_len];  // 最多支持 8 个系数
    for (size_t i = 0; i < dist_coeff_len && i < 8; ++i) {
        dist_coeffs[i] = mp_obj_get_float(dist_coeff_items[i]);
    }
    uint8_t* result=(uint8_t*)malloc(frame_shape.width*frame_shape.height*frame_shape.channel);
    rgb888_undistort(frame_shape,img_data,camera_matrix,dist_coeffs,dist_len,result);
    // 构造返回的 numpy 对象（共享内存，不复制）
    size_t ndarray_shape[4];
    ndarray_shape[0] = 1;
    ndarray_shape[1] = frame_shape.height;
    ndarray_shape[2] = frame_shape.width;
    ndarray_shape[3] = frame_shape.channel;
    ndarray_obj_t *out = ndarray_new_ndarray(4, ndarray_shape, NULL, NDARRAY_UINT8);
    uint8_t *out_data = (uint8_t *)out->array;
    memcpy(out_data, result, frame_shape.width * frame_shape.height * frame_shape.channel); // 独立返回一份数据
    free(result);
    return MP_OBJ_FROM_PTR(out);
}
STATIC MP_DEFINE_CONST_FUN_OBJ_VAR_BETWEEN(cv_lite_rgb888_undistort_obj, 5, 5, cv_lite_rgb888_undistort);

STATIC mp_obj_t cv_lite_rgb888_undistort_fast(size_t n_args, const mp_obj_t *args)
{
    mp_obj_list_t *frame_size_mp = MP_OBJ_TO_PTR(args[0]);
    ndarray_obj_t *data = MP_ROM_PTR(args[1]); // hwc
    uint8_t *img_data = data->array;

    // 构造图像尺寸
    FrameCHWSize frame_shape;
    frame_shape.height = mp_obj_get_int(frame_size_mp->items[0]);
    frame_shape.width  = mp_obj_get_int(frame_size_mp->items[1]);
    frame_shape.channel = 3;

    mp_obj_list_t *camera_matrix_mp = MP_OBJ_TO_PTR(args[2]);
    mp_obj_list_t *dist_coeffs_mp = MP_OBJ_TO_PTR(args[3]);
    int dist_len = mp_obj_get_int(args[4]);

    // camera_matrix_mp 和 dist_coeffs_mp 是传进来的 mp_obj_t
    mp_obj_t *camera_matrix_items;
    size_t camera_matrix_len;
    mp_obj_get_array(camera_matrix_mp, &camera_matrix_len, &camera_matrix_items);

    // 分配 float 缓冲
    float camera_matrix[9];
    for (size_t i = 0; i < camera_matrix_len && i < 9; ++i) {
        camera_matrix[i] = mp_obj_get_float(camera_matrix_items[i]);
    }

    // 解析 dist_coeffs
    mp_obj_t *dist_coeff_items;
    size_t dist_coeff_len;
    mp_obj_get_array(dist_coeffs_mp, &dist_coeff_len, &dist_coeff_items);

    float dist_coeffs[dist_coeff_len];  // 最多支持 8 个系数
    for (size_t i = 0; i < dist_coeff_len && i < 8; ++i) {
        dist_coeffs[i] = mp_obj_get_float(dist_coeff_items[i]);
    }
    uint8_t* result=(uint8_t*)malloc(frame_shape.width*frame_shape.height*frame_shape.channel);
    rgb888_undistort_fast(frame_shape,img_data,camera_matrix,dist_coeffs,dist_len,result);
    // 构造返回的 numpy 对象（共享内存，不复制）
    size_t ndarray_shape[4];
    ndarray_shape[0] = 1;
    ndarray_shape[1] = frame_shape.height;
    ndarray_shape[2] = frame_shape.width;
    ndarray_shape[3] = frame_shape.channel;
    ndarray_obj_t *out = ndarray_new_ndarray(4, ndarray_shape, NULL, NDARRAY_UINT8);
    uint8_t *out_data = (uint8_t *)out->array;
    memcpy(out_data, result, frame_shape.width * frame_shape.height * frame_shape.channel); // 独立返回一份数据
    free(result);
    return MP_OBJ_FROM_PTR(out);
}
STATIC MP_DEFINE_CONST_FUN_OBJ_VAR_BETWEEN(cv_lite_rgb888_undistort_fast_obj, 5, 5, cv_lite_rgb888_undistort_fast);

STATIC mp_obj_t cv_lite_rgb888_undistort_new_cam_mat(size_t n_args, const mp_obj_t *args)
{
    mp_obj_list_t *frame_size_mp = MP_OBJ_TO_PTR(args[0]);
    ndarray_obj_t *data = MP_ROM_PTR(args[1]); // hwc
    uint8_t *img_data = data->array;

    // 构造图像尺寸
    FrameCHWSize frame_shape;
    frame_shape.height = mp_obj_get_int(frame_size_mp->items[0]);
    frame_shape.width  = mp_obj_get_int(frame_size_mp->items[1]);
    frame_shape.channel = 3;

    mp_obj_list_t *camera_matrix_mp = MP_OBJ_TO_PTR(args[2]);
    mp_obj_list_t *dist_coeffs_mp = MP_OBJ_TO_PTR(args[3]);
    int dist_len = mp_obj_get_int(args[4]);

    // camera_matrix_mp 和 dist_coeffs_mp 是传进来的 mp_obj_t
    mp_obj_t *camera_matrix_items;
    size_t camera_matrix_len;
    mp_obj_get_array(camera_matrix_mp, &camera_matrix_len, &camera_matrix_items);

    // 分配 float 缓冲
    float camera_matrix[9];
    for (size_t i = 0; i < camera_matrix_len && i < 9; ++i) {
        camera_matrix[i] = mp_obj_get_float(camera_matrix_items[i]);
    }

    // 解析 dist_coeffs
    mp_obj_t *dist_coeff_items;
    size_t dist_coeff_len;
    mp_obj_get_array(dist_coeffs_mp, &dist_coeff_len, &dist_coeff_items);

    float dist_coeffs[dist_coeff_len];  // 最多支持 8 个系数
    for (size_t i = 0; i < dist_coeff_len && i < 8; ++i) {
        dist_coeffs[i] = mp_obj_get_float(dist_coeff_items[i]);
    }
    uint8_t* result=(uint8_t*)malloc(frame_shape.width*frame_shape.height*frame_shape.channel);
    rgb888_undistort_new_cam_mat(frame_shape,img_data,camera_matrix,dist_coeffs,dist_len,result);
    // 构造返回的 numpy 对象（共享内存，不复制）
    size_t ndarray_shape[4];
    ndarray_shape[0] = 1;
    ndarray_shape[1] = frame_shape.height;
    ndarray_shape[2] = frame_shape.width;
    ndarray_shape[3] = frame_shape.channel;
    ndarray_obj_t *out = ndarray_new_ndarray(4, ndarray_shape, NULL, NDARRAY_UINT8);
    uint8_t *out_data = (uint8_t *)out->array;
    memcpy(out_data, result, frame_shape.width * frame_shape.height * frame_shape.channel); // 独立返回一份数据
    free(result);
    return MP_OBJ_FROM_PTR(out);
}
STATIC MP_DEFINE_CONST_FUN_OBJ_VAR_BETWEEN(cv_lite_rgb888_undistort_new_cam_mat_obj, 5, 5, cv_lite_rgb888_undistort_new_cam_mat);

STATIC mp_obj_t cv_lite_rgb888_pnp_distance(size_t n_args, const mp_obj_t *args) {
    // 参数解析
    mp_obj_list_t *frame_size_mp = MP_OBJ_TO_PTR(args[0]);  // [height, width]
    ndarray_obj_t *data = MP_ROM_PTR(args[1]);              // 图像数据
    uint8_t *img_data = data->array;

    FrameCHWSize frame_shape;
    frame_shape.height = mp_obj_get_int(frame_size_mp->items[0]);
    frame_shape.width  = mp_obj_get_int(frame_size_mp->items[1]);
    frame_shape.channel = 3;

    mp_obj_list_t *roi_mp = MP_OBJ_TO_PTR(args[2]);  // [x, y, width, height]
    int roi[4];
    roi[0] = mp_obj_get_int(roi_mp->items[0]);
    roi[1] = mp_obj_get_int(roi_mp->items[1]);
    roi[2] = mp_obj_get_int(roi_mp->items[2]);
    roi[3] = mp_obj_get_int(roi_mp->items[3]);

    mp_obj_list_t *camera_matrix_mp = MP_OBJ_TO_PTR(args[3]);
    mp_obj_list_t *dist_coeffs_mp = MP_OBJ_TO_PTR(args[4]);
    int dist_len = mp_obj_get_int(args[5]);
    float roi_width_real = mp_obj_get_float(args[6]);
    float roi_height_real = mp_obj_get_float(args[7]);

    // camera_matrix_mp 和 dist_coeffs_mp 是传进来的 mp_obj_t
    mp_obj_t *camera_matrix_items;
    size_t camera_matrix_len;
    mp_obj_get_array(camera_matrix_mp, &camera_matrix_len, &camera_matrix_items);

    // 分配 float 缓冲
    float camera_matrix[9];
    for (size_t i = 0; i < camera_matrix_len && i < 9; ++i) {
        camera_matrix[i] = mp_obj_get_float(camera_matrix_items[i]);
    }

    // 解析 dist_coeffs
    mp_obj_t *dist_coeff_items;
    size_t dist_coeff_len;
    mp_obj_get_array(dist_coeffs_mp, &dist_coeff_len, &dist_coeff_items);

    float dist_coeffs[dist_coeff_len];  // 最多支持 8 个系数
    for (size_t i = 0; i < dist_coeff_len && i < 8; ++i) {
        dist_coeffs[i] = mp_obj_get_float(dist_coeff_items[i]);
    }

    // 调用底层直线查找函数
    float distance = rgb888_pnp_distance(frame_shape, img_data, roi, camera_matrix, dist_coeffs, dist_len, roi_width_real, roi_height_real);

    return mp_obj_new_float(distance);
}
STATIC MP_DEFINE_CONST_FUN_OBJ_VAR_BETWEEN(cv_lite_rgb888_pnp_distance_obj, 8, 8, cv_lite_rgb888_pnp_distance);

// STATIC mp_obj_t cv_lite_rgb888_pnp_distance_from_corners(size_t n_args, const mp_obj_t *args) {
//     // 参数解析
//     mp_obj_list_t *frame_size_mp = MP_OBJ_TO_PTR(args[0]);  // [height, width]
//     ndarray_obj_t *data = MP_ROM_PTR(args[1]);              // 图像数据
//     uint8_t *img_data = data->array;

//     FrameCHWSize frame_shape;
//     frame_shape.height = mp_obj_get_int(frame_size_mp->items[0]);
//     frame_shape.width  = mp_obj_get_int(frame_size_mp->items[1]);
//     frame_shape.channel = 3;

//     mp_obj_list_t *camera_matrix_mp = MP_OBJ_TO_PTR(args[2]);
//     mp_obj_list_t *dist_coeffs_mp = MP_OBJ_TO_PTR(args[3]);
//     int dist_len = mp_obj_get_int(args[4]);
//     float obj_width_cm = mp_obj_get_float(args[5]);
//     float obj_height_cm = mp_obj_get_float(args[6]);

//     // camera_matrix_mp 和 dist_coeffs_mp 是传进来的 mp_obj_t
//     mp_obj_t *camera_matrix_items;
//     size_t camera_matrix_len;
//     mp_obj_get_array(camera_matrix_mp, &camera_matrix_len, &camera_matrix_items);

//     // 分配 float 缓冲
//     float camera_matrix[9];
//     for (size_t i = 0; i < camera_matrix_len && i < 9; ++i) {
//         camera_matrix[i] = mp_obj_get_float(camera_matrix_items[i]);
//     }

//     // 解析 dist_coeffs
//     mp_obj_t *dist_coeff_items;
//     size_t dist_coeff_len;
//     mp_obj_get_array(dist_coeffs_mp, &dist_coeff_len, &dist_coeff_items);

//     float dist_coeffs[dist_coeff_len];  // 最多支持 8 个系数
//     for (size_t i = 0; i < dist_coeff_len && i < 8; ++i) {
//         dist_coeffs[i] = mp_obj_get_float(dist_coeff_items[i]);
//     }

//     // 调用底层透视变换函数
//     float distance = rgb888_pnp_distance_from_corners(frame_shape, img_data, camera_matrix, dist_coeffs, dist_len, obj_width_cm, obj_height_cm);

//     return mp_obj_new_float(distance);
// }
// STATIC MP_DEFINE_CONST_FUN_OBJ_VAR_BETWEEN(cv_lite_rgb888_pnp_distance_from_corners_obj, 7, 7, cv_lite_rgb888_pnp_distance_from_corners);

STATIC mp_obj_t cv_lite_rgb888_pnp_distance_from_corners(size_t n_args, const mp_obj_t *args) {
    // 参数解析
    mp_obj_list_t *frame_size_mp = MP_OBJ_TO_PTR(args[0]);  // [height, width]
    ndarray_obj_t *data = MP_ROM_PTR(args[1]);              // 图像数据
    uint8_t *img_data = data->array;

    FrameCHWSize frame_shape;
    frame_shape.height = mp_obj_get_int(frame_size_mp->items[0]);
    frame_shape.width  = mp_obj_get_int(frame_size_mp->items[1]);
    frame_shape.channel = 3;

    mp_obj_list_t *camera_matrix_mp = MP_OBJ_TO_PTR(args[2]);
    mp_obj_list_t *dist_coeffs_mp = MP_OBJ_TO_PTR(args[3]);
    int dist_len = mp_obj_get_int(args[4]);
    float obj_width_cm = mp_obj_get_float(args[5]);
    float obj_height_cm = mp_obj_get_float(args[6]);

    // camera_matrix_mp 和 dist_coeffs_mp 是传进来的 mp_obj_t
    mp_obj_t *camera_matrix_items;
    size_t camera_matrix_len;
    mp_obj_get_array(camera_matrix_mp, &camera_matrix_len, &camera_matrix_items);

    // 分配 float 缓冲
    float camera_matrix[9];
    for (size_t i = 0; i < camera_matrix_len && i < 9; ++i) {
        camera_matrix[i] = mp_obj_get_float(camera_matrix_items[i]);
    }

    // 解析 dist_coeffs
    mp_obj_t *dist_coeff_items;
    size_t dist_coeff_len;
    mp_obj_get_array(dist_coeffs_mp, &dist_coeff_len, &dist_coeff_items);

    float dist_coeffs[dist_coeff_len];  // 最多支持 8 个系数
    for (size_t i = 0; i < dist_coeff_len && i < 8; ++i) {
        dist_coeffs[i] = mp_obj_get_float(dist_coeff_items[i]);
    }

    int rects[4];
    int corners[8];

    float distance = rgb888_pnp_distance_from_corners(frame_shape, img_data, camera_matrix, dist_coeffs, dist_len, obj_width_cm, obj_height_cm, rects, corners);

    mp_obj_list_t *results_mp_list = mp_obj_new_list(0, NULL);
    mp_obj_list_append(results_mp_list, mp_obj_new_float(distance));
    mp_obj_list_t *results_mp_rect_list = mp_obj_new_list(0, NULL);
    mp_obj_list_append(results_mp_rect_list, mp_obj_new_int(rects[0]));
    mp_obj_list_append(results_mp_rect_list, mp_obj_new_int(rects[1]));
    mp_obj_list_append(results_mp_rect_list, mp_obj_new_int(rects[2]));
    mp_obj_list_append(results_mp_rect_list, mp_obj_new_int(rects[3]));
    mp_obj_list_append(results_mp_list, results_mp_rect_list);
    mp_obj_list_t *results_mp_tuple_corners_list = mp_obj_new_list(0, NULL);
    for (int i = 0; i < 4; i++)
    {
        mp_obj_list_t *corner = mp_obj_new_list(0, NULL);
        mp_obj_list_append(corner, mp_obj_new_int(corners[i * 2 + 0]));
        mp_obj_list_append(corner, mp_obj_new_int(corners[i * 2 + 1]));
        mp_obj_list_append(results_mp_tuple_corners_list, corner);
    }
    mp_obj_list_append(results_mp_list, results_mp_tuple_corners_list);
    return MP_OBJ_FROM_PTR(results_mp_list);
}
STATIC MP_DEFINE_CONST_FUN_OBJ_VAR_BETWEEN(cv_lite_rgb888_pnp_distance_from_corners_obj, 7, 7, cv_lite_rgb888_pnp_distance_from_corners);

STATIC mp_obj_t cv_lite_rgb888_perspective_transform(size_t n_args, const mp_obj_t *args) {
    // 参数解析
    mp_obj_list_t *frame_size_mp = MP_OBJ_TO_PTR(args[0]);  // [height, width]
    ndarray_obj_t *data = MP_ROM_PTR(args[1]);              // 图像数据
    uint8_t *img_data = data->array;

    FrameCHWSize frame_shape;
    frame_shape.height = mp_obj_get_int(frame_size_mp->items[0]);
    frame_shape.width  = mp_obj_get_int(frame_size_mp->items[1]);
    frame_shape.channel = 3;

    mp_obj_list_t *roi_mp = MP_OBJ_TO_PTR(args[2]);  // [x, y, width, height]
    int roi[4];
    roi[0] = mp_obj_get_int(roi_mp->items[0]);
    roi[1] = mp_obj_get_int(roi_mp->items[1]);
    roi[2] = mp_obj_get_int(roi_mp->items[2]);
    roi[3] = mp_obj_get_int(roi_mp->items[3]);

    mp_obj_list_t *dst_pts_mp = MP_OBJ_TO_PTR(args[3]);  // [x1, y1, x2, y2, x3, y3, x4, y4]
    float dst_pts[8];
    for (size_t i = 0; i < 8; ++i) {
        dst_pts[i] = mp_obj_get_float(dst_pts_mp->items[i]);
    }

    int out_width = mp_obj_get_int(args[4]);
    int out_height = mp_obj_get_int(args[5]);

    uint8_t *result = malloc(out_width * out_height * frame_shape.channel);

    rgb888_perspective_transform(frame_shape, img_data, roi, dst_pts, out_width, out_height, result);
    // 构造返回的 numpy 对象（共享内存，不复制）
    size_t ndarray_shape[4];
    ndarray_shape[0] = 1;
    ndarray_shape[1] = frame_shape.height;
    ndarray_shape[2] = frame_shape.width;
    ndarray_shape[3] = frame_shape.channel;
    ndarray_obj_t *out = ndarray_new_ndarray(4, ndarray_shape, NULL, NDARRAY_UINT8);
    uint8_t *out_data = (uint8_t *)out->array;
    memcpy(out_data, result, frame_shape.width * frame_shape.height * frame_shape.channel); // 独立返回一份数据
    free(result);
    return MP_OBJ_FROM_PTR(out);
}
STATIC MP_DEFINE_CONST_FUN_OBJ_VAR_BETWEEN(cv_lite_rgb888_perspective_transform_obj, 6, 6, cv_lite_rgb888_perspective_transform);

STATIC const mp_rom_map_elem_t cv_lite_globals_table[] = {
    { MP_ROM_QSTR(MP_QSTR___name__), MP_ROM_QSTR(MP_QSTR_cv_lite) },
    // 找色块
    { MP_ROM_QSTR(MP_QSTR_grayscale_find_blobs), MP_ROM_PTR(&cv_lite_grayscale_find_blobs_obj) },
    { MP_ROM_QSTR(MP_QSTR_rgb888_find_blobs), MP_ROM_PTR(&cv_lite_rgb888_find_blobs_obj) },
    // 找圆
    { MP_ROM_QSTR(MP_QSTR_grayscale_find_circles), MP_ROM_PTR(&cv_lite_grayscale_find_circles_obj) },
    { MP_ROM_QSTR(MP_QSTR_rgb888_find_circles), MP_ROM_PTR(&cv_lite_rgb888_find_circles_obj) },
    // 找矩形
    { MP_ROM_QSTR(MP_QSTR_grayscale_find_rectangles), MP_ROM_PTR(&cv_lite_grayscale_find_rectangles_obj) },
    { MP_ROM_QSTR(MP_QSTR_grayscale_find_rectangles_with_corners), MP_ROM_PTR(&cv_lite_grayscale_find_rectangles_with_corners_obj) },
    { MP_ROM_QSTR(MP_QSTR_rgb888_find_rectangles), MP_ROM_PTR(&cv_lite_rgb888_find_rectangles_obj) },
    { MP_ROM_QSTR(MP_QSTR_rgb888_find_rectangles_with_corners), MP_ROM_PTR(&cv_lite_rgb888_find_rectangles_with_corners_obj) },
    // 找直线或线段
    { MP_ROM_QSTR(MP_QSTR_grayscale_find_lines_raw), MP_ROM_PTR(&cv_lite_grayscale_find_lines_raw_obj) },
    { MP_ROM_QSTR(MP_QSTR_grayscale_find_lines), MP_ROM_PTR(&cv_lite_grayscale_find_lines_obj) },
    { MP_ROM_QSTR(MP_QSTR_rgb888_find_lines), MP_ROM_PTR(&cv_lite_rgb888_find_lines_obj) },
    { MP_ROM_QSTR(MP_QSTR_grayscale_find_lines_no_hough), MP_ROM_PTR(&cv_lite_grayscale_find_lines_no_hough_obj) },
    { MP_ROM_QSTR(MP_QSTR_grayscale_find_lines_sobel), MP_ROM_PTR(&cv_lite_grayscale_find_lines_sobel_obj) },
    // 边缘检测
    { MP_ROM_QSTR(MP_QSTR_grayscale_find_edges), MP_ROM_PTR(&cv_lite_grayscale_find_edges_obj) },
    { MP_ROM_QSTR(MP_QSTR_rgb888_find_edges), MP_ROM_PTR(&cv_lite_rgb888_find_edges_obj) },
    // 二值化
    { MP_ROM_QSTR(MP_QSTR_grayscale_threshold_binary), MP_ROM_PTR(&cv_lite_grayscale_threshold_binary_obj) },
    { MP_ROM_QSTR(MP_QSTR_rgb888_threshold_binary), MP_ROM_PTR(&cv_lite_rgb888_threshold_binary_obj) },
    //白平衡
    { MP_ROM_QSTR(MP_QSTR_rgb888_white_balance_gray_world), MP_ROM_PTR(&cv_lite_rgb888_white_balance_gray_world_obj) },
    { MP_ROM_QSTR(MP_QSTR_rgb888_white_balance_gray_world_fast), MP_ROM_PTR(&cv_lite_rgb888_white_balance_gray_world_fast_obj) },
    { MP_ROM_QSTR(MP_QSTR_rgb888_white_balance_gray_world_fast_ex), MP_ROM_PTR(&cv_lite_rgb888_white_balance_gray_world_fast_ex_obj) },
    { MP_ROM_QSTR(MP_QSTR_rgb888_white_balance_white_patch), MP_ROM_PTR(&cv_lite_rgb888_white_balance_white_patch_obj) },
    { MP_ROM_QSTR(MP_QSTR_rgb888_white_balance_white_patch_ex), MP_ROM_PTR(&cv_lite_rgb888_white_balance_white_patch_ex_obj) },
    { MP_ROM_QSTR(MP_QSTR_rgb888_white_balance_gray_world_adjustable), MP_ROM_PTR(&cv_lite_rgb888_white_balance_gray_world_adjustable_obj) },
    // 曝光调节
    { MP_ROM_QSTR(MP_QSTR_rgb888_adjust_exposure), MP_ROM_PTR(&cv_lite_rgb888_adjust_exposure_obj) },
    { MP_ROM_QSTR(MP_QSTR_rgb888_adjust_exposure_fast), MP_ROM_PTR(&cv_lite_rgb888_adjust_exposure_fast_obj) },
    //降噪滤波
    { MP_ROM_QSTR(MP_QSTR_rgb888_denoise), MP_ROM_PTR(&cv_lite_rgb888_denoise_obj) },
    { MP_ROM_QSTR(MP_QSTR_rgb888_denoise_fast), MP_ROM_PTR(&cv_lite_rgb888_denoise_fast_obj) },
    { MP_ROM_QSTR(MP_QSTR_rgb888_mean_blur), MP_ROM_PTR(&cv_lite_rgb888_mean_blur_obj) },
    { MP_ROM_QSTR(MP_QSTR_rgb888_mean_blur_fast), MP_ROM_PTR(&cv_lite_rgb888_mean_blur_fast_obj) },
    { MP_ROM_QSTR(MP_QSTR_rgb888_gaussian_blur_fast), MP_ROM_PTR(&cv_lite_rgb888_gaussian_blur_fast_obj) },
    // 形态学操作，腐蚀、膨胀、开运算、闭运算、形态学梯度、顶帽、黑帽
    { MP_ROM_QSTR(MP_QSTR_rgb888_erode), MP_ROM_PTR(&cv_lite_rgb888_erode_obj) },
    { MP_ROM_QSTR(MP_QSTR_rgb888_dilate), MP_ROM_PTR(&cv_lite_rgb888_dilate_obj) },
    { MP_ROM_QSTR(MP_QSTR_rgb888_open), MP_ROM_PTR(&cv_lite_rgb888_open_obj) },
    { MP_ROM_QSTR(MP_QSTR_rgb888_close), MP_ROM_PTR(&cv_lite_rgb888_close_obj) },
    { MP_ROM_QSTR(MP_QSTR_rgb888_gradient), MP_ROM_PTR(&cv_lite_rgb888_gradient_obj) },
    { MP_ROM_QSTR(MP_QSTR_rgb888_tophat), MP_ROM_PTR(&cv_lite_rgb888_tophat_obj) },
    { MP_ROM_QSTR(MP_QSTR_rgb888_blackhat), MP_ROM_PTR(&cv_lite_rgb888_blackhat_obj) },
    // 计算直方图
    { MP_ROM_QSTR(MP_QSTR_rgb888_calc_histogram), MP_ROM_PTR(&cv_lite_rgb888_calc_histogram_obj) },
    // 找角点
    { MP_ROM_QSTR(MP_QSTR_grayscale_find_corners), MP_ROM_PTR(&cv_lite_grayscale_find_corners_obj) },
    { MP_ROM_QSTR(MP_QSTR_rgb888_find_corners), MP_ROM_PTR(&cv_lite_rgb888_find_corners_obj) },
    { MP_ROM_QSTR(MP_QSTR_rgb888_find_corners_fast), MP_ROM_PTR(&cv_lite_rgb888_find_corners_fast_obj) },
    // 保存图像
    { MP_ROM_QSTR(MP_QSTR_save_image), MP_ROM_PTR(&cv_lite_save_image_obj) },
    // 读取图像
    { MP_ROM_QSTR(MP_QSTR_load_image), MP_ROM_PTR(&cv_lite_load_image_obj) },
    // 畸变修正
    { MP_ROM_QSTR(MP_QSTR_rgb888_undistort), MP_ROM_PTR(&cv_lite_rgb888_undistort_obj) },
    { MP_ROM_QSTR(MP_QSTR_rgb888_undistort_fast), MP_ROM_PTR(&cv_lite_rgb888_undistort_fast_obj) },
    { MP_ROM_QSTR(MP_QSTR_rgb888_undistort_new_cam_mat), MP_ROM_PTR(&cv_lite_rgb888_undistort_new_cam_mat_obj) },
    // 计算距离
    { MP_ROM_QSTR(MP_QSTR_rgb888_pnp_distance), MP_ROM_PTR(&cv_lite_rgb888_pnp_distance_obj) },
    { MP_ROM_QSTR(MP_QSTR_rgb888_pnp_distance_from_corners), MP_ROM_PTR(&cv_lite_rgb888_pnp_distance_from_corners_obj) },
    // 透视变换
    { MP_ROM_QSTR(MP_QSTR_rgb888_perspective_transform), MP_ROM_PTR(&cv_lite_rgb888_perspective_transform_obj) },
};

STATIC MP_DEFINE_CONST_DICT(cv_lite_globals, cv_lite_globals_table);

const mp_obj_module_t cv_lite = {
    .base = { &mp_type_module },
    .globals = (mp_obj_dict_t *)&cv_lite_globals,
};

MP_REGISTER_EXTENSIBLE_MODULE(MP_QSTR_cv_lite, cv_lite);