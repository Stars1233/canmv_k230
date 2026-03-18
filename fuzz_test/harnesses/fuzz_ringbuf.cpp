#include <stddef.h>
#include <stdint.h>

extern "C" {
#include "ringbuf.h"
}

extern "C" int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size) {
    ring_buf_t buf;
    ring_buf_init(&buf);

    for (size_t i = 0; i < size; ++i) {
        if ((data[i] & 1U) == 0U) {
            ring_buf_put(&buf, data[i]);
        } else {
            (void) ring_buf_get(&buf);
        }
    }

    while (!ring_buf_empty(&buf)) {
        (void) ring_buf_get(&buf);
    }

    return 0;
}
