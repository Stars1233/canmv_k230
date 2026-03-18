#include <gtest/gtest.h>

#include <array>
#include <cstddef>
#include <cstdint>
#include <cstring>

extern "C" {
#include "unaligned_memcpy.h"
}

TEST(UnalignedMemcpyTest, ReturnsDestinationAndCopiesAlignedBuffers) {
    uint64_t src[4] = {0x1122334455667788ULL, 0xAABBCCDDEEFF0011ULL, 0x0123456789ABCDEFULL, 0x13579BDF2468ACE0ULL};
    uint64_t dst[4] = {0};

    void *ret = unaligned_memcpy(dst, src, sizeof(src));

    EXPECT_EQ(ret, static_cast<void *>(dst));
    EXPECT_EQ(memcmp(dst, src, sizeof(src)), 0);
}

TEST(UnalignedMemcpyTest, CopiesUnalignedRanges) {
    std::array<uint8_t, 64> src{};
    std::array<uint8_t, 64> dst{};

    for (size_t i = 0; i < src.size(); ++i) {
        src[i] = static_cast<uint8_t>(i * 3U);
        dst[i] = 0;
    }

    void *ret = unaligned_memcpy(dst.data() + 1, src.data() + 1, 31);

    EXPECT_EQ(ret, static_cast<void *>(dst.data() + 1));
    EXPECT_EQ(memcmp(dst.data() + 1, src.data() + 1, 31), 0);
}

TEST(UnalignedMemcpyTest, ZeroLengthCopyIsNoOp) {
    uint8_t src[8] = {1, 2, 3, 4, 5, 6, 7, 8};
    uint8_t dst[8] = {9, 9, 9, 9, 9, 9, 9, 9};

    void *ret = unaligned_memcpy(dst, src, 0);

    EXPECT_EQ(ret, static_cast<void *>(dst));
    EXPECT_EQ(dst[0], 9);
    EXPECT_EQ(dst[7], 9);
}
