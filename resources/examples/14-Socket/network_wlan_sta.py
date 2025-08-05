import network
import time

SSID = "TEST"
PASSWORD = "12345678"

sta = network.WLAN(network.STA_IF)

sta.connect(SSID, PASSWORD)

timeout = 10  # seconds
start_time = time.time()

while not sta.isconnected():
    if time.time() - start_time > timeout:
        print("Connection timed out")
        break
    os.sleep(1)  # wait for a second before checking again

print(sta.ifconfig())

print(sta.status())

# Disconnect from the network, not necessary, just a test.
sta.disconnect()
print("Disconnected from the network")
print(sta.status())
