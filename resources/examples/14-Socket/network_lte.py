import time
from usb import Serial

ser = Serial("/dev/ttyUSB1", 200)

while True:
    if ser.open():
        break
    time.sleep(1)

# 获取模组信息
ser.write("AT+GMR\r\n")
time.sleep(0.1)
data = ser.read()
print(data)

# 拨号
ser.write("AT+QNETDEVCTL=1,1,1\r\n")
time.sleep(0.1)
data = ser.read()
print(data)

ser.close()
