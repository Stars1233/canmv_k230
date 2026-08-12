import socket
import struct
import time
from libs.Network import connect_network

MULTICAST_GROUP = '239.255.0.1'
MULTICAST_PORT = 5007

# Select "default", "lan", "wifi_sta", or "wifi_ap".
NETWORK_TYPE = "wifi_sta"
WLAN_DEVICE = "auto"  # "auto", "usb", "sdio", or "spi"
# Toggle for sender or receiver
IS_SENDER = False
NETWORK_TIMEOUT = 20
WIFI_SSID = "TEST"
WIFI_PASSWORD = "12345678"

def inet_aton(ip_str):
    """Convert dotted string to 4-byte IP (like socket.inet_aton)."""
    return bytes(map(int, ip_str.split('.')))

def multicast_sender(ip):
    """Send UDP multicast messages every 2 seconds."""
    sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
    ttl = struct.pack('b', 1)
    sock.setsockopt(0, 33, ttl)  # IPPROTO_IP = 0, IP_MULTICAST_TTL = 33

    count = 0
    print(f"[SENDER] Sending to {MULTICAST_GROUP}:{MULTICAST_PORT}")
    while True:
        msg = f"[{ip}] Multicast message {count}"
        try:
            sock.sendto(msg.encode(), (MULTICAST_GROUP, MULTICAST_PORT))
            print(f"[SENDER] Sent: {msg}")
        except Exception as e:
            print("[SENDER] Error:", e)
        count += 1
        time.sleep(2)

def multicast_receiver():
    """Listen to UDP multicast messages."""
    sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
    sock.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
    sock.bind(('0.0.0.0', MULTICAST_PORT))

    mreq = struct.pack("4s4s",
                       inet_aton(MULTICAST_GROUP),
                       inet_aton('0.0.0.0'))  # Use 'network_ip' for stricter binding

    sock.setsockopt(0, 35, mreq)  # IPPROTO_IP = 0, IP_ADD_MEMBERSHIP = 35

    print(f"[RECEIVER] Listening on {MULTICAST_GROUP}:{MULTICAST_PORT} ...")

    while True:
        try:
            data, addr = sock.recvfrom(1024)
            if data:
                try:
                    print(f"[RECEIVER] From {addr}: {data.decode()}")
                except UnicodeError:
                    print(f"[RECEIVER] From {addr}: <binary data>")
            else:
                print(f"[RECEIVER] From {addr}: <empty packet>")
        except Exception as e:
            print("[RECEIVER] Error:", e)
            break

def main():
    netif, ip = connect_network(
        NETWORK_TYPE,
        ssid=WIFI_SSID,
        password=WIFI_PASSWORD,
        wlan_device=WLAN_DEVICE,
        timeout=NETWORK_TIMEOUT,
    )
    if IS_SENDER:
        multicast_sender(ip)
    else:
        multicast_receiver()

main()
