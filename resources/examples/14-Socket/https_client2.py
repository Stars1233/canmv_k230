import requests
from libs.Network import connect_network

NETWORK_TIMEOUT = 20
NETWORK_TYPE = "wifi_sta"  # "default", "lan", "wifi_sta", or "wifi_ap"
WLAN_DEVICE = "auto"  # "auto", "usb", "sdio", or "spi"
WIFI_SSID = "TEST"
WIFI_PASSWORD = "12345678"

# Use urequests to make HTTPS request
def test_https_baidu():
    print("\n[TEST] HTTPS GET using urequests")
    try:
        response = requests.get("https://www.baidu.com")
        print(response.text)
        response.close()
        print("[✓] HTTPS request completed")
    except Exception as e:
        print("❌ HTTPS request failed:", e)

def main():
    netif, _ = connect_network(
        NETWORK_TYPE,
        ssid=WIFI_SSID,
        password=WIFI_PASSWORD,
        wlan_device=WLAN_DEVICE,
        timeout=NETWORK_TIMEOUT,
    )
    test_https_baidu()


main()
