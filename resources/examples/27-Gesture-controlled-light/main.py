import gc
import time

import sys
sys.path.insert(0, "/sdcard/examples/27-Gesture-controlled-light")
import camera
import led_control
import gesture
import intent
import zones
import menu as menu_mod
import device as device_mod
import perf
import ui
import config
from hand_keypoint import HandKeyPointDet


def run(pl, display_size, hkd, leds):
    tracker = intent.IntentTracker(display_size)
    menu = menu_mod.MenuController(leds, display_size)
    device = device_mod.DeviceController(leds, display_size)
    region = zones.ZoneController(leds, display_size)
    dash = ui.Dashboard(pl, display_size, leds)
    telemetry = perf.Perf()

    # initial state
    state = config.STATE_MENU
    thumb_latched = False
    reopen_ms = 0
    cooldown_ms = 0
    frame_i = 0

    while True:
        telemetry.begin()      # performance telemetry start
        dt = telemetry.dt
        img = pl.get_frame()

        t0 = time.ticks_ms()

        # run the hand detector and keypoint model
        _boxes, hand_res = hkd.run(img)
        infer_ms = time.ticks_diff(time.ticks_ms(), t0)

        # gesture processing
        hands = gesture.parse(hand_res)
        tracker.update(hands, dt)

        if cooldown_ms > 0:
            cooldown_ms -= dt

        # detect thumbs up
        ctrl = intent.controlling(hands)
        is_thumb = len(ctrl) == 1 and gesture.thumbs_up(ctrl[0].points)        # check exactly one hand + thumbs up

        # menu state
        if state == config.STATE_MENU:
            picked = menu.update(hands, dt)
            if picked is not None:
                label, kind, grp = menu.items[picked]
                if kind == "device":
                    device.select(label, grp)
                    state = config.STATE_DEVICE
                else:
                    state = config.STATE_REGION
                dash.notify_select((menu.cx, menu.cy))
                thumb_latched = True
                reopen_ms = 0
                cooldown_ms = config.MENU_COOLDOWN_MS
        else:
            if state == config.STATE_DEVICE:
                # open device page
                device.update(hands, dt)
            else:
                # open region page
                region.update(hands, dt)

            # if user releases thumbs up
            if not is_thumb:
                thumb_latched = False
                reopen_ms = 0

            # if thumb is held and not latched, start counting to reopen menu
            elif not thumb_latched and cooldown_ms <= 0:
                reopen_ms += dt

                # if held long enough, return to menu, reset timers, start cooldown
                if reopen_ms >= config.THUMB_CONFIRM_MS:
                    state = config.STATE_MENU
                    menu.reset()
                    thumb_latched = True
                    reopen_ms = 0
                    cooldown_ms = config.MENU_COOLDOWN_MS

        # performance telemetry end to calculate fps, frame time, inference time, etc.
        telemetry.end(infer_ms)

        # draw ui
        dash.draw(state, menu, device, region, hands, telemetry)
        pl.show_image()

        frame_i += 1
        if frame_i % config.GC_INTERVAL == 0:
            gc.collect()


if __name__ == "__main__":
    leds = led_control.create_leds()    # create LED objects to control the PWM channels
    pl = None
    try:
        pl, display_size = camera.create_pipeline()
        hkd = HandKeyPointDet(display_size)
        run(pl, display_size, hkd, leds)
    except KeyboardInterrupt:
        pass
    finally:
        led_control.shutdown(leds)
        if pl:
            pl.destroy()
        print("stopped, LEDs off")
