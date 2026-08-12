import usocket
import ussl
from libs.Network import connect_network

NETWORK_TIMEOUT = 20
NETWORK_TYPE = "wifi_sta"  # "default", "lan", "wifi_sta", or "wifi_ap"
WLAN_DEVICE = "auto"  # "auto", "usb", "sdio", or "spi"
WIFI_SSID = "TEST"
WIFI_PASSWORD = "12345678"

# HTTPS GET to www.baidu.com
def test_https_baidu():
    try:
        print("\n[TEST] HTTPS GET https://www.baidu.com")
        addr = usocket.getaddrinfo("www.baidu.com", 443)[0][-1]
        sock = usocket.socket()
        sock.connect(addr)

        # Wrap with TLS — SNI is required by Baidu
        ssl_sock = ussl.wrap_socket(sock, server_hostname="www.baidu.com")

        # Send HTTPS GET request
        ssl_sock.write(b"GET / HTTP/1.1\r\nHost: www.baidu.com\r\nConnection: close\r\n\r\n")

        # Read and print the response
        while True:
            data = ssl_sock.read()
            if not data:
                break
            print(data.decode(), end='')

        ssl_sock.close()
        print("\n[✓] HTTPS request to Baidu completed")
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
