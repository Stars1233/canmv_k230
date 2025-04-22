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
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <errno.h>
#include <fcntl.h>
#include <signal.h>
#include <sys/ioctl.h>
#include <unistd.h>

#include "drivers/drv_timer.h"

#define KD_TIMER_SIG (SIGUSR2)

#define HWTIMER_CTRL_FREQ_SET (19 * 0x100 + 0x01) /* set the count frequency */
#define HWTIMER_CTRL_STOP     (19 * 0x100 + 0x02) /* stop timer */
#define HWTIMER_CTRL_INFO_GET (19 * 0x100 + 0x03) /* get a timer feature information */
#define HWTIMER_CTRL_MODE_SET (19 * 0x100 + 0x04) /* Setting the timing mode(oneshot/period) */
#define HWTIMER_CTRL_IRQ_SET  (19 * 0x100 + 0x10)
#define HWTIMER_CTRL_FREQ_GET (19 * 0x100 + 0x11)

/* Time Value */
typedef struct _rt_hwtimerval {
    int32_t sec; /* second */
    int32_t usec; /* microsecond */
} rt_hwtimerval_t;

typedef struct {
    uint8_t enable;
    uint8_t signo;
    void*   sigval;
} hwtimer_irqcfg_t;

static const int timer_inst_type = 0;

static int timer_in_use[KD_TIMER_MAX_NUM];

static int drv_timer_open(int id)
{
    char dev_name[63];

    if (KD_TIMER_MAX_NUM <= id) {
        printf("invalid timer id %d\n", id);
        return -1;
    }

    snprintf(dev_name, sizeof(dev_name), "/dev/hwtimer%d", id);

    return open(dev_name, O_RDWR);
}

static int drv_timer_ioctl(drv_hwtimer_inst_t* inst, int cmd, void* arg)
{
    if ((NULL == inst) || (0x00 > inst->fd)) {
        printf("timer not open\n");
        return -1;
    }

    return ioctl(inst->fd, cmd, arg);
}

int drv_hwtimer_inst_create(int id, drv_hwtimer_inst_t** inst)
{
    int fd = -1;

    if (KD_TIMER_MAX_NUM <= id) {
        printf("invalid timer id %d\n", id);
        return -1;
    }

    if (0 > (fd = drv_timer_open(id))) {
        printf("open /dev/hwtimer%d failed\n", id);
        return -2;
    }

    if (timer_in_use[id]) {
        printf("timer maybe in use\n");
    }

    if (*inst) {
        free(*inst);
        *inst = NULL;
    }

    *inst = malloc(sizeof(drv_hwtimer_inst_t));
    if (NULL == *inst) {
        printf("malloc failed");
        return -1;
    }
    memset(*inst, 0x00, sizeof(drv_hwtimer_inst_t));

    (*inst)->base           = (void*)&timer_inst_type;
    (*inst)->id             = id;
    (*inst)->fd             = fd;
    (*inst)->started        = 0;
    (*inst)->curr_mode      = HWTIMER_MODE_ONESHOT;
    (*inst)->curr_period_ms = 1000; /* 1000ms */
    (*inst)->curr_freq_hz   = 12500 * 1000;

    timer_in_use[id] = 1;

    return 0;
}

void drv_hwtimer_inst_destroy(drv_hwtimer_inst_t** inst)
{
    int id;
    if (NULL == (*inst)) {
        return;
    }

    id = (*inst)->id;

    drv_hwtimer_stop(*inst);
    drv_hwtimer_unregister_irq(*inst);

    close((*inst)->fd);

    free(*inst);
    *inst = NULL;

    timer_in_use[id] = 0;

    return;
}

int drv_hwtimer_get_info(drv_hwtimer_inst_t* inst, rt_hwtimer_info_t* info)
{
    // if ((NULL == inst) || (0x00 > inst->fd)) {
    //     return -1;
    // }
    if (0x00 != drv_timer_ioctl(inst, HWTIMER_CTRL_INFO_GET, &inst->curr_timer_info)) {
        printf("get hwtimer%d info failed\n", inst->id);
        return -1;
    }

    if (info) {
        memcpy(info, &inst->curr_timer_info, sizeof(*info));
    }

    return 0;
}

int drv_hwtimer_set_mode(drv_hwtimer_inst_t* inst, rt_hwtimer_mode_t mode)
{
    rt_hwtimer_mode_t _mode = mode;

    if ((NULL == inst) || (0x00 > inst->fd)) {
        return -1;
    }

    if (inst->started) {
        printf("timer %d is started\n", inst->id);
        return -1;
    }

    if (0x00 != drv_timer_ioctl(inst, HWTIMER_CTRL_MODE_SET, &_mode)) {
        printf("set hwtimer%d mode failed.\n", inst->id);
        return -1;
    }
    inst->curr_mode = mode;

    return 0;
}

int drv_hwtimer_set_freq(drv_hwtimer_inst_t* inst, uint32_t freq)
{
    uint32_t _freq = freq;
    int32_t  max_freq, min_freq;

    if (0x00 != drv_hwtimer_get_info(inst, NULL)) {
        return -1;
    }

    if (inst->started) {
        printf("timer %d is started\n", inst->id);
        return -1;
    }

    min_freq = inst->curr_timer_info.minfreq;
    max_freq = inst->curr_timer_info.maxfreq;

    if ((_freq > max_freq) || (_freq < min_freq)) {
        printf("invalid freq %d, should be %d ~  %d\n", _freq, min_freq, max_freq);
        return -1;
    }

    if (inst->curr_freq_hz != _freq) {
        if ((0x00 != drv_timer_ioctl(inst, HWTIMER_CTRL_FREQ_SET, &_freq))) {
            printf("set freq failed\n");
            return -1;
        }

        inst->curr_freq_hz = _freq;
    }

    return 0;
}

int drv_hwtimer_get_freq(drv_hwtimer_inst_t* inst, uint32_t* freq)
{
    uint32_t _freq;

    if (0x00 != drv_timer_ioctl(inst, HWTIMER_CTRL_FREQ_GET, &_freq)) {
        printf("get timer current freq failed.\n");
        return -1;
    }
    inst->curr_freq_hz = _freq;

    if (freq) {
        *freq = _freq;
    }

    return 0;
}

int drv_hwtimer_set_period(drv_hwtimer_inst_t* inst, uint32_t period_ms)
{
    float minPeriod_ms, maxPeriod_ms;
    float freq_kHz; // Frequency in kHz for ms calculations

    if (0x00 != drv_hwtimer_get_info(inst, NULL)) {
        return -1;
    }

    if (0x00 != drv_hwtimer_get_freq(inst, NULL)) {
        return -1;
    }

    if (inst->started) {
        printf("timer %d is started\n", inst->id);
        return -1;
    }

    // Convert frequency to kHz for ms calculations (1 Hz = 0.001 kHz)
    freq_kHz = (float)inst->curr_freq_hz / 1000.0f;

    // Calculate min/max periods in ms
    minPeriod_ms = 1.0f / freq_kHz; // Smallest possible period in ms
    maxPeriod_ms = (float)inst->curr_timer_info.maxcnt / freq_kHz; // Largest possible period in ms

    if ((period_ms > (uint32_t)maxPeriod_ms) || (period_ms < (uint32_t)minPeriod_ms)) {
        printf("invalid period %d ms, should be %d ~ %d ms\n", period_ms,
               (uint32_t)(minPeriod_ms), // Round up min period
               (uint32_t)(maxPeriod_ms)); // Round down max period
        return -1;
    }

    inst->curr_period_ms = period_ms;

    return 0;
}

int drv_hwtimer_start(drv_hwtimer_inst_t* inst)
{
    rt_hwtimerval_t tv;
    uint32_t        period_ms;

    if (NULL == inst) {
        return -1;
    }
    period_ms = inst->curr_period_ms;

    tv.sec  = period_ms / 1000;
    tv.usec = (period_ms % 1000) * 1000;

    if (sizeof(tv) != write(inst->fd, &tv, sizeof(tv))) {
        printf("start timer failed\n");
        return -1;
    }
    inst->started = 1;

    return 0;
}

int drv_hwtimer_stop(drv_hwtimer_inst_t* inst)
{
    if (0x00 != drv_timer_ioctl(inst, HWTIMER_CTRL_STOP, NULL)) {
        printf("stop timer failed.\n");
        return -1;
    }
    inst->started = 0;

    return 0;
}

static void drv_timer_sig_handler(int sig, siginfo_t* si, void* uc)
{
    drv_hwtimer_inst_t* inst = si->si_ptr;

    if (KD_TIMER_SIG != sig) {
        return;
    }

    if (SI_TIMER != si->si_code) {
        return;
    }

    if (!inst || (&timer_inst_type != inst->base)) {
        return;
    }

    if (inst->irq_callback) {
        inst->irq_callback(inst->irq_args);
    }
}

int drv_hwtimer_register_irq(drv_hwtimer_inst_t* inst, timer_irq_callback callback, void* userargs)
{
    struct sigaction sa;
    hwtimer_irqcfg_t cfg;

    if (NULL == inst) {
        return -1;
    }

    if (inst->started) {
        printf("timer %d is started\n", inst->id);
        return -1;
    }

    sa.sa_flags     = SA_SIGINFO;
    sa.sa_sigaction = drv_timer_sig_handler;
    sigemptyset(&sa.sa_mask);
    if ((-1) == sigaction(KD_TIMER_SIG, &sa, NULL)) {
        printf("register sigaction failed.\n");
        return -1;
    }

    inst->irq_args     = userargs;
    inst->irq_callback = callback;

    cfg.enable = 1;
    cfg.signo  = KD_TIMER_SIG;
    cfg.sigval = inst;

    if (0x00 != drv_timer_ioctl(inst, HWTIMER_CTRL_IRQ_SET, &cfg)) {
        printf("set timer %d irq failed\n", inst->id);

        sa.sa_handler   = SIG_IGN;
        sa.sa_sigaction = NULL;
        sigemptyset(&sa.sa_mask);
        sa.sa_flags = 0;
        sigaction(KD_TIMER_SIG, &sa, NULL);

        return -1;
    }

    return 0;
}

int drv_hwtimer_unregister_irq(drv_hwtimer_inst_t* inst)
{
    struct sigaction sa;
    hwtimer_irqcfg_t cfg;

    if (NULL == inst) {
        return -1;
    }
    if (inst->started) {
        printf("timer %d is started\n", inst->id);
        return -1;
    }

    cfg.enable = 0;
    if (0x00 != drv_timer_ioctl(inst, HWTIMER_CTRL_IRQ_SET, &cfg)) {
        printf("disable timer %d irq failed\n", inst->id);
        return -1;
    }

    inst->irq_args     = NULL;
    inst->irq_callback = NULL;

    sa.sa_handler   = SIG_IGN;
    sa.sa_sigaction = NULL;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;
    sigaction(KD_TIMER_SIG, &sa, NULL);

    return 0;
}
