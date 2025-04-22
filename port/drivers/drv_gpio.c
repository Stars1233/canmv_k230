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
#include <stdio.h>
#include <stdlib.h>

#include <errno.h>
#include <fcntl.h>
#include <signal.h>
#include <sys/ioctl.h>
#include <unistd.h>

#include "drivers/drv_fpioa.h"
#include "drivers/drv_gpio.h"

#define DRV_GPIO_DEV ("/dev/gpio")

/* ioctl */
#define KD_GPIO_SET_MODE     _IOW('G', 20, int)
#define KD_GPIO_GET_MODE     _IOWR('G', 21, int)
#define KD_GPIO_SET_VALUE    _IOW('G', 22, int)
#define KD_GPIO_GET_VALUE    _IOWR('G', 23, int)
#define KD_GPIO_SET_IRQ      _IOW('G', 24, int)
#define KD_GPIO_GET_IRQ      _IOWR('G', 25, int)
#define KD_GPIO_SET_IRQ_STAT _IOW('G', 26, int)

#define KD_GPIO_SIG (SIGUSR1)

typedef struct {
    uint16_t pin;
    uint16_t value;
} gpio_cfg_t;

typedef struct {
    uint16_t pin;

#define KD_GPIO_IRQ_DISABLE 0x00
#define KD_GPIO_IRQ_ENABLE  0x01
    uint8_t enable;

    // @gpio_pin_edge_t
    uint8_t  mode;
    uint16_t debounce;

    uint8_t signo;
    void*   sigval;
} gpio_irqcfg_t;

static int gpio_fd      = -1;
static int gpio_ref_cnt = 0;

static const int gpio_inst_type = 0;

static int drv_gpio_open(void)
{
    if (0x00 > gpio_fd) {
        gpio_fd = open(DRV_GPIO_DEV, O_RDWR);
        if (0x00 > gpio_fd) {
            printf("open gpio device failed.\n");
            return -1;
        }
    }

    gpio_ref_cnt++;

    return 0;
}

static void drv_gpio_close(void)
{
    if (0x00 > gpio_fd) {
        if (0x00 == (--gpio_ref_cnt)) {
            close(gpio_fd);
            gpio_fd = -1;
        }
    }
}

static inline int drv_gpio_ioctl(int cmd, void* arg)
{
    if (0x00 > gpio_fd) {
        printf("gpio not open\n");
        return -1;
    }

    if (0x00 != ioctl(gpio_fd, cmd, arg)) {
        return -1;
    }

    return 0;
}

int drv_gpio_inst_create(int pin, drv_gpio_inst_t** inst)
{
    if (GPIO_MAX_NUM <= pin) {
        printf("invalid pin %d\n", pin);
        return -1;
    }

    if (0x00 != drv_gpio_open()) {
        return -1;
    }

    if (*inst) {
        free(*inst);
        *inst = NULL;
    }

    *inst = malloc(sizeof(drv_gpio_inst_t));
    if (NULL == *inst) {
        printf("malloc failed");
        drv_gpio_close();
        return -1;
    }

    (*inst)->base          = (void*)&gpio_inst_type;
    (*inst)->pin           = pin;
    (*inst)->curr_val      = -1;
    (*inst)->curr_mode     = GPIO_DM_MAX;
    (*inst)->curr_irq_mode = GPIO_PE_MAX;
    (*inst)->irq_args      = NULL;
    (*inst)->irq_callback  = NULL;

    return 0;
}

void drv_gpio_inst_destroy(drv_gpio_inst_t** inst)
{
    if (!*inst) {
        return;
    }

    drv_gpio_mode_set(*inst, GPIO_DM_INPUT);
    drv_gpio_close();

    free(*inst);
    *inst = NULL;
}

int drv_gpio_value_set(drv_gpio_inst_t* inst, gpio_pin_value_t val)
{
    if (NULL == inst) {
        return -1;
    }

    gpio_cfg_t cfg = { .pin = inst->pin, val };

    if (inst->curr_val == val) {
        return 0;
    }
    inst->curr_val = val;

    return drv_gpio_ioctl(KD_GPIO_SET_VALUE, &cfg);
}

gpio_pin_value_t drv_gpio_value_get(drv_gpio_inst_t* inst)
{
    if (NULL == inst) {
        return GPIO_PV_LOW;
    }

    int        ret;
    gpio_cfg_t cfg = { .pin = inst->pin };

    if (0x00 != (ret = drv_gpio_ioctl(KD_GPIO_GET_VALUE, &cfg))) {
        printf("read pin %d valued failed %d\n", cfg.pin, ret);
        return GPIO_PV_LOW;
    }
    inst->curr_val = cfg.value;

    return cfg.value;
}

int drv_gpio_mode_set(drv_gpio_inst_t* inst, gpio_drive_mode_t mode)
{
    if (NULL == inst) {
        return -1;
    }

    fpioa_iomux_cfg_t iomux, new_iomux;

    gpio_cfg_t cfg = { .pin = inst->pin, mode };

    if (mode == inst->curr_mode) {
        return 0;
    }
    inst->curr_mode = mode;

    if (0x00 != drv_fpioa_get_pin_cfg(inst->pin, &iomux.u.value)) {
        printf("get pin iomux failed\n");
        return -1;
    }
    new_iomux.u.value = iomux.u.value;

    switch (mode) {
    case GPIO_DM_OUTPUT:
        cfg.value = GPIO_DM_OUTPUT;

        new_iomux.u.bit.oe = 1;
        break;
    case GPIO_DM_INPUT:
        cfg.value = GPIO_DM_INPUT;

        new_iomux.u.bit.ie = 1;
        break;

    default:
        printf("invalid mode\n");
        return -1;
        break;
    }

    if (new_iomux.u.value != iomux.u.value) {
        if (0x00 != drv_fpioa_set_pin_cfg(inst->pin, new_iomux.u.value)) {
            printf("set pin iomux failed\n");
            return -1;
        }
    }

    return drv_gpio_ioctl(KD_GPIO_SET_MODE, &cfg);
}

gpio_drive_mode_t drv_gpio_mode_get(drv_gpio_inst_t* inst)
{
    if (NULL == inst) {
        return GPIO_DM_MAX;
    }

    int        ret;
    gpio_cfg_t cfg = { .pin = inst->pin };

    if (0x00 != (ret = drv_gpio_ioctl(KD_GPIO_GET_MODE, &cfg))) {
        printf("read pin %d mode failed %d\n", cfg.pin, ret);
        return GPIO_DM_MAX;
    }
    inst->curr_mode = cfg.value;

    return cfg.value;
}

int drv_gpio_set_irq(drv_gpio_inst_t* inst, int enable)
{
    if (NULL == inst) {
        return -1;
    }
    gpio_cfg_t cfg = { .pin = inst->pin, enable };

    return drv_gpio_ioctl(KD_GPIO_SET_IRQ_STAT, &cfg);
}

static void drv_gpio_sig_handler(int sig, siginfo_t* si, void* uc)
{
    drv_gpio_inst_t* inst = si->si_ptr;

    if (KD_GPIO_SIG != sig) {
        return;
    }

    if (SI_SIGIO != si->si_code) {
        return;
    }

    if (!inst || (&gpio_inst_type != inst->base)) {
        return;
    }

    if (inst->irq_callback) {
        inst->irq_callback(inst->irq_args);
    }
}

int drv_gpio_register_irq(drv_gpio_inst_t* inst, gpio_pin_edge_t mode, int debounce, gpio_irq_callback callback,
                          void* userargs)
{
    int              ret;
    struct sigaction sa;

    if (NULL == inst) {
        return -1;
    }

    gpio_irqcfg_t cfg = { .pin = inst->pin, .enable = 1, .mode = mode, .debounce = 10 };

    if (GPIO_IRQ_MAX_NUM <= inst->pin) {
        printf("pin irq only support 0~63, not %d\n", inst->pin);
        return -1;
    }

    if (10 > debounce) {
        debounce = 10;
    }
    cfg.debounce = debounce;

    if (GPIO_PE_MAX != inst->curr_irq_mode) {
        if (0x00 != drv_gpio_unregister_irq(inst)) {
            return -1;
        }
    }

    inst->curr_irq_mode = mode;
    inst->irq_args      = userargs;
    inst->irq_callback  = callback;

    sa.sa_flags     = SA_SIGINFO;
    sa.sa_sigaction = drv_gpio_sig_handler;
    sigemptyset(&sa.sa_mask);
    if ((-1) == sigaction(KD_GPIO_SIG, &sa, NULL)) {
        printf("register sigaction failed.\n");
        return -1;
    }

    cfg.signo  = KD_GPIO_SIG;
    cfg.sigval = inst;
    if (0x00 != (ret = drv_gpio_ioctl(KD_GPIO_SET_IRQ, &cfg))) {
        printf("set pin %d irq failed %d\n", cfg.pin, ret);

        sa.sa_handler   = SIG_IGN;
        sa.sa_sigaction = NULL;
        sigemptyset(&sa.sa_mask);
        sa.sa_flags = 0;
        sigaction(KD_GPIO_SIG, &sa, NULL);

        return -1;
    }

    return 0;
}

int drv_gpio_unregister_irq(drv_gpio_inst_t* inst)
{
    if (NULL == inst) {
        return -1;
    }

    int              ret;
    struct sigaction sa;
    gpio_irqcfg_t    cfg = { .pin = inst->pin, .enable = 0 };

    if ((GPIO_PE_MAX == inst->curr_irq_mode) && (NULL == inst->irq_callback)) {
        return 0;
    }

    if (0x00 != (ret = drv_gpio_ioctl(KD_GPIO_SET_IRQ, &cfg))) {
        printf("disable pin %d irq failed %d\n", cfg.pin, ret);
        return -1;
    }

    inst->curr_irq_mode = GPIO_PE_MAX;
    inst->irq_args      = NULL;
    inst->irq_callback  = NULL;

    sa.sa_handler   = SIG_IGN;
    sa.sa_sigaction = NULL;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;
    sigaction(KD_GPIO_SIG, &sa, NULL);

    return 0;
}
