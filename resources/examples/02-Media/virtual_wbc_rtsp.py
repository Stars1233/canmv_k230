import time, os, urandom, sys, network

from media.display import *
from media.media import *
from libs.WBCRtsp import WBCRtsp

DISPLAY_WIDTH = ALIGN_UP(1920, 16)
DISPLAY_HEIGHT = 1080

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


def display_test(network_ip):
    print("Virtual WBC RTSP stream: rtsp://%s:8554/test" % network_ip)

    # create image for drawing
    img = image.Image(DISPLAY_WIDTH, DISPLAY_HEIGHT, image.ARGB8888)

    # use lcd as display output
    Display.init(Display.VIRT, width = DISPLAY_WIDTH, height = DISPLAY_HEIGHT, fps = 60,to_ide=False)
    # init wbc
    WBCRtsp.configure(wbc_width=DISPLAY_WIDTH,wbc_height=DISPLAY_HEIGHT)
    # 启用wbc编码推流
    WBCRtsp.start(network_ip)

    try:
        while True:
            img.clear()
            for i in range(10):
                x = (urandom.getrandbits(11) % img.width())
                y = (urandom.getrandbits(11) % img.height())
                r = (urandom.getrandbits(8))
                g = (urandom.getrandbits(8))
                b = (urandom.getrandbits(8))
                size = (urandom.getrandbits(30) % 64) + 32
                # If the first argument is a scaler then this method expects
                # to see x, y, and text. Otherwise, it expects a (x,y,text) tuple.
                # Character and string rotation can be done at 0, 90, 180, 270, and etc. degrees.
                img.draw_string_advanced(x,y,size, "Hello World!，你好世界！！！", color = (r, g, b),)

            # draw result to screen
            Display.show_image(img)

            time.sleep(1)
            os.exitpoint()
    except KeyboardInterrupt as e:
        print("user stop: ", e)
    except BaseException as e:
        import sys
        sys.print_exception(e)

    WBCRtsp.stop()  # stop wbc
    # deinit display
    Display.deinit()
    os.exitpoint(os.EXITPOINT_ENABLE_SLEEP)
    time.sleep_ms(100)

if __name__ == "__main__":
    os.exitpoint(os.EXITPOINT_ENABLE)

    nic, network_ip = connect_network(NETWORK_MODE)
    display_test(network_ip)
