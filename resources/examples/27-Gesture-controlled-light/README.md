# 💡 AI Gesture-Controlled Remote Lighting

Real-time **touchless light control** on the **K230 / CanMV MicroPython** platform. AI hand detection and hand keypoint estimation turn the camera into the only control surface — no buttons, no remote, no touch screen.

---

## 📋 Prerequisites

### Hardware

- **K230 development board**
- **LCD display** (HDMI also supported via `DISPLAY_MODE`)
- **Camera module** on the CSI interface
- **2 × LED lights** driven by the K230 PWM pins
- SD card

### LED Wiring

Both LED modules are driven by hardware PWM, so brightness is a real analog dim rather than an on/off switch.

| LED | Signal | K230 Pin | PWM Channel |
|---|---|---|---|
| LED 1 | `IN` | **IO42** | PWM0 |
| LED 2 | `IN` | **IO43** | PWM1 |
| Both | `VCC` | **3V3** | — |
| Both | `GND` | **GND** | — |

```
       K230                          LED 1
    +----------+                  +---------+
    |     IO42 |----------------->| IN      |
    |      3V3 |----------------->| VCC     |
    |      GND |----------------->| GND     |
    |          |                  +---------+
    |          |
    |          |                     LED 2
    |     IO43 |----------------->| IN      |
    |      3V3 |----------------->| VCC     |
    |      GND |----------------->| GND     |
    +----------+                  +---------+
```

### Software

- CanMV IDE
- K230 MicroPython firmware — [Kendryte K230 Resource Center](https://www.kendryte.com/en/resource/images,k230)

---

## ⚙️ Setup

### 1. Flash Firmware

This project supports different K230 boards — flash the **MicroPython image suitable for your own board**. After flashing, verify the board runs MicroPython and connects to CanMV IDE.

### 2. Copy Project Files

```bash
git clone git@g.a-bug.org:canmv/k230/demo_apps.git
```

Copy the `remote-light` folder to the SD card. The path must be exactly `/sdcard/remote-light` — the models are loaded through that path.

```
SD Card
│
└── remote-light/
    ├── main.py           # entry point: capture → detect → gesture → state machine loop
    ├── config.py         # all tunable parameters
    ├── camera.py         # sensor and display setup
    ├── hand_keypoint.py  # hand detection + 21-point keypoint models
    ├── gesture.py        # hand openness and thumbs-up recognition
    ├── intent.py         # passing-hand filter (frame-to-frame tracking)
    ├── menu.py           # radial gesture menu
    ├── device.py         # whole-frame single-device control
    ├── zones.py          # 3-region split control
    ├── led_control.py    # PWM LED driver
    ├── perf.py           # performance telemetry
    ├── ui.py             # on-screen dashboard
    └── model/
        ├── hand_det.kmodel
        └── handkp_det.kmodel
```

### 3. Models

The system uses two models chained together:

| Model | Input | Purpose |
|---|---|---|
| **`hand_det`** | 512 × 512 | Locates hands in the frame |
| **`handkp_det`** | 256 × 256 | Returns 21 keypoints per detected hand |

### 4. Run

Open **CanMV IDE**, open `remote-light/main.py`, and run it on the K230 board.

---

## 🧩 How It Works

### 1. Radial Menu

On startup a radial menu opens with four options. Point your hand at a segment to hover it (it turns cyan), then hold a **thumbs-up** to confirm. A progress arc sweeps the segment while you hold.

```
              LED1
        \      |      /
         \     |     /
   REGION  \   |   /   BOTH
   ---------  ( )  ---------
           /   |   \
          /    |    \
         /     |     \
              LED2
```

| Option | Result |
|---|---|
| **LED1** | Whole frame controls LED 1 |
| **BOTH** | Whole frame controls both LEDs together |
| **LED2** | Whole frame controls LED 2 |
| **REGION** | Splits the screen into 3 independent control zones |

### 2. Device Mode

Picking a device gives you whole-frame control — no aiming required. Any single hand's openness sets the brightness:

```
Fist          Half open        Open hand
 0%     →        50%        →     100%
```

A large centred readout shows the device name, its percentage and a brightness ring.

### 3. Region Mode

Picking **REGION** splits the camera view into three vertical zones. Put your hand in a zone and its openness drives that zone's device directly.

```
+-------------+-------------+-------------+
|             |             |             |
|    LEFT     |   CENTER    |    RIGHT    |
|    LED2     |    BOTH     |    LED1     |
|             |             |             |
+-------------+-------------+-------------+
```

### 4. Reopening the Menu

While controlling in either mode, give a **thumbs-up** to reopen the menu.
---

## 🧠 AI Intent Detection

Hands are tracked frame to frame and given a speed. A hand only gains control after it has stayed slow long enough to **LOCK**:

| State | Colour | Meaning |
|---|---|---|
| **PASS** | Grey | Moving fast — passing traffic, ignored everywhere |
| **SEEN** | Amber | Slowing down, building intent |
| **LOCK** | Green | Deliberate — allowed to control |

A bystander walking past never touches the lights or the menu. Set `INTENT_ENABLED = False` to disable the filter entirely.

---

## 🔒 Multi-Person Safety

Two or more intentional hands on the same target is ambiguous, so control **freezes** and a pulsing red **SAFETY LOCK** banner appears — in the menu, in device mode, and per zone in region mode.

---

## 📈 Real-Time Performance Dashboard

The bottom strip reports live telemetry, with each bar turning amber when it runs hot:

- FPS and frame time
- CPU load
- Inference time (ms)
- Heap usage % and free KB
- Live PWM duty per LED
- Capture → display latency

---

## 🎛️ Configuration

All parameters live in `config.py`.

### LED Hardware

```python
# ---------------- LED hardware ----------------
LED_PINS = (("LED1", 42, 0),    # (name, IO pin, PWM channel)
            ("LED2", 43, 1))
PWM_FREQ = 1000
```

Change the IO pin and PWM channel here if your wiring differs. Adding a third entry adds a third controllable light.

### Gesture → Brightness Mapping

```python
# ---------------- Gesture -> brightness ----------------
CURL_CLOSED = 1.10              # extension value that maps to 0% brightness
CURL_OPEN   = 1.95              # extension value that maps to 100% brightness
SMOOTH      = 0.40              # 0..1, how fast brightness follows the hand
```

`CURL_CLOSED` / `CURL_OPEN` calibrate the range to your hand and camera distance. Lower `SMOOTH` for a slower, more cinematic fade; raise it for a snappier response.

### Intent Detection

```python
# ---------------- AI intent detection ----------------
INTENT_ENABLED = True
INTENT_TIME_MS = 400            # ms a hand must stay deliberate before it LOCKs
INTENT_SLOW    = 0.35           # speed below this = deliberate
INTENT_FAST    = 1.00           # speed above this = passing hand
```

Speeds are in screen widths per second. In a busy space, raise `INTENT_TIME_MS` so control requires a longer, clearer commitment.

### Key Parameters

| Parameter | Default | Purpose |
|---|---|---|
| `CURL_CLOSED` / `CURL_OPEN` | `1.10` / `1.95` | Hand extension range mapped to 0–100% |
| `SMOOTH` | `0.40` | How fast brightness follows the hand |
| `ENGAGE_FRAMES` | `3` | Frames a zone needs exactly 1 hand before control |
| `THUMB_CONFIRM_MS` | `400` | Hold time to confirm / reopen the menu |
| `THUMB_RELEASE_MS` | `150` | Pose must drop this long before it counts again |
| `MENU_COOLDOWN_MS` | `700` | Dead time after any menu open / close |
| `INTENT_TIME_MS` | `400` | Time a hand must stay deliberate before LOCK |
| `CONFIDENCE_THRESHOLD` | `0.30` | Hand detection confidence cutoff |
| `DET_INTERVAL` | `3` | Run the full hand detector every N frames |
| `PWM_FREQ` | `1000` | LED PWM frequency, Hz |

---

## 📊 System Block Diagram

```
Camera
  |
  v
Hand detection (hand_det)
  |
  v
Hand keypoints (handkp_det, 21 points)
  |
  v
Gesture parsing (openness + thumbs-up)
  |
  v
Intent tracking (PASS / SEEN / LOCK)
  |
  v
State machine (menu / device / region)
  |
  v
Multi-person safety check
  |
  v
PWM brightness output
```

---

## 🎯 Applications

- Touchless lighting in hospitals and clean rooms
- Hygienic control in kitchens and food preparation areas
- Meeting room and studio lighting control
- Smart home lighting without a remote or wall switch
- Accessible control for users who cannot reach physical switches
- Industrial environments where operators wear gloves
