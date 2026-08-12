#配置 tcp/udp socket调试工具
import socket
import time,os
from libs.Network import connect_network

NETWORK_TIMEOUT = 20
# Select "default", "lan", "wifi_sta", or "wifi_ap".
NETWORK_TYPE = "wifi_sta"
WLAN_DEVICE = "auto"  # "auto", "usb", "sdio", or "spi"
WIFI_SSID = "TEST"
WIFI_PASSWORD = "12345678"

def udpserver():
    netif, ip = connect_network(
        NETWORK_TYPE,
        ssid=WIFI_SSID,
        password=WIFI_PASSWORD,
        wlan_device=WLAN_DEVICE,
        timeout=NETWORK_TIMEOUT,
    )
        
    # Bind all interfaces so the server survives a network-device failover.
    ai = socket.getaddrinfo("0.0.0.0", 8080)
    #ai = socket.getaddrinfo("10.10.1.94", 60000)
    print("Address infos:", ai)
    addr = ai[0][-1]

    print("udp server %s port:%d\n" % ((ip),8080))
    #建立socket
    s = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
    #设置属性
    s.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
    #绑定
    s.bind(addr)
    #设置为非阻塞
    s.settimeout(0)
    #延时
    time.sleep(1)
    
    counter=0
    while True :
        os.exitpoint()
        data, addr = s.recvfrom(800)
        if data == b"":
            continue
        print("recv %d" % counter,data,addr)
        #回复内容
        s.sendto(b"%s have recv count=%d " % (data,counter), addr)
        counter = counter+1
        if counter > 10 :
            break
    #关闭
    s.close()
    print("udp server exit!!")




#main()
udpserver()
