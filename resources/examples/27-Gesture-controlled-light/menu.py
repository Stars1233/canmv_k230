import math
import config
import gesture
import intent


class MenuController:
    def __init__(self, leds, display_size):
        W, H = display_size

        # center of the radial menu
        self.cx = W // 2
        self.cy = config.HEADER_H + (H - config.HEADER_H - config.PERF_H) // 2

        self.items = []             # (label, kind, LED group)
        for label, kind, ids in config.MENU_ITEMS:
            grp = [leds[k] for k in ids] if kind == "device" else None
            self.items.append((label, kind, grp))

        self.n = len(self.items)    # number of menu items
        self.span = 360.0 / self.n
        self.reset()

    def reset(self):
        # reset menu selection state.
        self.hover = -1
        self.thumb_ms = 0
        self.release_ms = 0
        self.pointer = None
        self.warn = False
        self.seen_release = False   # must drop the pose once before confirming

    def group_level(self, i):
        # return brightness of a device group
        grp = self.items[i][2]
        return max(l.level for l in grp) if grp else -1

    def _segment_of(self, dx, dy):
        # convert pointer angle into menu segment index
        a = math.degrees(math.atan2(dy, dx))    # -180..180, 0 = right
        return int(((a + 90.0) % 360.0) // self.span)

    def update(self, hands, dt_ms):
        # Returns the confirmed item index, or None.
        hands = intent.controlling(hands)
        self.warn = len(hands) >= 2
        if self.warn:
            self.hover = -1
            self.thumb_ms = 0
            return None     # more than 1 hand: return None
        if not hands:
            self.pointer = None
            self.hover = -1
            self.thumb_ms = 0
            self.seen_release = True
            return None     # no hand: return None

        # use wrist as menu pointer
        h = hands[0]
        wx, wy = h.wrist
        self.pointer = (wx, wy)

        # calculate pointer position relative to menu center
        dx, dy = wx - self.cx, wy - self.cy
        dist = math.sqrt(dx * dx + dy * dy)

        if config.WHEEL_R_IN * 0.5 <= dist <= config.WHEEL_R_OUT + config.WHEEL_GRAB:
            self.hover = self._segment_of(dx, dy)
        else:
            self.hover = -1

        # wait for thumbs-up gesture
        if not gesture.thumbs_up(h.points):
            self.release_ms += dt_ms

            # ensure thumb has been released before allowing selection
            if self.release_ms >= config.THUMB_RELEASE_MS:
                self.seen_release = True
            self.thumb_ms = 0
            return None

        # thumbs-up held
        self.release_ms = 0

        # ignore if menu was opened while already holding thumbs-up
        if not self.seen_release:
            return None

        # confirm selection after holding thumbs-up long enough
        if self.hover >= 0:
            self.thumb_ms += dt_ms
            if self.thumb_ms >= config.THUMB_CONFIRM_MS:
                return self.hover
        return None
