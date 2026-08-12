import socket
import time


# This is a test script intended to run on a PC.
# It sends to the same multicast group used by the board receiver.


MCAST_GRP = '239.255.0.1'
MCAST_PORT = 5007

sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM, socket.IPPROTO_UDP)

# Set TTL to 1 to keep multicast packets on the local network
sock.setsockopt(socket.IPPROTO_IP, socket.IP_MULTICAST_TTL, 1)

message = "Hello from PC multicast sender!"

while True:
    sock.sendto(message.encode(), (MCAST_GRP, MCAST_PORT))
    print(f"Sent: {message}")
    time.sleep(1)
