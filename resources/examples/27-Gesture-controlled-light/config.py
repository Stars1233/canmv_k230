# ---------------- Display / camera ----------------
DISPLAY_MODE = "lcd"            # "lcd" / "hdmi" / ...
DISPLAY_SIZE = [800, 480]
RGB888P_SIZE = [1920, 1080]
SENSOR_SIZE  = [1920, 1080]

# ---------------- LED hardware ----------------
LED_PINS = (("LED1", 42, 0),    # (name, IO pin, PWM channel)
            ("LED2", 43, 1))
PWM_FREQ    = 1000

# ---------------- Model ----------------
HAND_DET_KMODEL = "/sdcard/examples/kmodel/hand_det.kmodel"
HAND_KP_KMODEL  = "/sdcard/examples/kmodel/handkp_det.kmodel"

DET_INPUT_SIZE = [512, 512]
KP_INPUT_SIZE  = [256, 256]

CONFIDENCE_THRESHOLD = 0.30
NMS_THRESHOLD        = 0.50
NMS_OPTION           = False
LABELS               = ["hand"]
ANCHORS              = [26, 27, 53, 52, 75, 71, 80, 99, 106, 82,
                        99, 134, 140, 113, 161, 172, 245, 276]
STRIDES              = [8, 16, 32]

DET_PAD_COLOR = [114, 114, 114]
KP_CROP_RATIO = 1.26
MIN_HAND_H_RATIO = 0.10
EDGE_FILTERS     = ((0.25, 0.03), (0.15, 0.01))


# ---------------- Gesture -> brightness ----------------
CURL_CLOSED = 1.10              # extension value that maps to 0% brightness
CURL_OPEN   = 1.95              # extension value that maps to 100% brightness

SMOOTH        = 0.30            # 0..1, how fast brightness follows the hand
ENGAGE_FRAMES = 3               # frames a zone needs exactly 1 hand before control


# ---------------- Thumbs-up confirm gesture ----------------
THUMB_CONFIRM_MS = 400          # hold the pose this long to confirm / reopen
THUMB_RELEASE_MS = 150          # pose must drop this long before it counts again
MENU_COOLDOWN_MS = 700          # dead time after any menu open/close

# ---------------- App states ----------------
STATE_MENU   = 0                # radial menu open
STATE_DEVICE = 1                # whole frame controls one chosen device
STATE_REGION = 2                # 3-region split

# ---------------- AI intent detection ----------------
INTENT_ENABLED   = True
INTENT_TIME_MS   = 400          # ms a hand must stay deliberate before it LOCKs


INTENT_SLOW      = 0.50         # speed below this = deliberate
INTENT_FAST      = 1.60         # speed above this = passing hand
TRACK_MATCH_DIST = 0.25         # max frame-to-frame jump to keep a track
TRACK_TTL        = 4            # how many frames a tracked hand is kept in memory after the hand is no longer detected.
EXIT_MARGIN      = 0.02         # fraction of frame width/height that counts as "exited"

# ---------------- Steady gate ----------------
# Brightness only changes while the wrist is holding still.
MOVE_STEADY = 0.55              # wrist speed (screen widths/s) below this = holding still
STEADY_MS   = 120               # must hold still this long before control resumes
MAX_STEP    = 20.0              # max % the LED can move in one frame (safety cap only)

# ---------------- Sanity checks ----------------
MIN_SPAN_RATIO   = 0.25         # wrist-to-knuckle span vs hand extent; below this the pose is impossible
WARN_HOLD_FRAMES = 3            # frames a zone safety banner stays up after the conflict clears

# ---------------- Performance ----------------
PERF_H       = 62               # height of the bottom telemetry strip
DET_INTERVAL = 3                # run the full 512x512 hand DETECTOR only every n frames
TRACK_EXPAND = 0.30             # margin added around the tracked keypoint box
GC_INTERVAL  = 8                # frames between gc.collect() calls


# ---------------- Radial menu ----------------
MENU_ITEMS  = (("LED1",   "device", (0,)),
               ("BOTH",   "device", (0, 1)),
               ("LED2",   "device", (1,)),
               ("REGION", "region", ()))
WHEEL_R_OUT = 168               # outer ring radius (px)
WHEEL_R_IN  = 94                # inner ring radius (px)
WHEEL_GRAB  = 70                # extra reach outside the ring that still hovers

# ---------------- Menu / header ----------------
HEADER_H = 44

# ---------------- Zones (classic mode) ----------------
ZONES = (("LED2", (1,)),        # LEFT
         ("BOTH", (0, 1)),      # CENTER
         ("LED1", (0,)))        # RIGHT

# ---------------- UI colors ----------------
COLOR_DIVIDER = (255, 80, 80, 80)
COLOR_WARN    = (255, 255, 40, 40)
COLOR_ACTIVE  = (255, 0, 255, 0)
COLOR_IDLE    = (255, 180, 180, 180)
COLOR_POINT   = (255, 0, 255, 0)
COLOR_READOUT = (255, 255, 255, 0)

COLOR_ACCENT      = (255, 0, 210, 255)      # cyan accent
COLOR_ACCENT_DIM  = (255, 0, 110, 140)
COLOR_PANEL       = (150, 16, 20, 28)       # translucent dark panel
COLOR_PANEL_LINE  = (255, 70, 80, 100)
COLOR_METRIC      = (255, 235, 235, 235)
COLOR_METRIC_DIM  = (255, 150, 155, 165)
COLOR_BAR_BG      = (255, 55, 60, 72)
COLOR_BAR         = (255, 0, 220, 180)
COLOR_BAR_HOT     = (255, 255, 150, 40)

COLOR_PASSING     = (255, 130, 130, 140)    # grey - ignored hand
COLOR_TRACKING    = (255, 255, 210, 40)     # amber - building intent
COLOR_LOCKED      = (255, 0, 255, 120)      # green - in control

COLOR_WHEEL_RING  = (255, 70, 82, 105)      # idle segment outline
COLOR_WHEEL_FILL  = (90, 22, 34, 52)        # idle segment fill (translucent)
COLOR_WHEEL_HOVER = (255, 0, 210, 255)      # cyan - pointing at it
COLOR_WHEEL_HFILL = (110, 0, 150, 190)      # hover fill
COLOR_WHEEL_SEL   = (255, 0, 255, 140)      # green - confirmed
COLOR_WHEEL_SFILL = (150, 0, 150, 90)       # confirmed fill
COLOR_WHEEL_GLOW  = (140, 0, 210, 255)      # outer glow / rotating dashes
COLOR_HUB         = (230, 12, 16, 24)       # hub disc
COLOR_WHITE       = (255, 255, 255, 255)    # menu labels


FINGER_COLORS = ((255, 255, 0, 0), (255, 255, 0, 255), (255, 255, 255, 0),
                 (255, 0, 255, 0), (255, 0, 0, 255))

FONT_TITLE   = 22
FONT_ZONE    = 24
FONT_INFO    = 20
FONT_WARN    = 26
FONT_READOUT = 20
FONT_METRIC  = 18
FONT_LABEL   = 14
FONT_BIG     = 40