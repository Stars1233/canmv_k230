import socket
from libs.Network import connect_network

NETWORK_TIMEOUT = 20
# Select "default", "lan", "wifi_sta", or "wifi_ap".
NETWORK_TYPE = "wifi_sta"
WLAN_DEVICE = "auto"  # "auto", "usb", "sdio", or "spi"
WIFI_SSID = "TEST"
WIFI_PASSWORD = "12345678"

def main(use_stream=True):
    netif, _ = connect_network(
        NETWORK_TYPE,
        ssid=WIFI_SSID,
        password=WIFI_PASSWORD,
        wlan_device=WLAN_DEVICE,
        timeout=NETWORK_TIMEOUT,
    )
    #创建socket
    s = socket.socket()
    #获取地址及端口号 对应地址
    ai = []
        
    for attempt in range(0, 3):
        try:
            #获取地址及端口号 对应地址
            ai = socket.getaddrinfo("www.baidu.com", 80)
            break
        except:
            print("getaddrinfo again");
    
    if(ai == []):
        print("connet error")
        s.close()
        return

    print("Address infos:", ai)
    addr = ai[0][-1]
    
    print("Connect address:", addr)
    #连接
    s.connect(addr)

    if use_stream:
        # MicroPython socket objects support stream (aka file) interface
        # directly, but the line below is needed for CPython.
        s = s.makefile("rwb", 0)
        #发送http请求
        s.write(b"GET /index.html HTTP/1.0\r\n\r\n")
        #打印请求内容
        print(s.read())
    else:
        #发送http请求
        s.send(b"GET /index.html HTTP/1.0\r\n\r\n")
        #打印请求内容
        print(s.recv(4096))
        #print(s.read())
    #关闭socket
    s.close()


#main()
main(use_stream=True)
main(use_stream=False)
