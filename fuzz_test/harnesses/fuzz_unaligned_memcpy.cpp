#include <stddef.h>
#include <stdint.h>

extern "C" {
#include "unaligned_memcpy.h"
}

namespace {

size_t max_size(size_t lhs, size_t rhs) {
    return lhs > rhs ? lhs : rhs;
}

}  // namespace

extern "C" int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size) {
    constexpr size_t kWorkingSet = 256;
    alignas(8) uint8_t src[kWorkingSet + 16] = {0};
    alignas(8) uint8_t dst[kWorkingSet + 16] = {0};

    for (size_t i = 0; i < sizeof(src); ++i) {
        src[i] = size == 0 ? 0U : data[i % size];
    }

    if (size < 3) {
        (void) unaligned_memcpy(dst, src, size);
        return 0;
    }

    for (size_t i = 0; i + 2 < size; i += 3) {
        const size_t src_offset = data[i] % 16U;
        const size_t dst_offset = data[i + 1] % 16U;
        const size_t max_len = kWorkingSet - max_size(src_offset, dst_offset);
        const size_t len = max_len == 0 ? 0U : data[i + 2] % (max_len + 1U);

        (void) unaligned_memcpy(dst + dst_offset, src + src_offset, len);
        src[(i / 3) % kWorkingSet] ^= dst[(i / 3) % kWorkingSet];
    }

    return 0;
}
