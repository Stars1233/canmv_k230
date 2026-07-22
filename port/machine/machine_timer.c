/* Copyright (c) 2023, Canaan Bright Sight Co., Ltd
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 * 1. Redistributions of source code must retain the above copyright
 * notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 * notice, this list of conditions and the following disclaimer in the
 * documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND
 * CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES,
 * INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
 * BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
 * NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "generated/autoconf.h"

#include "py/mpprint.h"
#include "py/mperrno.h"
#include "py/obj.h"
#include "py/runtime.h"
#include "shared/runtime/mpirq.h"

#include "drv_timer.h"
#include "modmachine.h"

extern bool system_is_exiting(void);
extern void mp_irq_enter(void);
extern void mp_irq_exit(void);

// Global IRQ objects registration
MP_REGISTER_ROOT_POINTER(void* machine_timer_irq_obj[KD_TIMER_MAX_NUM]);
MP_REGISTER_ROOT_POINTER(void* machine_timer_soft_irq_obj);

// Global timer objects registration for tracking existing instances
MP_REGISTER_ROOT_POINTER(void* machine_timer_obj[KD_TIMER_MAX_NUM]);
MP_REGISTER_ROOT_POINTER(void* machine_timer_soft_obj);

#if defined(CONFIG_ENABLE_MODULE_UART_PERIODIC_TX)
// These helpers run with the MicroPython GIL held. The hard-timer callback
// never accesses this state, so the claim check and update are intentionally
// non-atomic.
STATIC bool machine_timer_native_claimed[KD_TIMER_MAX_NUM];

int machine_timer_native_claim(int timer_id)
{
    if (timer_id < 0 || timer_id >= KD_TIMER_MAX_NUM) {
        return -1;
    }
    if (machine_timer_native_claimed[timer_id] || MP_STATE_PORT(machine_timer_obj[timer_id]) != NULL) {
        return -1;
    }

    machine_timer_native_claimed[timer_id] = true;
    return 0;
}

void machine_timer_native_release(int timer_id)
{
    // The caller must already have stopped and destroyed its native timer.
    // While this flag is set, machine_timer_make_new() rejects a Python Timer
    // for the same ID, so a live Python timer cannot share this reservation.
    if (timer_id >= 0 && timer_id < KD_TIMER_MAX_NUM) {
        machine_timer_native_claimed[timer_id] = false;
    }
}
#endif

/** soft timer wrap **********************************************************/

/** timer python binding *****************************************************/

// Timer IRQ object - following machine_pin.c pattern
typedef struct _machine_timer_irq_obj_t {
    mp_irq_obj_t base;
    uint32_t     flags;
    uint32_t     trigger;
} machine_timer_irq_obj_t;

typedef struct {
    mp_obj_base_t base;

    int      type; // 0: hardware timer, 1: software timer
    int      id;
    int      mode;
    uint64_t period;
    bool     hard;
    bool volatile active;
    bool volatile scheduled;
    uint32_t volatile generation;
    uint32_t volatile scheduled_generation;

    mp_obj_t callback;

    union {
        drv_soft_timer_inst_t* soft;
        drv_hard_timer_inst_t* hard;
    } inst;
} machine_timer_obj_t;

STATIC const mp_irq_methods_t machine_timer_irq_methods;

STATIC void machine_timer_invalidate_callback(machine_timer_obj_t* self)
{
    __atomic_store_n(&self->active, false, __ATOMIC_RELEASE);
    __atomic_add_fetch(&self->generation, 1, __ATOMIC_ACQ_REL);
}

STATIC mp_obj_t machine_timer_scheduled_callback(mp_obj_t self_in)
{
    machine_timer_obj_t* self = MP_OBJ_TO_PTR(self_in);
    uint32_t scheduled_generation = __atomic_load_n(&self->scheduled_generation, __ATOMIC_ACQUIRE);

    __atomic_store_n(&self->scheduled, false, __ATOMIC_RELEASE);

    if (system_is_exiting() ||
        !__atomic_load_n(&self->active, __ATOMIC_ACQUIRE) ||
        scheduled_generation != __atomic_load_n(&self->generation, __ATOMIC_ACQUIRE)) {
        return mp_const_none;
    }

    mp_obj_t callback = self->callback;
    if (callback != mp_const_none && callback != MP_OBJ_NULL) {
        mp_call_function_1(callback, self_in);
    }

    return mp_const_none;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_1(machine_timer_scheduled_callback_obj, machine_timer_scheduled_callback);

STATIC void machine_timer_schedule_callback(machine_timer_obj_t* self)
{
    uint32_t generation = __atomic_load_n(&self->generation, __ATOMIC_ACQUIRE);
    bool expected = false;

    if (!__atomic_load_n(&self->active, __ATOMIC_ACQUIRE) ||
        !__atomic_compare_exchange_n(&self->scheduled, &expected, true, false,
                                     __ATOMIC_ACQ_REL, __ATOMIC_ACQUIRE)) {
        return;
    }

    // Do not associate an event from an old configuration with a new one.
    if (!__atomic_load_n(&self->active, __ATOMIC_ACQUIRE) ||
        generation != __atomic_load_n(&self->generation, __ATOMIC_ACQUIRE)) {
        __atomic_store_n(&self->scheduled, false, __ATOMIC_RELEASE);
        return;
    }

    __atomic_store_n(&self->scheduled_generation, generation, __ATOMIC_RELEASE);
    if (!mp_sched_schedule(MP_OBJ_FROM_PTR(&machine_timer_scheduled_callback_obj), MP_OBJ_FROM_PTR(self))) {
        __atomic_store_n(&self->scheduled, false, __ATOMIC_RELEASE);
    }
}

// Get existing timer object or create new one
STATIC machine_timer_obj_t* machine_timer_get_or_create(int index)
{
    machine_timer_obj_t* self = NULL;

    if (index == -1) {
        // Soft timer
        self = MP_STATE_PORT(machine_timer_soft_obj);
        if (self != NULL) {
            return self; // Return existing soft timer
        }
    } else {
        // Hard timer
        if (index >= 0 && index < KD_TIMER_MAX_NUM) {
            self = MP_STATE_PORT(machine_timer_obj[index]);
            if (self != NULL) {
                return self; // Return existing hard timer
            }
        }
    }

    // No existing timer found, return NULL to create new one
    return NULL;
}

// Register timer object in tracking system
STATIC void machine_timer_register_obj(machine_timer_obj_t* self, int index)
{
    if (index == -1) {
        // Soft timer
        MP_STATE_PORT(machine_timer_soft_obj) = self;
    } else {
        // Hard timer
        if (index >= 0 && index < KD_TIMER_MAX_NUM) {
            MP_STATE_PORT(machine_timer_obj[index]) = self;
        }
    }
}

// Unregister timer object from tracking system
STATIC void machine_timer_unregister_obj(machine_timer_obj_t* self, int index)
{
    if (index == -1) {
        // Soft timer
        MP_STATE_PORT(machine_timer_soft_obj) = NULL;
    } else {
        // Hard timer
        if (index >= 0 && index < KD_TIMER_MAX_NUM) {
            MP_STATE_PORT(machine_timer_obj[index]) = NULL;
        }
    }
}

STATIC void machine_timer_handler(void* args)
{
    machine_timer_obj_t* self = MP_OBJ_TO_PTR(args);

    if (self == NULL || &machine_timer_type != self->base.type) {
        return;
    }

    mp_irq_enter();

    if (!system_is_exiting() && __atomic_load_n(&self->active, __ATOMIC_ACQUIRE)) {
        machine_timer_irq_obj_t* irq = NULL;

        if (self->type == 0 && self->id >= 0 && self->id < KD_TIMER_MAX_NUM) {
            irq = MP_STATE_PORT(machine_timer_irq_obj[self->id]);
        } else if (self->type == 1) {
            irq = MP_STATE_PORT(machine_timer_soft_irq_obj);
        }

        if (irq != NULL) {
            irq->flags = irq->trigger;
            if (irq->base.ishard) {
                mp_irq_handler(&irq->base);
            } else {
                machine_timer_schedule_callback(self);
            }
        }
    }

    mp_irq_exit();
}

STATIC machine_timer_irq_obj_t* machine_timer_get_irq(machine_timer_obj_t* timer)
{
    machine_timer_irq_obj_t* irq = NULL;

    if (timer->type == 0) { /* hard timer */
        int timer_id = timer->id;
        if (timer_id >= 0 && timer_id < KD_TIMER_MAX_NUM) {
            // Get the IRQ object.
            irq = MP_STATE_PORT(machine_timer_irq_obj[timer_id]);

            // Allocate the IRQ object if it doesn't already exist.
            if (irq == NULL) {
                irq                 = m_new_obj(machine_timer_irq_obj_t);
                irq->base.base.type = &mp_irq_type;
                irq->base.methods   = (mp_irq_methods_t*)&machine_timer_irq_methods;
                irq->base.parent    = timer;
                irq->base.handler   = mp_const_none;
                irq->base.ishard    = false; // Will be set properly during init

                MP_STATE_PORT(machine_timer_irq_obj[timer_id]) = irq;
            }
        }
    } else { /* soft timer */
        // For soft timers, use a single global IRQ object
        irq = MP_STATE_PORT(machine_timer_soft_irq_obj);

        if (irq == NULL) {
            irq                 = m_new_obj(machine_timer_irq_obj_t);
            irq->base.base.type = &mp_irq_type;
            irq->base.methods   = (mp_irq_methods_t*)&machine_timer_irq_methods;
            irq->base.parent    = timer;
            irq->base.handler   = mp_const_none;
            irq->base.ishard    = false; // Will be set properly during init

            MP_STATE_PORT(machine_timer_soft_irq_obj) = irq;
        }
    }

    return irq;
}

STATIC mp_obj_t machine_timer_deinit(mp_obj_t self_in)
{
    machine_timer_obj_t* self = MP_OBJ_TO_PTR(self_in);

    machine_timer_invalidate_callback(self);
    self->callback = mp_const_none;

    if (0x00 == self->type) { /* hard */
        if (NULL == self->inst.hard) {
            return mp_const_none;
        }

        int timer_id = self->id;
        if (timer_id >= 0 && timer_id < KD_TIMER_MAX_NUM) {
            machine_timer_irq_obj_t* irq = MP_STATE_PORT(machine_timer_irq_obj[timer_id]);
            if (irq != NULL) {
                irq->base.handler = mp_const_none;
            }
            MP_STATE_PORT(machine_timer_irq_obj[timer_id]) = NULL;
            // Unregister timer object from tracking
            machine_timer_unregister_obj(self, timer_id);
        }

        drv_hard_timer_stop(self->inst.hard);
        drv_hard_timer_unregister_irq(self->inst.hard);
        drv_hard_timer_inst_destroy(&self->inst.hard);
    } else { /* soft */
        if (NULL == self->inst.soft) {
            return mp_const_none;
        }

        machine_timer_irq_obj_t* irq = MP_STATE_PORT(machine_timer_soft_irq_obj);
        if (irq != NULL) {
            irq->base.handler = mp_const_none;
        }
        MP_STATE_PORT(machine_timer_soft_irq_obj) = NULL;
        // Unregister timer object from tracking
        machine_timer_unregister_obj(self, -1);

        drv_soft_timer_stop(self->inst.soft);
        drv_soft_timer_unregister_irq(self->inst.soft);
        drv_soft_timer_destroy(&self->inst.soft);
    }

    return mp_const_none;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_1(machine_timer_deinit_obj, machine_timer_deinit);

STATIC void machine_timer_init_helper(machine_timer_obj_t* self, mp_uint_t n_args, const mp_obj_t* pos_args, mp_map_t* kw_args)
{
    enum { ARG_mode, ARG_freq, ARG_period, ARG_callback, ARG_hard };
    static const mp_arg_t allowed_args[] = {
        { MP_QSTR_mode, MP_ARG_INT, { .u_int = HWTIMER_MODE_PERIOD } },
        { MP_QSTR_freq, MP_ARG_INT, { .u_int = -1 } },
        { MP_QSTR_period, MP_ARG_INT, { .u_int = -1 } },
        { MP_QSTR_callback, MP_ARG_OBJ, { .u_obj = mp_const_none } },
        { MP_QSTR_hard, MP_ARG_KW_ONLY | MP_ARG_BOOL, { .u_bool = true } },
    };
    mp_arg_val_t args[MP_ARRAY_SIZE(allowed_args)];
    mp_arg_parse_all(n_args, pos_args, kw_args, MP_ARRAY_SIZE(allowed_args), allowed_args, args);

    mp_int_t mode    = args[ARG_mode].u_int;
    mp_int_t freq    = args[ARG_freq].u_int;
    mp_int_t period  = args[ARG_period].u_int;
    mp_obj_t handler = args[ARG_callback].u_obj;
    bool     hard    = args[ARG_hard].u_bool;

    if (freq != -1 && freq <= 0) {
        mp_raise_ValueError(MP_ERROR_TEXT("invalid freq"));
    }

    mp_int_t period_ms = period;
    if (freq != -1) {
        period_ms = 1000 / freq;
    }

    if ((mp_const_none == handler) || !mp_obj_is_callable(handler)) {
        mp_raise_ValueError(MP_ERROR_TEXT("invalid callback"));
    }

    if (period_ms < 5) {
        mp_raise_ValueError(MP_ERROR_TEXT("invalid period or freq, period should >= 5"));
    }

    // The native timer interfaces accept a signed 32-bit millisecond period.
    if (period_ms > INT_MAX) {
        mp_raise_ValueError(MP_ERROR_TEXT("invalid period or freq"));
    }

    if ((HWTIMER_MODE_PERIOD != mode) && (HWTIMER_MODE_ONESHOT != mode)) {
        mp_raise_ValueError(MP_ERROR_TEXT("invalid mode"));
    }

    int native_period_ms = (int)period_ms;

    if ((self->type == 0 && self->inst.hard == NULL) ||
        (self->type == 1 && self->inst.soft == NULL)) {
        mp_raise_ValueError(MP_ERROR_TEXT("timer is deinitialized"));
    }

    machine_timer_invalidate_callback(self);

    if (self->type == 0) { /* hard */
        if (drv_hard_timer_is_started(self->inst.hard)) {
            drv_hard_timer_stop(self->inst.hard);
        }
    } else { /* soft */
        if (drv_soft_timer_is_started(self->inst.soft)) {
            drv_soft_timer_stop(self->inst.soft);
        }
    }

    self->mode     = (int)mode;
    self->period   = native_period_ms;
    self->hard     = hard;
    self->callback = handler;

    // Get and initialize IRQ object
    machine_timer_irq_obj_t* irq = machine_timer_get_irq(self);
    if (irq == NULL) {
        mp_raise_msg(&mp_type_RuntimeError, MP_ERROR_TEXT("get timer IRQ failed"));
    }

    irq->base.handler = handler;
    irq->base.ishard  = hard;
    irq->base.parent  = self;
    irq->flags        = 0;
    irq->trigger      = 1; // Timer trigger

    if (0x00 == self->type) { /* hard */
        if (0x00 != drv_hard_timer_set_mode(self->inst.hard, mode)) {
            mp_raise_msg(&mp_type_RuntimeError, MP_ERROR_TEXT("set timer mode failed"));
        }

        if (0x00 != drv_hard_timer_set_period(self->inst.hard, native_period_ms)) {
            mp_raise_msg(&mp_type_RuntimeError, MP_ERROR_TEXT("set timer period failed"));
        }

        if (0x00 != drv_hard_timer_register_irq(self->inst.hard, machine_timer_handler, self)) {
            mp_raise_msg(&mp_type_RuntimeError, MP_ERROR_TEXT("register timer callback failed"));
        }

        if (0x00 != drv_hard_timer_start(self->inst.hard)) {
            machine_timer_invalidate_callback(self);
            mp_raise_msg(&mp_type_RuntimeError, MP_ERROR_TEXT("start timer failed"));
        }
    } else { /* soft */
        if (0x00 != drv_soft_timer_set_mode(self->inst.soft, mode)) {
            mp_raise_msg(&mp_type_RuntimeError, MP_ERROR_TEXT("set timer mode failed"));
        }

        if (0x00 != drv_soft_timer_set_period(self->inst.soft, native_period_ms)) {
            mp_raise_msg(&mp_type_RuntimeError, MP_ERROR_TEXT("set timer period failed"));
        }

        if (0x00 != drv_soft_timer_register_irq(self->inst.soft, machine_timer_handler, self)) {
            mp_raise_msg(&mp_type_RuntimeError, MP_ERROR_TEXT("register timer callback failed"));
        }

        if (0x00 != drv_soft_timer_start(self->inst.soft)) {
            machine_timer_invalidate_callback(self);
            mp_raise_msg(&mp_type_RuntimeError, MP_ERROR_TEXT("start timer failed"));
        }
    }

    __atomic_store_n(&self->active, true, __ATOMIC_RELEASE);
}

STATIC mp_obj_t machine_timer_init(mp_uint_t n_args, const mp_obj_t* args, mp_map_t* kw_args)
{
    machine_timer_obj_t* self = MP_OBJ_TO_PTR(args[0]);

    machine_timer_init_helper(self, n_args - 1, args + 1, kw_args);

    return mp_const_none;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_KW(machine_timer_init_obj, 0, machine_timer_init);

STATIC mp_obj_t machine_timer_make_new(const mp_obj_type_t* type, size_t n_args, size_t n_kw, const mp_obj_t* args)
{
    mp_arg_check_num(n_args, n_kw, 1, 5, true);

    mp_int_t raw_index = mp_obj_get_int(args[0]);
    if (raw_index != -1 && (raw_index < 0 || raw_index >= KD_TIMER_MAX_NUM)) {
        mp_raise_ValueError(MP_ERROR_TEXT("invalid timer number"));
    }
    int index = (int)raw_index;
    #if defined(CONFIG_ENABLE_MODULE_UART_PERIODIC_TX)
    if (index >= 0 && machine_timer_native_claimed[index]) {
        mp_raise_OSError(MP_EBUSY);
    }
    #endif

    // Check if timer object already exists
    machine_timer_obj_t* self = machine_timer_get_or_create(index);
    if (self != NULL) {
        // Timer already exists, return existing instance
        return MP_OBJ_FROM_PTR(self);
    }

    // Create new timer object
    self = m_new_obj_with_finaliser(machine_timer_obj_t);

    self->base.type = &machine_timer_type;
    self->type      = -1;
    self->id        = index;
    self->callback  = mp_const_none;
    self->mode      = HWTIMER_MODE_ONESHOT;
    self->period    = 100;
    self->hard      = true;
    self->inst.soft = NULL;
    __atomic_store_n(&self->active, false, __ATOMIC_RELAXED);
    __atomic_store_n(&self->scheduled, false, __ATOMIC_RELAXED);
    __atomic_store_n(&self->generation, 0, __ATOMIC_RELAXED);
    __atomic_store_n(&self->scheduled_generation, 0, __ATOMIC_RELAXED);

    if ((-1) == index) {
        self->type = 1;

        if (0x00 != drv_soft_timer_create(&self->inst.soft)) {
            mp_raise_msg(&mp_type_RuntimeError, MP_ERROR_TEXT("create soft timer obj failed"));
        }
    } else {
        self->type = 0;

        if (0x00 != drv_hard_timer_inst_create(index, &self->inst.hard)) {
            mp_raise_msg_varg(&mp_type_RuntimeError, MP_ERROR_TEXT("create hard timer(%d) obj failed"), index);
        }
    }

    // Register the new timer object in tracking system
    machine_timer_register_obj(self, index);

    if ((n_args + n_kw) > 1) {
        mp_map_t kw_args;
        mp_map_init_fixed_table(&kw_args, n_kw, args + n_args);
        machine_timer_init_helper(self, n_args - 1, args + 1, &kw_args);
    }

    return MP_OBJ_FROM_PTR(self);
}

STATIC void machine_timer_print(const mp_print_t* print, mp_obj_t self_in, mp_print_kind_t kind)
{
    machine_timer_obj_t* self     = self_in;

    mp_printf(print, "Timer %d: period=%u ms, mode=%s, hard=%s, callback=%p\n", self->id, self->period,
              self->mode == HWTIMER_MODE_ONESHOT ? "oneshot" : "periodic", self->hard ? "True" : "False",
              self->callback);
}
//| # Auto-generated CanMV stub docs. Edit the signatures/docstrings here.
//| module: machine
//| class Timer:
//|     """machine.Timer object."""
//|     def __init__(self, index: int, /, mode: int = ..., freq: int = -1, period: int = -1, callback: Any = None, *, hard: bool = True) -> None:
//|         """Create a machine.Timer object. hard=True invokes in the timer IRQ; hard=False schedules normal Python execution."""
//|     def deinit(self, /) -> None:
//|         """Release resources held by machine.Timer."""
//|     def init(self, mode: int = ..., freq: int = -1, period: int = -1, callback: Any = None, *, hard: bool = True) -> None:
//|         """Configure and start the timer. hard=True invokes in the timer IRQ; hard=False schedules normal Python execution."""


STATIC const mp_rom_map_elem_t machine_timer_locals_dict_table[] = {
    { MP_ROM_QSTR(MP_QSTR___del__), MP_ROM_PTR(&machine_timer_deinit_obj) },
    { MP_ROM_QSTR(MP_QSTR_deinit), MP_ROM_PTR(&machine_timer_deinit_obj) },
    { MP_ROM_QSTR(MP_QSTR_init), MP_ROM_PTR(&machine_timer_init_obj) },

    { MP_ROM_QSTR(MP_QSTR_ONE_SHOT), MP_ROM_INT(HWTIMER_MODE_ONESHOT) },
    { MP_ROM_QSTR(MP_QSTR_PERIODIC), MP_ROM_INT(HWTIMER_MODE_PERIOD) },
};
STATIC MP_DEFINE_CONST_DICT(machine_timer_locals_dict, machine_timer_locals_dict_table);

/* clang-format off */
MP_DEFINE_CONST_OBJ_TYPE(
    machine_timer_type,
    MP_QSTR_Timer,
    MP_TYPE_FLAG_NONE,
    make_new, machine_timer_make_new,
    print, machine_timer_print,
    locals_dict, &machine_timer_locals_dict
);
/* clang-format on */

STATIC mp_uint_t machine_timer_irq_trigger(mp_obj_t self_in, mp_uint_t new_trigger)
{
    machine_timer_obj_t*     self = MP_OBJ_TO_PTR(self_in);
    machine_timer_irq_obj_t* irq  = machine_timer_get_irq(self);

    if (irq != NULL) {
        irq->flags   = 0;
        irq->trigger = new_trigger;
    }

    return 0;
}

STATIC mp_uint_t machine_timer_irq_info(mp_obj_t self_in, mp_uint_t info_type)
{
    machine_timer_obj_t*     self = MP_OBJ_TO_PTR(self_in);
    machine_timer_irq_obj_t* irq  = machine_timer_get_irq(self);

    if (irq == NULL) {
        return 0;
    }

    if (info_type == MP_IRQ_INFO_FLAGS) {
        return irq->flags;
    } else if (info_type == MP_IRQ_INFO_TRIGGERS) {
        return irq->trigger;
    }

    return 0;
}

STATIC const mp_irq_methods_t machine_timer_irq_methods = {
    .trigger = machine_timer_irq_trigger,
    .info    = machine_timer_irq_info,
};

// Initialize timer IRQ and instance tracking system
void machine_timer_irq_init(void)
{
    for (size_t i = 0; i < KD_TIMER_MAX_NUM; i++) {
        MP_STATE_PORT(machine_timer_irq_obj[i]) = NULL;
        MP_STATE_PORT(machine_timer_obj[i])     = NULL;
        #if defined(CONFIG_ENABLE_MODULE_UART_PERIODIC_TX)
        machine_timer_native_claimed[i]         = false;
        #endif
    }
    MP_STATE_PORT(machine_timer_soft_irq_obj) = NULL;
    MP_STATE_PORT(machine_timer_soft_obj)     = NULL;
}
