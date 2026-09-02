#include <opencv2/imgproc.hpp>
#include <vector>
#include <string>
#include <string.h>
#include <cmath>
#include <algorithm>
#include <stdlib.h>
#include <iostream>
#include <stdint.h>
// #include <opencv/cv.hpp>
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include "aidemo_wrap.h"
#include "aidemo_size.h"

using namespace std;


using std::vector;

uint8_t* body_seg_postprocess(float* data, int num_class, FrameSize ori_shape, FrameSize dst_shape,uint8_t* color)
{
    size_t ori_pixels;
    size_t dst_result_size;
    if (!aidemo_checked_image_size(ori_shape.width, ori_shape.height, 1, &ori_pixels)
        || !aidemo_checked_image_size(dst_shape.width, dst_shape.height, 4, &dst_result_size)) {
        return NULL;
    }
    (void)ori_pixels;

    float* output = data;
    cv::Mat images_pred_color = cv::Mat::zeros(ori_shape.height, ori_shape.width, CV_8UC4);
    for (int y = 0; y < ori_shape.height; ++y)
    {
        for (int x = 0; x < ori_shape.width; ++x)
        {
            float s = 0.0;
            float score0 = 0.0;
            int idx=0;
            int loc = num_class * (x + y * ori_shape.width);
            for (int c = 0; c < num_class; c++)
                s += exp(output[loc + c]);
            for (int c = 0; c < num_class; c++)
            {
                output[loc + c] = output[loc + c] / s;
                if(output[loc + c]>score0){
                    idx=c;
                    score0=output[loc + c];
                }

            }

            cv::Vec4b& color_xy = images_pred_color.at<cv::Vec4b>(cv::Point(x, y));
            if(idx>0){
                color_xy[0] = color[idx*4+0];
                color_xy[1] = color[idx*4+1];
                color_xy[2] = color[idx*4+2];
                color_xy[3] = color[idx*4+3];
            }

        }
    }
    cv::resize(images_pred_color, images_pred_color, cv::Size(dst_shape.width, dst_shape.height));

    uint8_t *result = (uint8_t *)malloc(dst_result_size);
    if (result == NULL && dst_result_size != 0) {
        return NULL;
    }
    if (dst_result_size != 0) {
        hal_rvv_memcpy(result, images_pred_color.data, dst_result_size);
    }
    return result;
}

void body_seg_free_output(void *context)
{
    free(context);
}
