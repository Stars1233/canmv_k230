/* Copyright (c) 2025, Canaan Bright Sight Co., Ltd
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
#pragma once

#define GPIO_IRQ_MAX_NUM (64)
#define GPIO_MAX_NUM     (64 + 8)

typedef enum _gpio_pin_edge {
    GPIO_PE_RISING = 0,
    GPIO_PE_FALLING,
    GPIO_PE_BOTH,
    GPIO_PE_HIGH,
    GPIO_PE_LOW,
    GPIO_PE_MAX,
} gpio_pin_edge_t;

typedef enum _gpio_drive_mode {
    GPIO_DM_OUTPUT = 0,
    GPIO_DM_INPUT,
    GPIO_DM_MAX,
} gpio_drive_mode_t;

typedef enum _gpio_pin_value { GPIO_PV_LOW, GPIO_PV_HIGH } gpio_pin_value_t;

typedef void (*gpio_irq_callback)(void* args);

typedef struct _drv_gpio_inst {
    void* base;

    int pin;

    gpio_pin_value_t  curr_val;
    gpio_drive_mode_t curr_mode;
    gpio_pin_edge_t   curr_irq_mode;

    void*             irq_args;
    gpio_irq_callback irq_callback;
} drv_gpio_inst_t;

int  drv_gpio_inst_create(int pin, drv_gpio_inst_t** inst);
void drv_gpio_inst_destroy(drv_gpio_inst_t** inst);

int              drv_gpio_value_set(drv_gpio_inst_t* inst, gpio_pin_value_t val);
gpio_pin_value_t drv_gpio_value_get(drv_gpio_inst_t* inst);

int               drv_gpio_mode_set(drv_gpio_inst_t* inst, gpio_drive_mode_t mode);
gpio_drive_mode_t drv_gpio_mode_get(drv_gpio_inst_t* inst);

int drv_gpio_set_irq(drv_gpio_inst_t* inst, int enable);

int drv_gpio_register_irq(drv_gpio_inst_t* inst, gpio_pin_edge_t mode, int debounce, gpio_irq_callback callback,
                          void* userargs);
int drv_gpio_unregister_irq(drv_gpio_inst_t* inst);

static inline int drv_gpio_get_pin_id(drv_gpio_inst_t* inst) { return inst->pin; }

static inline int drv_gpio_toggle(drv_gpio_inst_t* inst)
{
    if ((-1) == inst->curr_val) {
        inst->curr_val = 0;
    }
    return drv_gpio_value_set(inst, 1 - inst->curr_val);
}

static inline int drv_gpio_enable_irq(drv_gpio_inst_t* inst) { return drv_gpio_set_irq(inst, 1); }
static inline int drv_gpio_disable_irq(drv_gpio_inst_t* inst) { return drv_gpio_set_irq(inst, 0); }
