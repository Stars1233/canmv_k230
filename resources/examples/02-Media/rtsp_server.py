# Description: This example demonstrates how to stream video and audio to the network using the RTSP server.
#
# Note: You will need an SD card to run this example.
#
# You can run the rtsp server to stream video and audio to the network

from media.vencoder import *
from media.sensor import *
from media.media import *
import time, os
import network
import _thread
import multimedia as mm

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


class RtspServer:
    def __init__(self,session_name="test",port=8554,video_type = mm.multi_media_type.media_h264,enable_audio=False):
        self.session_name = session_name # session name
        self.video_type = video_type  # 视频类型264/265
        self.enable_audio = enable_audio # 是否启用音频
        self.port = port   #rtsp 端口号
        self.rtspserver = mm.rtsp_server() # 实例化rtsp server
        self.start_stream = False #是否启动推流线程
        self.runthread_over = False #推流线程是否结束

    def start(self):
        if self.rtspserver.rtspserver_init(self.port) != 0:
            raise RuntimeError("RTSP server failed to bind port %d" % self.port)

        try:
            # 创建session
            if self.rtspserver.rtspserver_createsession(self.session_name,self.video_type,self.enable_audio) != 0:
                raise RuntimeError("RTSP session creation failed")
            # 初始化推流
            self._init_stream()
            # 启动rtsp server
            self.rtspserver.rtspserver_start()
            self._start_stream()
        except:
            self.rtspserver.rtspserver_deinit()
            raise

        # 启动推流线程
        self.start_stream = True
        _thread.start_new_thread(self._do_rtsp_stream,())


    def stop(self):
        if (self.start_stream == False):
            return
        # 等待推流线程退出
        self.start_stream = False
        while not self.runthread_over:
            time.sleep(0.1)
        self.runthread_over = False

        # 停止推流
        self._stop_stream()
        self.rtspserver.rtspserver_stop()
        #self.rtspserver.rtspserver_destroysession(self.session_name)
        self.rtspserver.rtspserver_deinit()

    def get_rtsp_url(self, host=None):
        if host is not None:
            return "rtsp://%s:%d/%s" % (host, self.port, self.session_name)
        return self.rtspserver.rtspserver_getrtspurl(self.session_name)

    def _init_stream(self):
        width = 1280
        height = 720
        width = ALIGN_UP(width, 16)
        # 初始化sensor
        self.sensor = Sensor()
        self.sensor.reset()
        self.sensor.set_framesize(width = width, height = height, alignment=12)
        self.sensor.set_pixformat(Sensor.YUV420SP)
        # 实例化video encoder
        self.encoder = Encoder()
        self.encoder.SetOutBufs(8, width, height)
        # 创建编码器
        chnAttr = ChnAttrStr(self.encoder.PAYLOAD_TYPE_H264, self.encoder.H264_PROFILE_MAIN, width, height)
        self.encoder.Create(chnAttr)
        # 绑定camera和venc
        self.link = MediaManager.link(self.sensor.bind_info()['src'], (VIDEO_ENCODE_MOD_ID, VENC_DEV_ID, self.encoder.chn))

    def _start_stream(self):
        # 开始编码
        self.encoder.Start()
        # 启动camera
        self.sensor.run()

    def _stop_stream(self):
        # 停止camera
        self.sensor.stop()
        # 接绑定camera和venc
        del self.link
        # 停止编码
        self.encoder.Stop()
        self.encoder.Destroy()

    def _do_rtsp_stream(self):
        try:
            streamData = StreamData()
            while self.start_stream:
                os.exitpoint()
                # 获取一帧码流
                self.encoder.GetStream(streamData)
                # 推流
                for pack_idx in range(0, streamData.pack_cnt):
                    stream_data = bytes(uctypes.bytearray_at(streamData.data[pack_idx], streamData.data_size[pack_idx]))
                    self.rtspserver.rtspserver_sendvideodata(self.session_name,stream_data, streamData.data_size[pack_idx],1000)
                    #print("stream size: ", streamData.data_size[pack_idx], "stream type: ", streamData.stream_type[pack_idx])
                # 释放一帧码流
                self.encoder.ReleaseStream(streamData)

        except BaseException as e:
            import sys
            sys.print_exception(e)
        finally:
            self.runthread_over = True
            # 停止rtsp server
            self.stop()

        self.runthread_over = True

if __name__ == "__main__":
    os.exitpoint(os.EXITPOINT_ENABLE)
    # Connect the selected network interface before opening the RTSP socket.
    nic, network_ip = connect_network(NETWORK_MODE)
    # 创建rtsp server对象
    rtspserver = RtspServer()
    # 启动rtsp server
    rtspserver.start()
    # 打印rtsp url
    print("RTSP server started:", rtspserver.get_rtsp_url(network_ip))
    # 推流60s
    time.sleep(60)
    # 停止rtsp server
    rtspserver.stop()
    print("done")
