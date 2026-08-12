from libs.Network import connect_network

SSID = "TEST"
PASSWORD = "12345678"
WLAN_DEVICE = "auto"  # "auto", "usb", "sdio", or "spi"

sta, _ = connect_network("wifi_sta", ssid=SSID, password=PASSWORD,
                         wlan_device=WLAN_DEVICE, timeout=10)
print(sta.ifconfig())

print(sta.status())

# 这里的断开网络，只是一个测试。实际应用可不断开
sta.disconnect()
print("断开连接")
print(sta.status())
