import socket
import struct


# This is a test script intended to run on a PC.
# It joins the same multicast group used by the board receiver.


MCAST_GRP = '239.255.0.1'
MCAST_PORT = 5007

# Create UDP socket
sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM, socket.IPPROTO_UDP)
sock.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)

# Bind to the port
sock.bind(('', MCAST_PORT))  # '' means all interfaces

# Join multicast group
mreq = struct.pack("=4sl", socket.inet_aton(MCAST_GRP), socket.INADDR_ANY)
sock.setsockopt(socket.IPPROTO_IP, socket.IP_ADD_MEMBERSHIP, mreq)

print(f"Listening on multicast group {MCAST_GRP}:{MCAST_PORT}...")

while True:
    data, addr = sock.recvfrom(1024)
    print(f"Received from {addr}: {data.decode()}")
