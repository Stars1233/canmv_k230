from libs.Network import NetworkManager, get_default_device


# "default" reuses an interface that already has an IP address.
# "lan" uses DHCP, "wifi_sta" joins an AP, and "wifi_ap" creates an AP.
NETWORK_TYPE = "wifi_sta"  # "default", "lan", "wifi_sta", or "wifi_ap"

# "auto" lets netmgmt select an available Wi-Fi card. Use "usb", "sdio", or
# "spi" only when this application must use that specific Wi-Fi transport.
WLAN_DEVICE = "auto"
WIFI_SSID = "TEST"
WIFI_PASSWORD = "12345678"
NETWORK_TIMEOUT = 20


def main():
    manager = NetworkManager(
        network_type=NETWORK_TYPE,
        ssid=WIFI_SSID,
        password=WIFI_PASSWORD,
        wlan_device=WLAN_DEVICE,
        timeout=NETWORK_TIMEOUT,
        show=False,
    )

    # Inspect registered devices before choosing one. Netdev names are assigned
    # dynamically, so applications should not hard-code them.
    manager.show_devices()

    netif, ip = manager.connect()
    manager.show_info()
    print("Network ready:", ip)
    print("Default network device:", get_default_device() or "auto")

    # The manager keeps the interface for reuse by all application components.
    # manager.connect() reconnects when needed, manager.scan() scans Wi-Fi, and
    # manager.disconnect() stops Wi-Fi and restores automatic route selection.
    return netif


main()
