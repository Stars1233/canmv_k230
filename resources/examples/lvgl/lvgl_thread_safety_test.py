# LVGL binding thread-safety stress test.
#
# This intentionally runs lv.task_handler() from two MicroPython threads. It
# also updates LVGL objects from a worker and runs a Python timer callback, so
# the LVGL mutex and GIL paths are exercised at the same time.

import _thread
import gc
import image
import os
import sys
import time
import uctypes

import lvgl as lv
from media.display import *
from media.media import *


DISPLAY_TYPE = Display.ST7701
DISPLAY_WIDTH = ALIGN_UP(800, 16)
DISPLAY_HEIGHT = 480


disp_imgs = None
display_started = False
lvgl_started = False
running = True
workers_started = 0
workers_done = 0
error_text = None

object_updates = 0
handler_calls = 0
timer_calls = 0

worker_bar = None
worker_label = None
callback_label = None
status_label = None
shared_point = None
timer_ref = None
source_image = None
source_img_obj = None
native_image_updates = 0


def report_error(source, exc):
    global error_text

    if error_text is None:
        error_text = "%s: %s" % (source, exc)
    print("[LVGL thread test] %s" % error_text)
    try:
        sys.print_exception(exc)
    except Exception:
        pass


def lvgl_flush_cb(disp_drv, area, color):
    global disp_imgs

    try:
        if disp_drv.flush_is_last():
            color_ptr = uctypes.addressof(color.__dereference__())
            if disp_imgs[0].virtaddr() == color_ptr:
                Display.show_image(disp_imgs[0], layer=Display.LAYER_OSD0)
            else:
                Display.show_image(disp_imgs[1], layer=Display.LAYER_OSD0)
    finally:
        disp_drv.flush_ready()


def lvgl_setup():
    global disp_imgs, lvgl_started

    lv.init()
    lvgl_started = True

    disp_imgs = [
        image.Image(DISPLAY_WIDTH, DISPLAY_HEIGHT, image.BGRA8888),
        image.Image(DISPLAY_WIDTH, DISPLAY_HEIGHT, image.BGRA8888),
    ]
    disp_imgs[0].clear()
    disp_imgs[1].clear()

    disp_drv = lv.disp_create(DISPLAY_WIDTH, DISPLAY_HEIGHT)
    disp_drv.set_color_format(lv.COLOR_FORMAT.ARGB8888)
    disp_drv.set_draw_buffers(
        disp_imgs[0].bytearray(),
        disp_imgs[1].bytearray(),
        disp_imgs[0].size(),
        lv.DISP_RENDER_MODE.FULL,
    )
    disp_drv.set_flush_cb(lvgl_flush_cb)


def timer_cb(timer):
    global timer_calls

    try:
        timer_calls += 1
        point_x = shared_point.x
        point_y = shared_point.y

        callback_label.set_text(
            "Python timer callbacks: %d\nShared point read: (%d, %d)"
            % (timer_calls, point_x, point_y)
        )

        if error_text is None:
            status_label.set_text(
                "PASS - objects: %d   image: %d   handlers: %d   callbacks: %d"
                % (object_updates, native_image_updates, handler_calls, timer_calls)
            )
        else:
            status_label.set_text("FAIL - " + error_text)
    except BaseException as exc:
        report_error("timer callback", exc)


def build_ui():
    global callback_label, shared_point, status_label, timer_ref, worker_bar
    global source_image, source_img_obj

    scr = lv.scr_act()
    scr.set_style_bg_color(lv.color_hex(0x101820), 0)

    title = lv.label(scr)
    title.set_text("LVGL thread safety test")
    title.set_style_text_color(lv.color_hex(0xFFFFFF), 0)
    title.align(lv.ALIGN.TOP_MID, 0, 26)

    description = lv.label(scr)
    description.set_text(
        "Main and worker threads call LVGL concurrently\n"
        "No user lock is used around the LVGL objects"
    )
    description.set_width(DISPLAY_WIDTH - 40)
    description.set_style_text_color(lv.color_hex(0xB8C7D9), 0)
    description.align(lv.ALIGN.TOP_MID, 0, 62)

    worker_bar = lv.bar(scr)
    worker_bar.set_size(560, 30)
    worker_bar.set_range(0, 100)
    worker_bar.set_value(0, lv.ANIM.OFF)
    worker_bar.align(lv.ALIGN.CENTER, 0, -38)

    # This exercises the native image -> LVGL setter from the worker thread.
    source_image = image.Image(160, 120, image.RGB565)
    source_image.clear()
    source_img_obj = lv.img(scr)
    source_img_obj.align(lv.ALIGN.BOTTOM_MID, 0, -56)
    source_image.as_lvgl_img_src(source_img_obj)

    callback_label = lv.label(scr)
    callback_label.set_text("Python timer callbacks: 0\nShared point read: (0, 0)")
    callback_label.set_style_text_color(lv.color_hex(0x80D8FF), 0)
    callback_label.align(lv.ALIGN.CENTER, 0, 42)

    status_label = lv.label(scr)
    status_label.set_width(DISPLAY_WIDTH - 40)
    status_label.set_style_text_color(lv.color_hex(0x80FF9B), 0)
    status_label.align(lv.ALIGN.BOTTOM_MID, 0, -26)

    # Struct member reads and writes are also made from different threads.
    shared_point = lv.point_t({"x": 0, "y": 0})
    timer_ref = lv.timer_create(timer_cb, 100, None)


def object_worker():
    global object_updates, worker_label, workers_done, native_image_updates

    try:
        # The object is constructed from the worker after the main UI exists.
        worker_label = lv.label(lv.scr_act())
        worker_label.set_style_text_color(lv.color_hex(0xFFD166), 0)
        worker_label.align(lv.ALIGN.CENTER, 0, -2)

        value = 0
        direction = 1
        while running:
            os.exitpoint()
            value += direction
            if value >= 100:
                value = 100
                direction = -1
            elif value <= 0:
                value = 0
                direction = 1

            worker_bar.set_value(value, lv.ANIM.OFF)
            shared_point.x = value
            shared_point.y = 100 - value

            # Read the same struct from this thread before the timer callback
            # reads it from whichever thread is running the handler.
            point_x = shared_point.x
            point_y = shared_point.y
            worker_label.set_text(
                "Worker object updates: %d\nShared point write/read: (%d, %d)"
                % (object_updates, point_x, point_y)
            )
            source_image.as_lvgl_img_src(source_img_obj)
            native_image_updates += 1
            object_updates += 1
            time.sleep_ms(2)
    except BaseException as exc:
        report_error("object worker", exc)
    finally:
        workers_done += 1


def handler_worker():
    global handler_calls, workers_done

    try:
        while running:
            os.exitpoint()
            delay = lv.task_handler()
            handler_calls += 1
            if delay is None or delay < 1:
                delay = 1
            elif delay > 5:
                delay = 5
            time.sleep_ms(delay)
    except BaseException as exc:
        report_error("handler worker", exc)
    finally:
        workers_done += 1


def start_workers():
    global workers_started

    _thread.start_new_thread(object_worker, ())
    workers_started += 1
    _thread.start_new_thread(handler_worker, ())
    workers_started += 1


def stop_workers():
    global running

    running = False
    deadline = time.ticks_add(time.ticks_ms(), 1000)
    while workers_done < workers_started:
        if time.ticks_diff(deadline, time.ticks_ms()) <= 0:
            break
        time.sleep_ms(10)


def main():
    global display_started

    try:
        Display.init(DISPLAY_TYPE, width=DISPLAY_WIDTH, height=DISPLAY_HEIGHT, to_ide=True)
        display_started = True
        lvgl_setup()
        build_ui()
        start_workers()

        print("LVGL thread safety test started")
        print("Run it with the thread-safe LVGL binding and watch for FAIL/crashes")

        while True:
            os.exitpoint()
            delay = lv.task_handler()
            if delay is None or delay < 1:
                delay = 1
            elif delay > 10:
                delay = 10
            time.sleep_ms(delay)
    except KeyboardInterrupt:
        print("LVGL thread safety test stopped")
    except BaseException as exc:
        report_error("main", exc)
    finally:
        stop_workers()
        if workers_done < workers_started:
            print("LVGL workers did not stop; skipping LVGL deinit")
        elif lvgl_started:
            try:
                lv.deinit()
            except BaseException as exc:
                report_error("lv.deinit", exc)

        if display_started:
            try:
                Display.deinit()
            except BaseException as exc:
                report_error("Display.deinit", exc)
        gc.collect()


if __name__ == "__main__":
    main()
