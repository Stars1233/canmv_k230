import math
import config
import gesture


class Dashboard:
    def __init__(self, pl, display_size, leds):
        self.pl = pl
        self.ds = display_size
        self.leds = leds
        self.t = 0
        self.header_h = config.HEADER_H                            # Header
        self.body_top = config.HEADER_H + 6                        # Beginning of working area
        self.body_bot = display_size[1] - config.PERF_H - 4        # Bottom of working area
        self._burst = 0
        self._burst_c = None

    # ---------------- helpers ----------------
    def _text(self, x, y, s, color, size):
        osd = self.pl.osd_img
        try:
            osd.draw_string_advanced(int(x), int(y), size, s, color=color)
        except Exception:
            try:
                osd.draw_string(int(x), int(y), s, color=color, scale=2)
            except Exception:
                pass

    def _rect(self, x, y, w, h, color, thickness=1, fill=False):
        try:
            self.pl.osd_img.draw_rectangle(int(x), int(y), int(w), int(h),
                                           color=color, thickness=thickness,
                                           fill=fill)
        except Exception:
            pass

    def _line(self, x0, y0, x1, y1, color, thickness=1):
        self.pl.osd_img.draw_line(int(x0), int(y0), int(x1), int(y1),
                                  color=color, thickness=thickness)

    def _circle(self, x, y, r, color, fill=False):
        try:
            self.pl.osd_img.draw_circle(int(x), int(y), int(r),
                                        color=color, fill=fill)
        except Exception:
            pass

    def _arc(self, cx, cy, r, a0, a1, color, thickness=2, step=10):
        a = a0
        px = cx + r * math.cos(math.radians(a))
        py = cy + r * math.sin(math.radians(a))
        while a < a1:
            a = min(a + step, a1)
            nx = cx + r * math.cos(math.radians(a))
            ny = cy + r * math.sin(math.radians(a))
            self._line(px, py, nx, ny, color, thickness)
            px, py = nx, ny

    def _sector(self, cx, cy, r_in, r_out, a0, a1, color, astep=4):
        a = a0
        while a <= a1:
            ar = math.radians(a)
            ca, sa = math.cos(ar), math.sin(ar)
            self._line(cx + r_in * ca, cy + r_in * sa,
                       cx + r_out * ca, cy + r_out * sa, color, 2)
            a += astep

    def _bulb(self, x, y, color):
        self._circle(x, y, 7, color)
        self._circle(x, y, 3, color, fill=True)
        self._line(x - 3, y + 8, x + 3, y + 8, color, 2)
        self._line(x - 2, y + 11, x + 2, y + 11, color, 2)

    def _grid_icon(self, x, y, color):
        # three vertical bars = the 3-region split
        for dx in (-8, 0, 8):
            self._rect(x + dx - 2, y - 8, 4, 16, color, thickness=2)

    def _pulse(self, lo, hi, period=24):
        p = self.t % period
        f = p / (period / 2.0)
        if f > 1.0:
            f = 2.0 - f
        return lo + (hi - lo) * f

    def _blink(self, period=12):
        return (self.t // period) % 2 == 0

    def notify_select(self, center):
        self._burst = 16
        self._burst_c = center

    def _draw_burst(self):
        if self._burst > 0 and self._burst_c:
            cx, cy = self._burst_c
            k = 16 - self._burst
            for j in range(3):
                self._arc(cx, cy, 14 + k * 9 + j * 12, 0, 360,
                          config.COLOR_ACCENT, 2, step=24)
            self._burst -= 1

    # ---------------- header ----------------
    def _draw_header(self, state, dev_name):
        W, _ = self.ds
        self._rect(0, 0, W, self.header_h, config.COLOR_PANEL, fill=True)
        self._line(0, self.header_h, W, self.header_h, config.COLOR_PANEL_LINE, 1)
        self._text(10, 6, "K230 GESTURE LIGHT", config.COLOR_ACCENT,
                   config.FONT_TITLE)

        if state == config.STATE_MENU:
            label, hint = "MENU", "point + thumbs-up to pick"
        elif state == config.STATE_DEVICE:
            label, hint = dev_name, "thumbs-up = menu"
        else:
            label, hint = "REGION", "thumbs-up = menu"
        self._text(270, 11, label, config.COLOR_WHITE, config.FONT_INFO)

        self._circle(W - 210, 21, int(self._pulse(3, 6)),
                     config.COLOR_LOCKED, fill=True)
        self._text(W - 198, 13, hint, config.COLOR_METRIC_DIM, config.FONT_LABEL)

    # ---------------- telemetry ----------------
    def _metric(self, x, y, w, label, value, frac, hot=False):
        self._text(x, y + 4, label, config.COLOR_METRIC_DIM, config.FONT_LABEL)
        self._text(x, y + 20, value, config.COLOR_METRIC, config.FONT_METRIC)
        bw = w - 16
        frac = 0.0 if frac < 0 else (1.0 if frac > 1 else frac)
        self._rect(x, y + 44, bw, 6, config.COLOR_BAR_BG, fill=True)
        if frac > 0.01:
            col = config.COLOR_BAR_HOT if hot else config.COLOR_BAR
            self._rect(x, y + 44, int(bw * frac), 6, col, fill=True)

    def _draw_perf(self, perf):
        W, H = self.ds
        y = H - config.PERF_H
        self._rect(0, y, W, config.PERF_H, config.COLOR_PANEL, fill=True)
        self._line(0, y, W, y, config.COLOR_PANEL_LINE, 1)
        duties = "/".join("%d" % l.level for l in self.leds)
        peak = max(l.level for l in self.leds)
        cols = (
            ("FPS",     "%.1f" % perf.fps,           perf.fps / 30.0,      False),
            ("CPU",     "%d%%" % int(perf.cpu),      perf.cpu / 100.0,     perf.cpu > 85),
            ("INFER",   "%dms" % int(perf.infer_ms), perf.infer_ms / 60.0, perf.infer_ms > 50),
            ("MEM",     "%d%% (%dK)" % (int(perf.mem_pct), perf.mem_free_kb),
                        perf.mem_pct / 100.0,         perf.mem_pct > 85),
            ("PWM",     "%s%%" % duties,             peak / 100.0,         False),
            ("LATENCY", "%dms" % int(perf.lat_ms),   perf.lat_ms / 120.0,  perf.lat_ms > 100),
        )
        cw = W // len(cols)
        for i, (label, value, frac, hot) in enumerate(cols):
            self._metric(8 + i * cw, y + 2, cw, label, value, frac, hot)

    # ---------------- safety ----------------
    def _draw_safety(self, x, y, w, h, line1="MULTI-HAND",
                     line2="SAFETY LOCK", hint="one hand only"):
        th = 3 if self._blink(6) else 5
        self._rect(x, y, w, h, config.COLOR_WARN, thickness=th)
        self._rect(x + 6, y + 6, w - 12, h - 12, config.COLOR_WARN, thickness=1)
        if self._blink(8):
            self._text(x + 14, y + h // 2 - 34, line1,
                       config.COLOR_WARN, config.FONT_WARN)
            self._text(x + 14, y + h // 2 - 2, line2,
                       config.COLOR_WARN, config.FONT_WARN)
        self._text(x + 14, y + h // 2 + 32, hint,
                   config.COLOR_METRIC_DIM, config.FONT_LABEL)

    # ---------------- radial menu ----------------
    def _draw_menu(self, menu):
        cx, cy = menu.cx, menu.cy
        r_in, r_out = config.WHEEL_R_IN, config.WHEEL_R_OUT
        span = menu.span

        for k in range(3):
            self._arc(cx, cy, r_out + 6 + k * 4, 0, 360,
                      config.COLOR_WHEEL_GLOW, 1, step=20)
        base = (self.t * 4) % 360
        for k in range(12):
            a = base + k * 30
            self._arc(cx, cy, r_out + 14, a, a + 10,
                      config.COLOR_WHEEL_GLOW, 2, step=10)

        for i, (label, kind, grp) in enumerate(menu.items):
            a0 = i * span - 90 + 3
            a1 = (i + 1) * span - 90 - 3
            am = math.radians((a0 + a1) / 2.0)
            hov = (i == menu.hover)

            if hov:
                fill, edge, astep = config.COLOR_WHEEL_HFILL, config.COLOR_WHEEL_HOVER, 3
            else:
                fill, edge, astep = config.COLOR_WHEEL_FILL, config.COLOR_WHEEL_RING, 6

            self._sector(cx, cy, r_in, r_out, a0, a1, fill, astep)
            self._arc(cx, cy, r_in, a0, a1, edge, 2)
            self._arc(cx, cy, r_out, a0, a1, edge, 3)
            for a in (a0, a1):
                ar = math.radians(a)
                self._line(cx + r_in * math.cos(ar), cy + r_in * math.sin(ar),
                           cx + r_out * math.cos(ar), cy + r_out * math.sin(ar),
                           edge, 2)

            if hov and menu.thumb_ms > 0:
                frac = min(1.0, menu.thumb_ms / float(config.THUMB_CONFIRM_MS))
                self._arc(cx, cy, r_out + 4, a0, a0 + (a1 - a0) * frac,
                          config.COLOR_ACCENT, 5)

            # icon (bulb for a device, grid for region)
            mid_r = (r_in + r_out) / 2.0
            ix = cx + mid_r * math.cos(am)
            iy = cy + mid_r * math.sin(am)
            ic = config.COLOR_WHEEL_HOVER if hov else config.COLOR_METRIC_DIM
            if kind == "device":
                self._bulb(ix, iy - 6, ic)
            else:
                self._grid_icon(ix, iy, ic)

            # WHITE label + level outside the ring
            lx = cx + (r_out + 34) * math.cos(am) - 26
            ly = cy + (r_out + 34) * math.sin(am) - 10
            self._text(lx, ly, label, config.COLOR_WHITE, config.FONT_INFO)
            lvl = menu.group_level(i)
            sub = "%d%%" % lvl if lvl >= 0 else "3 zones"
            self._text(lx + 2, ly + 22, sub, config.COLOR_WHITE, config.FONT_LABEL)

        # hub
        self._circle(cx, cy, r_in - 6, config.COLOR_HUB, fill=True)
        self._arc(cx, cy, r_in - 6, 0, 360, config.COLOR_ACCENT_DIM, 2, step=16)
        if menu.warn:
            if self._blink(8):
                self._text(cx - 58, cy - 26, "SAFETY", config.COLOR_WARN, config.FONT_WARN)
                self._text(cx - 42, cy + 6, "LOCK", config.COLOR_WARN, config.FONT_WARN)
        else:
            self._text(cx - 34, cy - 26, "MENU", config.COLOR_WHITE, config.FONT_ZONE)
            if menu.hover >= 0:
                self._text(cx - 58, cy + 6, "thumbs-up", config.COLOR_WHITE, config.FONT_INFO)
                self._text(cx - 46, cy + 28, "to pick", config.COLOR_METRIC_DIM, config.FONT_LABEL)
            else:
                self._text(cx - 42, cy + 6, "point at", config.COLOR_METRIC_DIM, config.FONT_INFO)
                self._text(cx - 44, cy + 28, "an option", config.COLOR_METRIC_DIM, config.FONT_LABEL)

        if menu.pointer and not menu.warn:
            px, py = menu.pointer
            self._line(cx, cy, px, py, config.COLOR_ACCENT, 2)
            self._circle(px, py, int(self._pulse(5, 9)), config.COLOR_ACCENT)

    # ---------------- device (whole-frame) ----------------
    def _draw_device(self, dev):
        W = self.ds[0]
        cx = W // 2
        cy = (self.body_top + self.body_bot) // 2

        if dev.warn:
            self._draw_safety(cx - 170, cy - 120, 340, 240)
            return

        lvl = dev.level
        r = 118
        self._arc(cx, cy, r, 0, 360, config.COLOR_WHEEL_RING, 3, step=12)
        # brightness ring, clockwise from top
        self._arc(cx, cy, r, -90, -90 + 360 * lvl / 100.0, config.COLOR_BAR, 7, step=8)
        # animated ticks
        for k in range(0, 360, 30):
            ar = math.radians(k - 90)
            self._line(cx + (r + 6) * math.cos(ar), cy + (r + 6) * math.sin(ar),
                       cx + (r + 12) * math.cos(ar), cy + (r + 12) * math.sin(ar),
                       config.COLOR_ACCENT_DIM, 1)

        self._bulb(cx, cy - 58, config.COLOR_WHITE)
        self._text(cx - len(dev.name) * 7, cy - 34, dev.name,
                   config.COLOR_WHITE, config.FONT_ZONE)
        self._text(cx - 44, cy + 2, "%d%%" % lvl, config.COLOR_WHITE, config.FONT_BIG)
        self._text(cx - 96, cy + 74, "whole frame controls this",
                   config.COLOR_METRIC_DIM, config.FONT_LABEL)

    # ---------------- zones (region mode) ----------------
    def _draw_zones(self, zones):
        top, bot = self.body_top, self.body_bot
        for z in zones:
            if z.id > 0:
                self._line(z.x0, top, z.x0, bot, config.COLOR_DIVIDER, 2)
            col = (config.COLOR_WARN if z.warn else
                   config.COLOR_ACTIVE if z.active else config.COLOR_IDLE)
            self._text(z.x0 + 10, top + 4, z.name, col, config.FONT_ZONE)
            self._text(z.x0 + 10, top + 34, z.status, col, config.FONT_INFO)

            w = z.x1 - z.x0
            if z.warn:
                if z.warn_kind == "SHARED":
                    self._draw_safety(z.x0 + 4, top + 64, w - 8, bot - top - 68,
                                      "SHARED LED", "SAFETY LOCK",
                                      "one zone at a time")
                else:
                    self._draw_safety(z.x0 + 4, top + 64, w - 8, bot - top - 68)
                continue
            if z.active:
                g = int(self._pulse(14, 26))
                x0, y0, x1, y1 = z.x0 + 6, top + 64, z.x1 - 6, bot - 4
                for (bx, by, sx, sy) in ((x0, y0, 1, 1), (x1, y0, -1, 1),
                                         (x0, y1, 1, -1), (x1, y1, -1, -1)):
                    self._line(bx, by, bx + sx * g, by, col, 3)
                    self._line(bx, by, bx, by + sy * g, col, 3)
                mh = bot - top - 90
                mx = z.x1 - 22
                self._rect(mx, top + 80, 10, mh, config.COLOR_BAR_BG, fill=True)
                fh = int(mh * z.level / 100.0)
                if fh > 0:
                    self._rect(mx, top + 80 + mh - fh, 10, fh, config.COLOR_BAR, fill=True)

    # ---------------- hands ----------------
    def _hand_color(self, h):
        if h.intent == "PASS":
            return config.COLOR_PASSING
        if h.intent == "SEEN":
            return config.COLOR_TRACKING
        return None

    def _draw_hands(self, hands):
        osd = self.pl.osd_img
        for h in hands:
            mono = self._hand_color(h)
            for f, chain in enumerate(gesture.FINGER_CHAINS):
                col = mono if mono else config.FINGER_COLORS[f]
                for k in range(len(chain) - 1):
                    a = h.points[chain[k]]
                    b = h.points[chain[k + 1]]
                    osd.draw_line(a[0], a[1], b[0], b[1], color=col, thickness=2)
            pcol = mono if mono else config.COLOR_POINT
            for (px, py) in h.points:
                osd.draw_circle(px, py, 2, color=pcol, fill=True)

            wx, wy = h.wrist
            if h.intent == "PASS":
                self._text(wx - 24, wy + 6, "passing", config.COLOR_PASSING, config.FONT_LABEL)
            elif h.intent == "SEEN":
                dots = "." * (1 + (self.t // 6) % 3)
                self._text(wx - 10, wy + 6, dots, config.COLOR_TRACKING, config.FONT_READOUT)
            elif gesture.thumbs_up(h.points):
                self._text(wx - 16, wy + 6, "GOOD", config.COLOR_LOCKED, config.FONT_READOUT)
            else:
                self._text(wx - 10, wy + 6, "%d%%" % h.percent,
                           config.COLOR_READOUT, config.FONT_READOUT)

    # ---------------- frame ----------------
    def draw(self, state, menu, device, zone_ctrl, hands, perf):
        self.t += 1
        self.pl.osd_img.clear()

        if state == config.STATE_MENU:
            self._draw_menu(menu)
            dev_name = ""
        elif state == config.STATE_DEVICE:
            self._draw_device(device)
            dev_name = device.name
        else:
            self._draw_zones(zone_ctrl.zones)
            dev_name = ""

        self._draw_hands(hands)
        self._draw_header(state, dev_name)
        self._draw_perf(perf)
        self._draw_burst()