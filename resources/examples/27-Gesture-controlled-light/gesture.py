import math
import config

KEYPOINT_COUNT = 21
FINGER_COUNT   = 5

WRIST      = 0
FINGER_MCP = (5, 9, 13, 17)     # index, middle, ring, pinky knuckles
FINGER_TIP = (8, 12, 16, 20)    # index, middle, ring, pinky tips (thumb ignored)
THUMB_MCP  = 2                  # thumb base joint
THUMB_TIP  = 4                  # thumb tip

# Each finger is wrist -> 4 joints, used for drawing the skeleton.
FINGER_CHAINS = tuple((WRIST, f * 4 + 1, f * 4 + 2, f * 4 + 3, f * 4 + 4)
                      for f in range(FINGER_COUNT))


def _dist(a, b):
    return math.sqrt((a[0] - b[0]) ** 2 + (a[1] - b[1]) ** 2)


def _openness(points):
    # Most-extended finger wins, so a single raised finger still dims/brightens.
    best = 0.0
    for tip, mcp in zip(FINGER_TIP, FINGER_MCP):
        # calculate finger base distance
        base = _dist(points[mcp], points[WRIST])
        if base < 1:
            base = 1.0

        # calculate finger tip distance
        ext = _dist(points[tip], points[WRIST]) / base
        if ext > best:
            best = ext

    # If fingers are extended, openness is 1.0. If fingers are curled, openness is 0.0.
    o = (best - config.CURL_CLOSED) / (config.CURL_OPEN - config.CURL_CLOSED)
    if o < 0.0:
        o = 0.0
    if o > 1.0:
        o = 1.0
    return o


FINGER_PIP = (6, 10, 14, 18)


def plausible(points):
    # Reject impossible hand geometry, e.g. a hand exiting wrist-first.
    xs = [p[0] for p in points]
    ys = [p[1] for p in points]
    extent = max(max(xs) - min(xs), max(ys) - min(ys))
    if extent < 1:
        return False
    span = _dist(points[FINGER_MCP[1]], points[WRIST])
    return span > config.MIN_SPAN_RATIO * extent


def thumbs_up(points):
    w = points[WRIST]
    span = _dist(points[FINGER_MCP[1]], w)
    if span < 1:
        span = 1.0

    tip_reach = 0.0

    # check for all fingers folded except thumb
    for tip, pip in zip(FINGER_TIP, FINGER_PIP):
        d_tip = _dist(points[tip], w)
        d_pip = _dist(points[pip], w)
        if d_tip > d_pip * 1.05:
            return False
        if d_tip > tip_reach:
            tip_reach = d_tip

    thumb_reach = _dist(points[THUMB_TIP], w)
    if thumb_reach < tip_reach + 0.35 * span:       # thumb must stick out
        return False

    ty = points[THUMB_TIP][1]
    if ty > w[1] - 0.45 * span:                     # thumb clearly above wrist
        return False
    for tip in FINGER_TIP:
        if ty > points[tip][1] - 0.30 * span:       # and above every fingertip
            return False
    return True



class Hand:
    __slots__ = ("points", "cx", "cy", "openness", "intent", "speed", "steady")

    def __init__(self, points):
        self.points = points                                 # [(x, y)] * 21
        self.cx = sum(p[0] for p in points) / len(points)    # display coords
        self.cy = sum(p[1] for p in points) / len(points)
        self.openness = _openness(points)                    # 0.0 .. 1.0
        self.intent = "LOCK"                                 # set by IntentTracker
        self.speed = 0.0                                     # screen widths / second
        self.steady = True                                   # set by IntentTracker

    @property
    def wrist(self):
        return self.points[WRIST]

    @property
    def percent(self):
        return int(self.openness * 100)


def parse(hand_res):
    hands = []
    for res in hand_res:
        points = [(int(res[i * 2]), int(res[i * 2 + 1]))
                  for i in range(KEYPOINT_COUNT)]
        if not plausible(points):
            continue
        hands.append(Hand(points))
    return hands