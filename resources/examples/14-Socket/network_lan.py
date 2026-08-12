from libs.Network import configure_ip, connect_network, network_device_name


def main():
    a, ip = connect_network("lan")
    print(a.active())
    #查看网口 ip，掩码，网关，dns配置
    print(a.ifconfig())
    #设置网口 ip，掩码，网关，dns配置
    configure_ip(a, ('192.168.0.4', '255.255.255.0',
                     '192.168.0.1', '8.8.8.8'))
    print(a.ifconfig())
    #查看网口 ip，掩码，网关，dns配置
    print(a.ifconfig())
    #设置网口为dhcp模式，并复用公共等待逻辑
    ip = configure_ip(a, "dhcp")
    print(a.ifconfig())
    print("LAN device:", network_device_name(a))
    print("LAN address:", ip)
    #查看网口 ip，掩码，网关，dns配置
    print(a.ifconfig())
    #查看网口mac地址
    print(a.config("mac"))
    configure_ip(a, "dhcp")
    print(a.ifconfig())
    #查看网口 ip，掩码，网关，dns配置
    print(a.ifconfig())




main()
