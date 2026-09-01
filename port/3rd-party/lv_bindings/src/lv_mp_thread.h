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
 *
 * Once both locks are held, generated bindings keep the GIL while LVGL runs.
 * LVGL allocates from the MicroPython heap (LV_MALLOC is m_malloc), and
 * gc_alloc()/gc_free()/gc_collect() are only mutually exclusive because the
 * GIL serializes them. Dropping the GIL around a native LVGL call would let
 * two threads mutate the heap metadata at once, so do not "optimize" this by
 * releasing the GIL for the duration of a call. The per-thread Python-call
 * depth lets synchronous LVGL callbacks reuse the GIL already held by the
 * calling binding.
 *
 * The cost is that a long LVGL call (lv_timer_handler() in particular) blocks
 * every other MicroPython thread. Use lv_mp_thread_gil_yield() at points where
 * LVGL is between operations - no allocation in flight and all allocated
 * blocks linked into a structure reachable from lvgl_root_pointers - to give
 * those threads a slot. The LVGL mutex stays held across the yield, so no
 * other thread can enter LVGL while it happens.
 */
#ifndef LV_MP_THREAD_H
#define LV_MP_THREAD_H

void lv_mp_thread_lock(void);
void lv_mp_thread_unlock(void);
void lv_mp_thread_python_call_enter(void);
void lv_mp_thread_python_call_exit(void);
int lv_mp_thread_python_call_active(void);
void lv_mp_thread_gil_yield(void);

#endif
