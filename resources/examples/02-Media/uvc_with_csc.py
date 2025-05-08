import time, os, urandom, sys

from media.display import *
from media.media import *
from media.uvc import *

from nonai2d import CSC

DISPLAY_WIDTH = ALIGN_UP(800, 16)
DISPLAY_HEIGHT = 480

csc = CSC(0, CSC.PIXEL_FORMAT_RGB_565)

# use lcd as display output
Display.init(Display.ST7701, width = DISPLAY_WIDTH, height = DISPLAY_HEIGHT, to_ide = True)
# init media manager
MediaManager.init()

while True:
    plugin, dev = UVC.probe()
    if plugin:
        print(f"detect USB Camera {dev}")
        break
    time.sleep_ms(100)

mode = UVC.video_mode(640, 480, UVC.FORMAT_MJPEG, 30)

succ, mode = UVC.select_video_mode(mode)
print(f"select mode success: {succ}, mode: {mode}")

UVC.start(cvt = True)

clock = time.clock()

while True:
    clock.tick()

    img = UVC.snapshot()
    if img is not None:
        img = csc.convert(img)
        Display.show_image(img)
        img.__del__()

    print(f"fps: {clock.fps()}")

# deinit display
Display.deinit()
csc.destroy()
UVC.stop()
time.sleep_ms(100)
# release media buffer
MediaManager.deinit()
