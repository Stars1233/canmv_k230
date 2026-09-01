import config
import gesture
import intent

class Zone:
    __slots__ = ("id", "name", "leds", "led_ids", "x0", "x1", "engage",
                 "smooth", "hands", "warn", "warn_kind", "warn_ttl")

    def __init__(self, zid, name, leds, led_ids, x0, x1):
        self.id = zid
        self.name = name
        self.leds = leds
        self.led_ids = led_ids          # raw indices, used to detect sharing
        self.x0 = x0
        self.x1 = x1
        self.engage = 0
        self.smooth = 0.0
        self.hands = []
        self.warn = False
        self.warn_kind = None           # "MULTI" | "SHARED" | None
        self.warn_ttl = 0

    @property
    def active(self):
        # checks whether zone can control LED
        return (not self.warn
                and len(self.hands) == 1
                and self.engage >= config.ENGAGE_FRAMES)

    @property
    def level(self):
        # gets current LED brightness
        return max(l.level for l in self.leds)

    @property
    def status(self):
        # status display
        if len(self.leds) == 1:
            return "%d%%" % self.leds[0].level
        return "/".join("%d" % l.level for l in self.leds)

    def set_level(self, pct):
        # set all LEDs in zone
        for l in self.leds:
            l.set(pct)


class ZoneController:
    def __init__(self, leds, display_size):
        n = len(config.ZONES)
        self.width = display_size[0] / float(n)
        self.zones = [Zone(i, name, [leds[k] for k in led_ids], tuple(led_ids),
                           int(i * self.width), int((i + 1) * self.width))
                      for i, (name, led_ids) in enumerate(config.ZONES)]

    # finding zone from hand position
    def zone_of(self, x):
        i = int(x / self.width)
        if i < 0:
            i = 0
        elif i >= len(self.zones):
            i = len(self.zones) - 1
        return self.zones[i]

    def update(self, hands, dt_ms):
        # filter hands
        hands = intent.controlling(hands)

        # assign hands to zones
        for z in self.zones:
            z.hands = []
        for h in hands:
            self.zone_of(h.cx).hands.append(h)

        # detect LED conflicts
        claims = {}
        # count how many zones are requesting each LED
        for z in self.zones:
            if z.hands:
                for k in z.led_ids:
                    claims[k] = claims.get(k, 0) + 1

        # resolve conflicts
        for z in self.zones:
            n = len(z.hands)

            kind = None
            # two hands in one zone
            if n >= 2:
                kind = "MULTI"

            # shared LED
            elif n == 1:
                for k in z.led_ids:
                    if claims[k] > 1:
                        kind = "SHARED"
                        break

            # warning action
            if kind:
                z.warn = True
                z.warn_kind = kind
                z.warn_ttl = config.WARN_HOLD_FRAMES
                z.engage = 0
                continue

            if z.warn_ttl > 0:                      # cooling down from a warning
                z.warn_ttl -= 1
                z.engage = 0
                continue

            z.warn = False
            z.warn_kind = None

            if n != 1:
                z.engage = 0
                continue

            h = z.hands[0]
            if gesture.thumbs_up(h.points):
                z.engage = 0                        # hold during the menu gesture
                continue
            if not h.steady:
                # hand is moving: return without changing brightness
                continue
            if z.engage == 0:
                z.smooth = float(z.level)
            z.engage += 1
            if z.engage >= config.ENGAGE_FRAMES:
                target = h.openness * 100.0
                step = (target - z.smooth) * config.SMOOTH
                if step > config.MAX_STEP:
                    step = config.MAX_STEP
                elif step < -config.MAX_STEP:
                    step = -config.MAX_STEP
                z.smooth += step
                z.set_level(int(round(z.smooth)))