import os
import ujson
from media.sensor import *
from media.display import *
from media.media import *
from libs.Utils import ScopedTiming
import nncase_runtime as nn
import ulab.numpy as np
import image
import gc
import sys
import time

# PipeLine类
class PipeLine:
    def __init__(self,rgb888p_size=[224,224],display_mode="hdmi",display_size=None,osd_layer_num=1,debug_mode=0):
        # sensor给AI的图像分辨率
        self.rgb888p_size=[ALIGN_UP(rgb888p_size[0],16),rgb888p_size[1]]
        # 视频输出VO图像分辨率
        if display_size is None:
            self.display_size=None
        else:
            self.display_size=[display_size[0],display_size[1]]
        # 视频显示模式，支持："lcd"(default st7701 800*480)，"hdmi"(default lt9611)，"lt9611"，"st7701"，"hx8399", "nt35516", "nt35532", "gc9503"
        self.display_mode=display_mode
        # sensor对象
        self.sensor=None
        # osd显示Image对象
        self.osd_img=None
        self.cur_frame=None
        self.debug_mode=debug_mode
        self.osd_layer_num = osd_layer_num

    # PipeLine初始化函数
    def create(self,sensor=None,hmirror=None,vflip=None,fps=60,to_ide=True):
        with ScopedTiming("init PipeLine",self.debug_mode > 0):
            nn.shrink_memory_pool()
            if self.display_mode=="nt35516":
                fps=30
            # 初始化并配置sensor
            brd=os.uname()[-1]
            if brd=="k230d_canmv_bpi_zero":
                self.sensor = Sensor(fps=30) if sensor is None else sensor
            elif brd=="k230_canmv_lckfb":
                self.sensor = Sensor(fps=30) if sensor is None else sensor
            elif brd=="k230d_canmv_atk_dnk230d":
                self.sensor = Sensor(fps=30) if sensor is None else sensor
            else:
                self.sensor = Sensor(fps=fps) if sensor is None else sensor
            self.sensor.reset()
            if hmirror is not None and (hmirror==True or hmirror==False):
                self.sensor.set_hmirror(hmirror)
            if vflip is not None and (vflip==True or vflip==False):
                self.sensor.set_vflip(vflip)

            DISPLAY_MAP = {
                "hdmi":     Display.LT9611,
                "lt9611":   Display.LT9611,
                "lcd":      Display.ST7701,
                "st7701":   Display.ST7701,
                "hx8399":   Display.HX8399,
                "nt35516":  Display.NT35516,
                "nt35532":  Display.NT35532,
                "gc9503":   Display.GC9503,
            }

            # Look up type, fallback to ST7701 if not found
            display_type = DISPLAY_MAP.get(self.display_mode, Display.ST7701)

            # Call init
            if self.display_size:
                Display.init(
                    display_type,
                    width=self.display_size[0],
                    height=self.display_size[1],
                    osd_num=self.osd_layer_num,
                    to_ide=to_ide
                )
            else:
                Display.init(
                    display_type,
                    osd_num=self.osd_layer_num,
                    to_ide=to_ide
                )

            # Update actual size after init
            self.display_size = [Display.width(), Display.height()]

            # 通道0直接给到显示VO，格式为YUV420
            self.sensor.set_framesize(w = self.display_size[0], h = self.display_size[1])
            self.sensor.set_pixformat(PIXEL_FORMAT_YUV_SEMIPLANAR_420)
            # 通道2给到AI做算法处理，格式为RGB888
            self.sensor.set_framesize(w = self.rgb888p_size[0], h = self.rgb888p_size[1], chn=CAM_CHN_ID_2)
            # set chn2 output format
            self.sensor.set_pixformat(PIXEL_FORMAT_RGB_888_PLANAR, chn=CAM_CHN_ID_2)

            # OSD图像初始化
            self.osd_img = image.Image(self.display_size[0], self.display_size[1], image.ARGB8888)

            sensor_bind_info = self.sensor.bind_info(x = 0, y = 0, chn = CAM_CHN_ID_0)
            Display.bind_layer(**sensor_bind_info, layer = Display.LAYER_VIDEO1)

            # media初始化
            MediaManager.init()
            # 启动sensor
            self.sensor.run()

    # 获取一帧图像数据，返回格式为ulab的array数据
    def get_frame(self):
        with ScopedTiming("get a frame",self.debug_mode > 0):
            self.cur_frame = self.sensor.snapshot(chn=CAM_CHN_ID_2)
            input_np=self.cur_frame.to_numpy_ref()
            return input_np

    # 在屏幕上显示osd_img
    def show_image(self):
        with ScopedTiming("show result",self.debug_mode > 0):
            Display.show_image(self.osd_img, 0, 0, Display.LAYER_OSD3)

    def get_display_size(self):
        return self.display_size

    # PipeLine销毁函数
    def destroy(self):
        with ScopedTiming("deinit PipeLine",self.debug_mode > 0):
            os.exitpoint(os.EXITPOINT_ENABLE_SLEEP)
            # stop sensor
            self.sensor.stop()
            # deinit lcd
            Display.deinit()
            time.sleep_ms(50)
            # deinit media buffer
            MediaManager.deinit()
