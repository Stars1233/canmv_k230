#include <gtest/gtest.h>

#include <cstdint>
#include <cstdlib>

extern "C" {
#include "array.h"
}

namespace {

int compare_tristate(const void *lhs, const void *rhs) {
    const intptr_t a = reinterpret_cast<intptr_t>(lhs);
    const intptr_t b = reinterpret_cast<intptr_t>(rhs);
    return (a > b) - (a < b);
}

int compare_greater_than(const void *lhs, const void *rhs) {
    return reinterpret_cast<intptr_t>(lhs) > reinterpret_cast<intptr_t>(rhs);
}

int g_destructor_calls = 0;

void counting_free_destructor(void *ptr) {
    ++g_destructor_calls;
    free(ptr);
}

}  // namespace

TEST(ArrayTest, PushPopAndLength) {
    array_t *array = nullptr;
    array_alloc(&array, nullptr);

    for (intptr_t i = 0; i < 8; ++i) {
        array_push_back(array, reinterpret_cast<void *>(i));
    }

    EXPECT_EQ(array_length(array), 8);
    EXPECT_EQ(reinterpret_cast<intptr_t>(array_pop_back(array)), 7);
    EXPECT_EQ(array_length(array), 7);

    array_free(array);
}

TEST(ArrayTest, TakeEraseAndDestructorBehavior) {
    g_destructor_calls = 0;

    array_t *array = nullptr;
    array_alloc(&array, counting_free_destructor);

    int *v1 = static_cast<int *>(malloc(sizeof(int)));
    int *v2 = static_cast<int *>(malloc(sizeof(int)));
    int *v3 = static_cast<int *>(malloc(sizeof(int)));
    ASSERT_NE(v1, nullptr);
    ASSERT_NE(v2, nullptr);
    ASSERT_NE(v3, nullptr);
    *v1 = 10;
    *v2 = 20;
    *v3 = 30;

    array_push_back(array, v1);
    array_push_back(array, v2);
    array_push_back(array, v3);

    void *taken = array_take(array, 1);
    EXPECT_EQ(*static_cast<int *>(taken), 20);
    free(taken);

    array_erase(array, 0);
    EXPECT_EQ(g_destructor_calls, 1);

    array_free(array);
    EXPECT_EQ(g_destructor_calls, 2);
}

TEST(ArrayTest, ResizeSortAndInsertionSort) {
    array_t *array = nullptr;
    array_alloc(&array, nullptr);

    array_push_back(array, reinterpret_cast<void *>(5));
    array_push_back(array, reinterpret_cast<void *>(1));
    array_push_back(array, reinterpret_cast<void *>(4));
    array_push_back(array, reinterpret_cast<void *>(2));

    array_sort(array, compare_tristate);
    EXPECT_EQ(reinterpret_cast<intptr_t>(array_at(array, 0)), 1);
    EXPECT_EQ(reinterpret_cast<intptr_t>(array_at(array, 1)), 2);
    EXPECT_EQ(reinterpret_cast<intptr_t>(array_at(array, 2)), 4);
    EXPECT_EQ(reinterpret_cast<intptr_t>(array_at(array, 3)), 5);

    array_push_back(array, reinterpret_cast<void *>(3));
    array_push_back(array, reinterpret_cast<void *>(0));
    array_isort(array, compare_greater_than);

    EXPECT_EQ(reinterpret_cast<intptr_t>(array_at(array, 0)), 0);
    EXPECT_EQ(reinterpret_cast<intptr_t>(array_at(array, 1)), 1);
    EXPECT_EQ(reinterpret_cast<intptr_t>(array_at(array, 2)), 2);
    EXPECT_EQ(reinterpret_cast<intptr_t>(array_at(array, 3)), 3);
    EXPECT_EQ(reinterpret_cast<intptr_t>(array_at(array, 4)), 4);
    EXPECT_EQ(reinterpret_cast<intptr_t>(array_at(array, 5)), 5);

    array_resize(array, 2);
    EXPECT_EQ(array_length(array), 2);

    array_free(array);
}
