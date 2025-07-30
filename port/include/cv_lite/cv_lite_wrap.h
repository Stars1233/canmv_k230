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
#ifndef _CV_LITE_WRAP_H_
#define _CV_LITE_WRAP_H_

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include "cv_lite_type.h"

//for common
typedef struct FrameCHWSize FrameCHWSize;
typedef struct ImageData ImageData;

#ifdef __cplusplus
extern "C" {
#endif
    //for opencv
    // 找色块
    int* grayscale_find_blobs(FrameCHWSize frame_shape, uint8_t* data,int threshold_min, int threshold_max,int min_area, int kernel_size,int* ret_num);
    int* rgb888_find_blobs(FrameCHWSize frame_shape, uint8_t* data, int threshold[6], int min_area, int kernel_size, int* ret_num);
    // 找圆
    int* grayscale_find_circles(FrameCHWSize frame_shape, uint8_t* data, int dp, int minDist, int param1, int param2, int minRadius, int maxRadius, int* ret_num);
    int* rgb888_find_circles(FrameCHWSize frame_shape, uint8_t* data, int dp, int minDist, int param1, int param2, int minRadius, int maxRadius, int* ret_num);
    // 找矩形
    int* grayscale_find_rectangles(FrameCHWSize frame_shape, uint8_t* data,int canny_thresh1, int canny_thresh2,float approx_epsilon_ratio,float area_min_ratio,float max_angle_cos,int gaussian_blur_size,int* ret_num);
    int* rgb888_find_rectangles(FrameCHWSize frame_shape, uint8_t* data, int canny_thresh1, int canny_thresh2,float approx_epsilon_ratio,float area_min_ratio,float max_angle_cos,int gaussian_blur_size, int* ret_num);
    // 找直线或线段
    int* grayscale_find_lines_raw(FrameCHWSize frame_shape, uint8_t* data,int x_stride, int y_stride,int sobel_thresh,float rho_step, float theta_step,int hough_thresh,int* ret_num);
    int* grayscale_find_lines(FrameCHWSize frame_shape, uint8_t* data,int canny_thresh1, int canny_thresh2,int gaussian_blur_size,float rho, float theta,int hough_thresh, float min_line_length,float max_line_gap,int* ret_num);
    int* rgb888_find_lines(FrameCHWSize frame_shape, uint8_t* data,int canny_thresh1, int canny_thresh2,int gaussian_blur_size,float rho, float theta,int hough_thresh, float min_line_length,float max_line_gap,int* ret_num);
    int* grayscale_find_lines_no_hough(FrameCHWSize frame_shape, uint8_t* data,int canny_thresh1, int canny_thresh2,int gaussian_blur_size,float min_contour_len,int* ret_num);
    int* grayscale_find_lines_sobel(FrameCHWSize frame_shape, uint8_t* data,int sobel_thresh,int gaussian_blur_size,float rho, float theta,int hough_thresh, float min_line_length,float max_line_gap,int* ret_num);
    // 边缘检测
    void grayscale_find_edges(FrameCHWSize frame_shape, uint8_t* data,int threshold1, int threshold2,uint8_t* result);
    void rgb888_find_edges(FrameCHWSize frame_shape, uint8_t* data,int threshold1, int threshold2,uint8_t* result);
    // 二值化
    void grayscale_threshold_binary(FrameCHWSize frame_shape, uint8_t* data, int thresh, int maxval, uint8_t* result);
    void rgb888_threshold_binary(FrameCHWSize frame_shape, uint8_t* data, int thresh, int maxval, uint8_t* result);
    // 白平衡算法
    void rgb888_white_balance_gray_world(FrameCHWSize frame_shape, uint8_t* data, uint8_t* result);
    void rgb888_white_balance_gray_world_fast(FrameCHWSize frame_shape, uint8_t* data, uint8_t* result);
    void rgb888_white_balance_gray_world_fast_ex(FrameCHWSize frame_shape, uint8_t* data, uint8_t* result,float gain_clip, float brightness_boost);
    void rgb888_white_balance_white_patch(FrameCHWSize frame_shape, uint8_t* data, uint8_t* result);
    void rgb888_white_balance_white_patch_ex(FrameCHWSize frame_shape, uint8_t* data, uint8_t* result,float top_percent, float gain_clip, float brightness_boost);
    void rgb888_white_balance_gray_world_adjustable(FrameCHWSize frame_shape, uint8_t* data, float alpha, uint8_t* result);
    // 调整曝光
    void rgb888_adjust_exposure(FrameCHWSize frame_shape, uint8_t* data, float exposure_gain, uint8_t* result);
    void rgb888_adjust_exposure_fast(FrameCHWSize frame_shape, uint8_t* data, float exposure_gain, uint8_t* result);
    // 降噪滤波
    void rgb888_denoise(FrameCHWSize frame_shape, uint8_t* data, int method, int strength, uint8_t* result);
    void rgb888_mean_blur(FrameCHWSize frame_shape, uint8_t* data, int kernel_size, uint8_t* result);
    void rgb888_mean_blur_fast(FrameCHWSize frame_shape, uint8_t* data, int kernel_size, uint8_t* result);
    void rgb888_gaussian_blur_fast(FrameCHWSize frame_shape, uint8_t* data, int kernel_size, uint8_t* result);
    void rgb888_denoise_fast(FrameCHWSize frame_shape, uint8_t* data, int method, int strength, uint8_t* result);
    // 形态学操作，腐蚀、膨胀、开闭运算、形态学梯度、顶帽、黑帽
    void rgb888_erode(FrameCHWSize frame_shape, uint8_t* data, int kernel_size, int iterations, int threshold_value, uint8_t* result);
    void rgb888_dilate(FrameCHWSize frame_shape, uint8_t* data, int kernel_size, int iterations, int threshold_value, uint8_t* result);
    void rgb888_open(FrameCHWSize frame_shape, uint8_t* data, int kernel_size, int iterations,int threshold_value, uint8_t* result);
    void rgb888_close(FrameCHWSize frame_shape, uint8_t* data, int kernel_size, int iterations, int threshold_value,uint8_t* result);
    void rgb888_gradient(FrameCHWSize frame_shape, uint8_t* data, int kernel_size, int iterations, int threshold_value,uint8_t* result);
    void rgb888_tophat(FrameCHWSize frame_shape, uint8_t* data, int kernel_size, int iterations,int threshold_value,  uint8_t* result);
    void rgb888_blackhat(FrameCHWSize frame_shape, uint8_t* data, int kernel_size, int iterations, int threshold_value, uint8_t* result);
    // 直方图统计
    void rgb888_calc_histogram(FrameCHWSize frame_shape, uint8_t* data, uint32_t* result);

    int* rgb888_find_corners(FrameCHWSize frame_shape, uint8_t* data,int maxCorners, float qualityLevel, float minDistance,int* ret_num);
    int* rgb888_find_corners_fast(FrameCHWSize frame_shape, uint8_t* data, int maxCorners,float qualityLevel, float minDistance, int* ret_num);

    // void calibrate_camera(char* images_path,int board_width,int board_height,float square_size,float* camera_matrix,float* dist_coeffs);

#ifdef __cplusplus
}
#endif

#endif // _CV_LITE_WRAP_H_