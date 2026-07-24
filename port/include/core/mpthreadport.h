/*
 * This file is part of the MicroPython project, http://micropython.org/
 *
 * The MIT License (MIT)
 *
 * Copyright (c) 2016 Damien P. George on behalf of Pycom Ltd
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */

#ifndef MP_THREAD_PORT_H
#define MP_THREAD_PORT_H

#include <pthread.h>

#include "py/mphal.h"

typedef pthread_mutex_t mp_thread_mutex_t;

void mp_thread_init(void);
void mp_thread_set_main_ready(bool ready);
void mp_thread_shutdown_workers(void);
void mp_thread_deinit(void);
void mp_thread_gc_others(void);

// Port-global recursive lock used by MicroPython atomic sections.
void mp_thread_begin_atomic_section(void);
void mp_thread_end_atomic_section(void);

void mp_thread_set_exception_main(mp_obj_t exception);
void mp_thread_set_exception_other(mp_obj_t exception);

#endif // MP_THREAD_PORT_H
