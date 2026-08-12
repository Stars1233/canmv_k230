from libs.Network import connect_network

WLAN_DEVICE = "auto"  # "auto", "usb", "sdio", or "spi"

def ap_test():
    ap, _ = connect_network("wifi_ap", ssid='k230_ap_wjx',
                            password='12345678', wlan_device=WLAN_DEVICE)
    #查看ap信息
    print(ap.info())
    #查看ap的状态
    print(ap.status())

ap_test()
