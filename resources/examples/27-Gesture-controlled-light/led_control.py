from machine import PWM, FPIOA
import config


def _pwm_func(channel):
    name = "PWM%d" % channel
    try:
        return getattr(FPIOA, name)
    except AttributeError:
        raise ValueError("FPIOA has no %s on this firmware" % name)


class LED:
    def __init__(self, pwm, name):
        self.pwm  = pwm
        self.name = name
        self.level = 0             # current brightness 0..100

    # changes brightness
    def set(self, pct):
        pct = int(max(0, min(100, pct)))
        if pct != self.level:
            self.level = pct
            self.pwm.duty(pct)
            print(self.name, "->", pct, "%")


def create_leds():
    # create LED objects to control the PWM channels
    fpioa = FPIOA()
    leds = []
    for (name, pin, channel) in config.LED_PINS:
        fpioa.set_function(pin, _pwm_func(channel))
        leds.append(LED(PWM(channel, freq=config.PWM_FREQ, duty=0), name))
    return leds


def shutdown(leds):
    # release PWM hardware and turn off all LEDs.
    for l in leds:
        try:
            l.pwm.duty(0)
            l.pwm.deinit()
        except Exception:
            pass
