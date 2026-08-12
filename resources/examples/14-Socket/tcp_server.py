#配置 tcp/udp socket调试工具
import socket
import time,os
from libs.Network import connect_network


CONTENT = b"""
Hello #%d from k230 canmv MicroPython!
"""

NETWORK_TIMEOUT = 20
# Select "default", "lan", "wifi_sta", or "wifi_ap".
NETWORK_TYPE = "wifi_sta"
WLAN_DEVICE = "auto"  # "auto", "usb", "sdio", or "spi"
WIFI_SSID = "TEST"
WIFI_PASSWORD = "12345678"


def server():
    netif, ip = connect_network(
        NETWORK_TYPE,
        ssid=WIFI_SSID,
        password=WIFI_PASSWORD,
        wlan_device=WLAN_DEVICE,
        timeout=NETWORK_TIMEOUT,
    )

    counter=1

    #建立socket
    s = socket.socket(socket.AF_INET, socket.SOCK_STREAM, 0)
    #设置属性
    s.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
    # Bind all interfaces so the server survives a network-device failover.
    ai = socket.getaddrinfo("0.0.0.0", 8080)
    print("Address infos:", ai,8080)
    addr = ai[0][-1]
    print("Connect address:", addr)
    #绑定
    s.bind(addr)
    #设置为非阻塞
    s.settimeout(0)
    #监听
    s.listen(5)
    print("tcp server %s port:%d\n" % ((ip),8080))


    while True:
        #接受连接
        try:
            res = s.accept()
        except Exception as e:
            if e.errno == 11:
                continue    
            else:
                raise
        client_sock = res[0]
        client_addr = res[1]
        print("Client address:", client_addr)
        print("Client socket:", client_sock)
        client_sock.setblocking(False)

        client_stream = client_sock
        #发送字符传
        client_stream.write(CONTENT % counter)

        while True:
            #读取内容
            h = client_stream.read()
            if h == None :
                continue
            if h != b"" :
                print(h)
                #回复内容
                client_stream.write("recv :%s" % h)
            if "end" in h :
                #关闭socket
                client_stream.close()
                break
            os.exitpoint()
            
        counter += 1
        if counter > 10 :
            print("server exit!")
            #关闭
            s.close()
            break
        os.exitpoint()
#main()
server()
