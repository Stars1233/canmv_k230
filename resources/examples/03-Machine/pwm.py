import time
from machine import PWM
from machine import FPIOA

# 实例化FPIOA
fpioa = FPIOA()

# 设置PIN42为PWM通道0
fpioa.set_function(42, fpioa.PWM0)

# 实例化PWM通道0，频率为1000Hz，占空比为50%，默认使能输出
pwm0 = PWM(0)

# 调整通道0频率为2000Hz
pwm0.freq(2000)

# 调整通道0的占空比为 50% (32768 / 65535)
pwm0.duty_u16(32768)
print(pwm0.duty_u16())

# 输出1s之后关闭输出
time.sleep(1)
pwm0.deinit()
time.sleep(1)

# 调整通道0频率为10KHz，占空比为 30%
pwm0.freq(10000)
pwm0.duty(30)
print(pwm0.duty())

# 输出1s之后关闭输出
time.sleep(1)
pwm0.deinit()
