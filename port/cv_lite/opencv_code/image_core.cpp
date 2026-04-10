#include <vector>
#include <opencv2/highgui.hpp>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/opencv.hpp>
#include "cv_lite_wrap.h"
#include <sys/time.h>  // gettimeofday
#include <stdio.h>     // printf
#include <string>
#include <iostream>

#include <riscv_vector.h>

#include "hal_rvv_ops.h"

#define MIN(a,b) ((a)<(b)?(a):(b))
#define MAX(a,b) ((a)>(b)?(a):(b))

using std::vector;
using namespace std;
using namespace cv;

uint8_t* load_image(const char* load_path,FrameCHWSize* image_frame_shape){
    Mat image = imread(load_path, IMREAD_COLOR);
    if (image.empty()) {
        printf("load image failed: %s\n", load_path);
        return nullptr;
    }
    image_frame_shape->height = image.rows;
    image_frame_shape->width = image.cols;
    image_frame_shape->channel = image.channels();
    // 转换为BGR格式
    cvtColor(image, image, COLOR_RGB2BGR);
    // 分配内存并复制数据
    uint8_t* img_data = (uint8_t*)malloc(image_frame_shape->height * image_frame_shape->width * image_frame_shape->channel * sizeof(uint8_t));
    hal_rvv_memcpy(img_data, image.data, image_frame_shape->height * image_frame_shape->width * image_frame_shape->channel * sizeof(uint8_t));
    // 释放OpenCV分配的内存
    image.release();
    // 返回数据指针
    return img_data;
}