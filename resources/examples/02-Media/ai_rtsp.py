from libs.PipeLine import PipeLine
from libs.AIBase import AIBase
from libs.AI2D import Ai2d
from libs.Utils import *
from libs.WBCRtsp import WBCRtsp
import os,sys,ujson,gc,math
from media.media import *
import nncase_runtime as nn
import ulab.numpy as np
import image
import aidemo
import network,time


# Select "wifi_sta", "wifi_ap", or "lan".
NETWORK_MODE = "wifi_sta"
NETWORK_TIMEOUT = 15

# Default test credentials. Change them before running the example.
WIFI_STA_SSID = "Test"
WIFI_STA_PASSWORD = "12345678"

WIFI_AP_SSID = "K230_RTSP"
WIFI_AP_PASSWORD = "12345678"

NETWORK_DEVICE_NAMES = {
    "wifi_sta": "w0",
    "wifi_ap": "w1",
    "lan": "u0",
}


def wait_for_ip(nic, timeout=NETWORK_TIMEOUT, require_connection=False):
    start_time = time.time()
    while time.time() - start_time < timeout:
        config = nic.ifconfig()
        connected = not require_connection or nic.isconnected()
        if connected and config and config[0] != "0.0.0.0":
            return config[0]
        time.sleep(0.2)
    raise RuntimeError("Network did not obtain an IP address")


def require_network_device(mode):
    if not hasattr(network, "get_dev_list"):
        raise RuntimeError("Network device discovery is not supported by this firmware")

    devices = network.get_dev_list()
    device_name = NETWORK_DEVICE_NAMES[mode]
    if devices is None or device_name not in devices:
        raise RuntimeError("Network device '%s' is not available; found: %s" %
                           (device_name, devices))


def connect_wifi_sta(ssid, password, timeout=NETWORK_TIMEOUT):
    if not hasattr(network, "WLAN"):
        raise RuntimeError("Wi-Fi is not supported by this firmware")

    wlan = network.WLAN(network.STA_IF)
    wlan.active(True)

    # A WLAN connection survives a script restart. Disconnect first so new
    # credentials can switch the station to a different access point.
    if wlan.isconnected():
        print("Disconnecting current Wi-Fi...")
        if not wlan.disconnect():
            raise RuntimeError("Wi-Fi disconnect failed")
        start_time = time.time()
        while wlan.isconnected():
            if time.time() - start_time >= 5:
                raise RuntimeError("Wi-Fi disconnect timeout")
            os.exitpoint()
            time.sleep_ms(100)

    print("Connecting to Wi-Fi access point...")
    if wlan.connect(ssid, password) is False:
        raise RuntimeError("Failed to start Wi-Fi connection")

    ip = wait_for_ip(wlan, timeout, True)
    print("Wi-Fi STA network information:", wlan.ifconfig())
    return wlan, ip


def start_wifi_ap(ssid, password, timeout=NETWORK_TIMEOUT):
    if not hasattr(network, "WLAN"):
        raise RuntimeError("Wi-Fi is not supported by this firmware")

    ap = network.WLAN(network.AP_IF)
    print("Starting Wi-Fi access point:", ssid)
    if ap.config(ssid=ssid, key=password) is False:
        raise RuntimeError("Failed to start Wi-Fi access point")

    ip = wait_for_ip(ap, timeout)
    print("Wi-Fi AP network information:", ap.ifconfig())
    return ap, ip


def connect_lan(timeout=NETWORK_TIMEOUT):
    if not hasattr(network, "LAN"):
        raise RuntimeError("LAN is not supported by this firmware")

    lan = network.LAN()
    print("Connecting LAN with DHCP...")
    if lan.ifconfig("dhcp") is False:
        raise RuntimeError("Failed to start LAN DHCP")
    ip = wait_for_ip(lan, timeout, True)
    print("LAN network information:", lan.ifconfig())
    return lan, ip


def connect_network(mode):
    if mode not in NETWORK_DEVICE_NAMES:
        raise ValueError("NETWORK_MODE must be 'wifi_sta', 'wifi_ap', or 'lan'")

    require_network_device(mode)
    if mode == "wifi_sta":
        nic, ip = connect_wifi_sta(WIFI_STA_SSID, WIFI_STA_PASSWORD)
    elif mode == "wifi_ap":
        nic, ip = start_wifi_ap(WIFI_AP_SSID, WIFI_AP_PASSWORD)
    else:
        nic, ip = connect_lan()

    if not hasattr(network, "set_default_dev"):
        raise RuntimeError("Default network device selection is not supported by this firmware")
    if network.set_default_dev(NETWORK_DEVICE_NAMES[mode]) is False:
        raise RuntimeError("Failed to select network device '%s'" % NETWORK_DEVICE_NAMES[mode])

    return nic, ip

# 自定义人脸检测类，继承自AIBase基类
class FaceDetectionApp(AIBase):
    def __init__(self, kmodel_path, model_input_size, anchors, confidence_threshold=0.5, nms_threshold=0.2, rgb888p_size=[224,224], display_size=[1920,1080], debug_mode=0):
        super().__init__(kmodel_path, model_input_size, rgb888p_size, debug_mode)  # 调用基类的构造函数
        self.kmodel_path = kmodel_path  # 模型文件路径
        self.model_input_size = model_input_size  # 模型输入分辨率
        self.confidence_threshold = confidence_threshold  # 置信度阈值
        self.nms_threshold = nms_threshold  # NMS（非极大值抑制）阈值
        self.anchors = anchors  # 锚点数据，用于目标检测
        self.rgb888p_size = [ALIGN_UP(rgb888p_size[0], 16), rgb888p_size[1]]  # sensor给到AI的图像分辨率，并对宽度进行16的对齐
        self.display_size = [ALIGN_UP(display_size[0], 16), display_size[1]]  # 显示分辨率，并对宽度进行16的对齐
        self.debug_mode = debug_mode  # 是否开启调试模式
        self.ai2d = Ai2d(debug_mode)  # 实例化Ai2d，用于实现模型预处理
        self.ai2d.set_ai2d_dtype(nn.ai2d_format.NCHW_FMT, nn.ai2d_format.NCHW_FMT, np.uint8, np.uint8)  # 设置Ai2d的输入输出格式和类型

    # 配置预处理操作，这里使用了pad和resize，Ai2d支持crop/shift/pad/resize/affine，具体代码请打开/sdcard/app/libs/AI2D.py查看
    def config_preprocess(self, input_image_size=None):
        with ScopedTiming("set preprocess config", self.debug_mode > 0):  # 计时器，如果debug_mode大于0则开启
            ai2d_input_size = input_image_size if input_image_size else self.rgb888p_size  # 初始化ai2d预处理配置，默认为sensor给到AI的尺寸，可以通过设置input_image_size自行修改输入尺寸
            top, bottom, left, right,_ =letterbox_pad_param(self.rgb888p_size,self.model_input_size)
            self.ai2d.pad([0, 0, 0, 0, top, bottom, left, right], 0, [104, 117, 123])  # 填充边缘
            self.ai2d.resize(nn.interp_method.tf_bilinear, nn.interp_mode.half_pixel)  # 缩放图像
            self.ai2d.build([1,3,ai2d_input_size[1],ai2d_input_size[0]],[1,3,self.model_input_size[1],self.model_input_size[0]])  # 构建预处理流程

    # 自定义当前任务的后处理，results是模型输出array列表，这里使用了aidemo库的face_det_post_process接口
    def postprocess(self, results):
        with ScopedTiming("postprocess", self.debug_mode > 0):
            post_ret = aidemo.face_det_post_process(self.confidence_threshold, self.nms_threshold, self.model_input_size[1], self.anchors, self.rgb888p_size, results)
            if len(post_ret) == 0:
                return post_ret
            else:
                return post_ret[0]

    # 绘制检测结果到画面上
    def draw_result(self, pl, dets):
        with ScopedTiming("display_draw", self.debug_mode > 0):
            if dets:
                pl.osd_img.clear()  # 清除OSD图像
                for det in dets:
                    # 将检测框的坐标转换为显示分辨率下的坐标
                    x, y, w, h = map(lambda x: int(round(x, 0)), det[:4])
                    x = x * self.display_size[0] // self.rgb888p_size[0]
                    y = y * self.display_size[1] // self.rgb888p_size[1]
                    w = w * self.display_size[0] // self.rgb888p_size[0]
                    h = h * self.display_size[1] // self.rgb888p_size[1]
                    pl.osd_img.draw_rectangle(x, y, w, h, color=(255, 255, 0, 255), thickness=2)  # 绘制矩形框
            else:
                pl.osd_img.clear()

if __name__ == "__main__":
    # 添加显示模式，默认hdmi，可选hdmi/lcd/lt9611/st7701/hx8399/nt35516/nt35532/gc9503/aml020t/jd9852/ili9806/virt；其中hdmi默认对应lt9611，lcd默认对应st7701
    display_mode="lcd"
    # 显示分辨率，None表示使用当前显示屏默认分辨率；使用virt时可在这里手动设置，例如[800, 480]
    display_size=None
    # k230保持不变，k230d可调整为[640,360]
    rgb888p_size = [1280, 720]
    # 设置模型路径和其他参数
    kmodel_path = "/sdcard/examples/kmodel/face_detection_320.kmodel"
    # 其它参数
    confidence_threshold = 0.5
    nms_threshold = 0.2
    anchor_len = 4200
    det_dim = 4
    anchors_path = "/sdcard/examples/utils/prior_data_320.bin"
    anchors = np.fromfile(anchors_path, dtype=np.float)
    anchors = anchors.reshape((anchor_len, det_dim))

    nic, network_ip = connect_network(NETWORK_MODE)

    print("Virtual WBC AI + RTSP stream: rtsp://%s:8554/test" % network_ip)

    # 初始化PipeLine，rgb888p_size为传给AI的图像分辨率，display_size为显示分辨率
    pl = PipeLine(rgb888p_size=rgb888p_size, display_mode=display_mode, display_size=display_size)
    # 创建PipeLine，可按需传入sensor_id选择摄像头，例如pl.create(sensor_id=2)
    pl.create(to_ide=False)  # 创建PipeLine实例
    # init wbc,wbc_width和wbc_height为原始屏幕的宽高
    WBCRtsp.configure(wbc_width=480,wbc_height=800)
    # 启用wbc编码推流
    WBCRtsp.start(network_ip)

    display_size=pl.get_display_size()
    # 初始化自定义人脸检测实例
    face_det = FaceDetectionApp(kmodel_path, model_input_size=[320, 320], anchors=anchors, confidence_threshold=confidence_threshold, nms_threshold=nms_threshold, rgb888p_size=rgb888p_size, display_size=display_size, debug_mode=0)
    face_det.config_preprocess()  # 配置预处理

    try:
        while True:
            img = pl.get_frame()            # 获取当前帧数据
            res = face_det.run(img)         # 推理当前帧
            face_det.draw_result(pl, res)   # 绘制结果
            pl.show_image()                 # 显示结果
            gc.collect()                    # 垃圾回收
    except KeyboardInterrupt as e:
        print("user stop: ", e)
    except BaseException as e:
        import sys
        sys.print_exception(e)

    face_det.deinit()                       # 反初始化
    WBCRtsp.stop()                         # 停止WBC推流
    pl.destroy()                            # 销毁PipeLine实例
