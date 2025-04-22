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

#include <errno.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <unistd.h>

#include "drivers/drv_fpioa.h"

#define IOMUX_REG_ADD 0X91105000

static volatile uint32_t* fpioa_reg = NULL;

static volatile uint32_t* check_fpioa(void)
{
    if (NULL == fpioa_reg) {
        int mem_fd = -1;
        if (0 > mem_fd) {
            if (0 > (mem_fd = open("/dev/mem", O_RDWR | O_SYNC))) {
                printf("open /dev/mem failed\n");
                return NULL;
            }
        }

        fpioa_reg = (uint32_t*)mmap(NULL, 4096, PROT_READ | PROT_WRITE, MAP_SHARED, mem_fd, IOMUX_REG_ADD);

        close(mem_fd);
        mem_fd = -1;

        if (fpioa_reg == NULL) {
            printf("mmap fpioa failed\n");
            return NULL;
        }
    }

    return fpioa_reg;
}

int drv_fpioa_get_pin_cfg(int pin, uint32_t* value)
{
    if (NULL == check_fpioa()) {
        return -1;
    }
    *value = *(fpioa_reg + pin);

    return 0;
}

int drv_fpioa_set_pin_cfg(int pin, uint32_t value)
{
    if (NULL == check_fpioa()) {
        return -1;
    }
    *(fpioa_reg + pin) = (*(fpioa_reg + pin) & 0x200) | value;

    return 0;
}
