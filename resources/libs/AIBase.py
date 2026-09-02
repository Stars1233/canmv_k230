from libs.PipeLine import ScopedTiming
import os
import ujson
from media.sensor import *
from media.display import *
from media.media import *
from time import *
import nncase_runtime as nn
import ulab.numpy as np
import time
import utime
import image
import random
import gc
import sys

# AIBase类别主要抽象的是AI任务推理流程
class AIBase:
    def __init__(self,kmodel_path,model_input_size=None,rgb888p_size=None,debug_mode=0):
        # kmodel路径
        self.kmodel_path=kmodel_path
        # 模型输入分辨率
        self.model_input_size=model_input_size
        # sensor给到AI的图像分辨率
        self.rgb888p_size=rgb888p_size
        # 调试模式
        self.debug_mode=debug_mode
        # kpu对象
        self.kpu=nn.kpu()
        self.kpu.load_kmodel(self.kmodel_path)
        self.cur_img=None
        self.tensors=[]
        # 推理结果列表
        self.results=[]

    def get_kmodel_inputs_num(self):
        return self.kpu.inputs_size()
    
    def get_kmodel_outputs_num(self):
        return self.kpu.outputs_size()
    
    def preprocess(self,input_np):
        with ScopedTiming("preprocess",self.debug_mode > 0):
            return [self.ai2d.run(input_np)]

    def inference(self,tensors):
        with ScopedTiming("set input",self.debug_mode > 0):
            self.results.clear()
            for i in range(self.kpu.inputs_size()):
                # 将ai2d的输出tensor绑定为kmodel的输入数据
                self.kpu.set_input_tensor(i, tensors[i])
        with ScopedTiming("kpu run",self.debug_mode > 0):
            # 运行kmodel做推理
            self.kpu.run()
        with ScopedTiming("get output",self.debug_mode > 0):
            # 获取kmodel的推理输出tensor,输出可能为多个，因此返回的是一个列表
            for i in range(self.kpu.outputs_size()):
                output_data = self.kpu.get_output_tensor(i)
                try:
                    result = output_data.to_numpy()
                    self.results.append(result)
                finally:
                    release = getattr(output_data, "release", None)
                    if callable(release):
                        try:
                            release()
                        except Exception:
                            pass
                    output_data = None
            return self.results

    # 基类后处理接口
    def postprocess(self,results):
        return

    # kmodel运行pipe，包括预处理+推理+后处理，后处理在单独的任务类中实现
    def run(self,input_np):
        self.cur_img=input_np
        self.tensors.clear()
        self.tensors=self.preprocess(input_np)
        self.results=self.inference(self.tensors)
        return self.postprocess(self.results)

    # AIBase销毁函数
    def deinit(self):
        if getattr(self, "_deinitialized", False):
            return
        with ScopedTiming("deinit",self.debug_mode > 0):
            self.cur_img = None

            if hasattr(self, "tensors"):
                self.tensors.clear()
                self.tensors = []
            if hasattr(self, "results"):
                self.results.clear()
                self.results = []
            if hasattr(self, "masks"):
                self.masks = None

            # The interpreter keeps references to its bound input/output
            # tensors. Drop it before releasing the Ai2d-owned tensors.
            kpu = getattr(self, "kpu", None)
            self.kpu = None
            cleanup_error = None
            if kpu is not None:
                try:
                    kpu.release()
                except Exception as error:
                    self.kpu = kpu
                    cleanup_error = error

            ai2d = getattr(self, "ai2d", None)
            self.ai2d = None
            if ai2d is not None:
                try:
                    deinit = getattr(ai2d, "deinit", None)
                    if callable(deinit):
                        deinit()
                except Exception as error:
                    self.ai2d = ai2d
                    if cleanup_error is None:
                        cleanup_error = error
            try:
                gc.collect()
                nn.shrink_memory_pool()
                gc.collect()
            except Exception as error:
                if cleanup_error is None:
                    cleanup_error = error
            time.sleep_ms(100)
            if cleanup_error is not None:
                raise cleanup_error
            self._deinitialized = True
