import math
import config


class _Track:
    __slots__ = ("x", "y", "wx", "wy", "speed", "raw",
                 "slow_ms", "calm_ms", "state", "steady", "ttl")

    def __init__(self, x, y, wx, wy):
        self.x = x
        self.y = y
        self.wx = wx            # wrist position: motion reference
        self.wy = wy
        self.speed = 0.0        # smoothed, used for PASS/SEEN/LOCK
        self.raw = 0.0          # instantaneous, used for the steady gate
        self.slow_ms = 0
        self.calm_ms = 0
        self.state = "SEEN"
        self.steady = False     # a new track must earn steadiness first
        self.ttl = config.TRACK_TTL


class IntentTracker:
    # Hands are tracked frame-to-frame. A hand only gains control after it has
    # stayed slow for INTENT_TIME_MS ("LOCK"). Fast-moving hands are
    # classified as passing traffic and ignored completely.
    #
    # hand carries a "steady" flag: brightness is only
    # applied while the wrist is holding still. A hand on its way out of the
    # frame is moving, so it can never drag the level with it.
    def __init__(self, display_size):
        self.W = float(display_size[0])
        self.tracks = []

    def _step(self, t, dt_ms):
        if t.state == "LOCK":
            return

        # PASS uses the smoothed speed so it doesn't flicker on one bad frame.
        if t.speed > config.INTENT_FAST:
            t.slow_ms = 0
            t.state = "PASS"

        # LOCK uses the raw speed: the moment the hand stops, the timer starts.
        # (On the EMA it takes ~4 frames to decay, which delayed every gesture.)
        elif t.raw < config.INTENT_SLOW:
            t.slow_ms += dt_ms
            # if the hand is stable enough, mark it as "LOCK"
            if t.slow_ms >= config.INTENT_TIME_MS:
                t.state = "LOCK"
            # if the hand is moving at a medium speed, reset the slow timer and mark it as "seen"
            else:
                t.state = "SEEN"

    def update(self, hands, dt_ms):
        if dt_ms <= 0:
            dt_ms = 66
        free = list(self.tracks)
        max_jump = config.TRACK_MATCH_DIST * self.W

        for h in hands:
            wx, wy = h.wrist

            best, best_d = None, max_jump
            for t in free:
                d = abs(t.x - h.cx) + abs(t.y - h.cy) * 0.5
                if d < best_d:
                    best_d, best = d, t

            if best is None:
                t = _Track(h.cx, h.cy, wx, wy)
                self.tracks.append(t)
            else:
                t = best
                free.remove(t)
                dx = (wx - t.wx) / self.W
                dy = (wy - t.wy) / self.W
                t.raw = math.sqrt(dx * dx + dy * dy) * (1000.0 / dt_ms)
                t.speed = t.speed * 0.6 + t.raw * 0.4
                t.x, t.y = h.cx, h.cy
                t.wx, t.wy = wx, wy
                t.ttl = config.TRACK_TTL

            # Steadiness runs off the raw speed so a sudden move suspends
            # control on the same frame instead of waiting for the average.
            if t.raw > config.MOVE_STEADY:
                t.calm_ms = 0
            else:
                t.calm_ms += dt_ms
            t.steady = t.calm_ms >= config.STEADY_MS

            self._step(t, dt_ms)
            h.intent = t.state if config.INTENT_ENABLED else "LOCK"
            h.speed = t.speed
            h.steady = t.steady if config.INTENT_ENABLED else True

        for t in free:
            t.ttl -= 1
        self.tracks = [t for t in self.tracks if t.ttl > 0]


def controlling(hands):
    if not config.INTENT_ENABLED:
        return hands
    return [h for h in hands if h.intent == "LOCK"]