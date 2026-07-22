from machine import Timer
import time

count = 0


def one_shot_callback(timer):
    print(1)


def periodic_callback(timer):
    global count
    count += 1
    print(2)


# 实例化一个软定时器
tim = Timer(-1)
# 初始化定时器为单次模式，周期100ms
tim.init(period=100, mode=Timer.ONE_SHOT, callback=one_shot_callback, hard=False)
time.sleep_ms(200)
# 初始化定时器为周期模式，频率为1Hz
tim.init(freq=1, mode=Timer.PERIODIC, callback=periodic_callback, hard=False)
while count < 2:
    # 让解释器处理已调度的回调，避免忙等。
    time.sleep_ms(10)
# 释放定时器资源
tim.deinit()
