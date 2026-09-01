import config
import gesture
import intent


class DeviceController:
    def __init__(self, leds, display_size):
        self.name = ""
        self.group = []
        self.smooth = 0.0
        self.warn = False

    def select(self, name, group):
        self.name = name
        self.group = group
        self.smooth = float(self.level)

    @property
    def level(self):
        return max(l.level for l in self.group) if self.group else 0

    def update(self, hands, dt_ms):
        hands = intent.controlling(hands)
        n = len(hands)
        if n == 1:
            self.warn = False
            h = hands[0]
            if gesture.thumbs_up(h.points):
                return
            if not h.steady:
                # hand is moving: return without changing brightness
                return
            target = h.openness * 100.0

            # Smooth filter, with a cap on how far one frame can move the LED
            step = (target - self.smooth) * config.SMOOTH
            if step > config.MAX_STEP:
                step = config.MAX_STEP
            elif step < -config.MAX_STEP:
                step = -config.MAX_STEP
            self.smooth += step
            pct = int(round(self.smooth))
            for l in self.group:
                l.set(pct)
        elif n == 0:
            self.warn = False                # hold

        # If two hands (ambiguous), no LED changes.
        else:
            self.warn = True