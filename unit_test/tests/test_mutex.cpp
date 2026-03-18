#include <gtest/gtest.h>

extern "C" {
#include "mutex.h"
}

TEST(MutexTest, InitClearsState) {
    omv_mutex_t mutex = {123, 456, 789};
    mutex_init0(&mutex);

    EXPECT_EQ(mutex.tid, 0U);
    EXPECT_EQ(mutex.lock, 0U);
    EXPECT_EQ(mutex.last_tid, 0U);
}

TEST(MutexTest, UnlockOnlyClearsWhenTidMatches) {
    omv_mutex_t mutex = {MUTEX_TID_IDE, 0x55, 0xAA};

    mutex_unlock(&mutex, MUTEX_TID_OMV);
    EXPECT_EQ(mutex.tid, MUTEX_TID_IDE);
    EXPECT_EQ(mutex.lock, 0x55U);

    mutex_unlock(&mutex, MUTEX_TID_IDE);
    EXPECT_EQ(mutex.tid, 0U);
    EXPECT_EQ(mutex.lock, 0U);
    EXPECT_EQ(mutex.last_tid, 0xAAU);
}

TEST(MutexTest, TryLockBehaviorMatchesCurrentImplementation) {
    omv_mutex_t mutex;
    mutex_init0(&mutex);

    EXPECT_EQ(mutex_try_lock(&mutex, MUTEX_TID_IDE), 0);
    EXPECT_EQ(mutex.tid, 0U);

    mutex.tid = MUTEX_TID_IDE;
    mutex.lock = 1;
    EXPECT_EQ(mutex_try_lock(&mutex, MUTEX_TID_IDE), 0);
    EXPECT_EQ(mutex.tid, 0U);
    EXPECT_EQ(mutex.lock, 0U);
}

TEST(MutexTest, AlternateAndTimeoutBehaveAsExpected) {
    omv_mutex_t mutex;
    mutex_init0(&mutex);

    EXPECT_EQ(mutex_try_lock_alternate(&mutex, MUTEX_TID_IDE), 0);
    EXPECT_EQ(mutex.last_tid, 0U);

    EXPECT_EQ(mutex_lock_timeout(&mutex, MUTEX_TID_OMV, 100), 0);
    EXPECT_EQ(mutex.tid, 0U);

    mutex_lock(&mutex, MUTEX_TID_IDE);
    EXPECT_EQ(mutex.tid, 0U);
}
