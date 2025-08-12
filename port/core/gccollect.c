/*
 * This file is part of the MicroPython project, http://micropython.org/
 *
 * The MIT License (MIT)
 *
 * Copyright (c) 2013, 2014 Damien P. George
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

#include <stdio.h>

#include "py/gc.h"
#include "py/mpstate.h"

#if MICROPY_ENABLE_GC

static inline __attribute__((always_inline)) void* get_sp(void)
{
    void* sp;
    asm volatile("mv %0, sp" : "=r"(sp));
    return sp;
}

// The level argument must be volatile to force the compiler to emit code that
// will call this function recursively, to nest the C stack.
static void gc_collect_inner(volatile unsigned int level)
{
    if (level < 8) {
        // Go deeper on the stack to spill more registers from the register window.
        gc_collect_inner(level + 1);
    } else {
        // Deep enough so that all registers are on the C stack, now trace the stack.
        volatile void* sp = get_sp();
        gc_collect_root((void**)sp, ((mp_uint_t)MP_STATE_THREAD(stack_top) - (mp_uint_t)sp) / sizeof(void*));
    }
}

void gc_collect(void)
{
    gc_collect_start();
    gc_collect_inner(0);
#if MICROPY_PY_THREAD
    mp_thread_gc_others();
#endif
    gc_collect_end();
}

#endif // MICROPY_ENABLE_GC

#if MICROPY_GC_SPLIT_HEAP_AUTO

// The largest new region that is available to become Python heap is the largest free block
size_t gc_get_max_new_split(void) {
    return 4 * 1024 * 1024; // 4 MiB
}

#endif
