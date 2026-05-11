#ifndef FUZZ_TEST_STUB_HAL_RVV_OPS_H_
#define FUZZ_TEST_STUB_HAL_RVV_OPS_H_

#include <stddef.h>
#include <string.h>

static inline void *hal_rvv_memcpy(void *dst, const void *src, size_t n) {
    return memmove(dst, src, n);
}

static inline void *hal_rvv_memset(void *dst, int value, size_t n) {
    return memset(dst, value, n);
}

#endif
