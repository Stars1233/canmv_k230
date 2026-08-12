#配置 tcp/udp socket调试工具
import socket
import os,time
from libs.Network import connect_network

NETWORK_TIMEOUT = 20
# Select "default", "lan", "wifi_sta", or "wifi_ap".
NETWORK_TYPE = "wifi_sta"
WLAN_DEVICE = "auto"  # "auto", "usb", "sdio", or "spi"
WIFI_SSID = "TEST"
WIFI_PASSWORD = "12345678"

def client():
    netif, _ = connect_network(
        NETWORK_TYPE,
        ssid=WIFI_SSID,
        password=WIFI_PASSWORD,
        wlan_device=WLAN_DEVICE,
        timeout=NETWORK_TIMEOUT,
    )
    
    #建立socket
    s = socket.socket(socket.AF_INET, socket.SOCK_STREAM, 0)
    #获取地址及端口号 对应地址
    ai = socket.getaddrinfo("192.168.1.110", 8080)
    #ai = socket.getaddrinfo("10.10.1.94", PORT)
    print("Address infos:", ai)
    addr = ai[0][-1]

    print("Connect address:", addr)
    #连接地址
    if(s.connect(addr) == False):
        s.close()
        print("conner err")
        return

    for i in range(10):
        str="K230 tcp client send test {0} \r\n".format(i)
        print(str)
        #print(s.send(str))
        #发送字符串
        print(s.write(str))
        time.sleep(0.2)
        #time.sleep(1)
        #print(s.recv(4096))
        #print(s.read())
    #延时1秒
    time.sleep(1)
    #关闭socket
    s.close()
    print("end")



#main()
client()
