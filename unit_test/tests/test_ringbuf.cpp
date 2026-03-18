#include <gtest/gtest.h>

extern "C" {
#include "ringbuf.h"
}

TEST(RingBufTest, InitAndEmptyState) {
    ring_buf_t buf;
    ring_buf_init(&buf);

    EXPECT_TRUE(ring_buf_empty(&buf));
    EXPECT_EQ(ring_buf_get(&buf), 0);
}

TEST(RingBufTest, PutGetOrdering) {
    ring_buf_t buf;
    ring_buf_init(&buf);

    ring_buf_put(&buf, 0x11);
    ring_buf_put(&buf, 0x22);
    ring_buf_put(&buf, 0x33);

    EXPECT_FALSE(ring_buf_empty(&buf));
    EXPECT_EQ(ring_buf_get(&buf), 0x11);
    EXPECT_EQ(ring_buf_get(&buf), 0x22);
    EXPECT_EQ(ring_buf_get(&buf), 0x33);
    EXPECT_TRUE(ring_buf_empty(&buf));
}

TEST(RingBufTest, OverflowDropsNewData) {
    ring_buf_t buf;
    ring_buf_init(&buf);

    for (int i = 0; i < BUFFER_SIZE - 1; ++i) {
        ring_buf_put(&buf, static_cast<uint8_t>(i));
    }

    ring_buf_put(&buf, 0xAA);

    for (int i = 0; i < BUFFER_SIZE - 1; ++i) {
        EXPECT_EQ(ring_buf_get(&buf), static_cast<uint8_t>(i));
    }

    EXPECT_EQ(ring_buf_get(&buf), 0);
    EXPECT_TRUE(ring_buf_empty(&buf));
}
