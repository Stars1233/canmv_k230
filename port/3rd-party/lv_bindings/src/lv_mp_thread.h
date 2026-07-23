/*
 * Serialize LVGL access across MicroPython threads and native callbacks.
 *
 * When both locks are needed, callers must use this order:
 *   1. release the MicroPython GIL;
 *   2. acquire the LVGL mutex;
 *   3. reacquire the GIL while holding the LVGL mutex.
 *
 * Native LVGL callbacks follow the reverse entry path: they acquire the
 * LVGL mutex first and then the GIL. Never wait for the LVGL mutex while
 * holding the GIL.
 */
#ifndef LV_MP_THREAD_H
#define LV_MP_THREAD_H

void lv_mp_thread_lock(void);
void lv_mp_thread_unlock(void);

#endif
