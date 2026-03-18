#include <stddef.h>
#include <stdint.h>

extern "C" {
#include "array.h"
}

namespace {

int compare_numeric(const void *lhs, const void *rhs) {
    const uintptr_t a = (uintptr_t) lhs;
    const uintptr_t b = (uintptr_t) rhs;
    return (a > b) - (a < b);
}

int compare_isort(const void *lhs, const void *rhs) {
    return (uintptr_t) lhs > (uintptr_t) rhs;
}

size_t valid_index(const array_t *array, uint8_t raw) {
    const int length = array_length(const_cast<array_t *>(array));
    return length > 0 ? static_cast<size_t>(raw % length) : 0U;
}

}  // namespace

extern "C" int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size) {
    array_t *array = nullptr;
    array_alloc(&array, nullptr);

    for (size_t i = 0; i < size; ++i) {
        const uint8_t op = data[i] % 8;
        switch (op) {
            case 0:
                array_push_back(array, (void *) ((uintptr_t) data[i]));
                break;
            case 1:
                (void) array_pop_back(array);
                break;
            case 2:
                if (array_length(array) > 0) {
                    (void) array_take(array, valid_index(array, data[i]));
                }
                break;
            case 3:
                if (array_length(array) > 0) {
                    array_erase(array, valid_index(array, data[i]));
                }
                break;
            case 4:
                array_resize(array, static_cast<int>(data[i] % 64));
                break;
            case 5:
                array_sort(array, compare_numeric);
                break;
            case 6:
                array_isort(array, compare_isort);
                break;
            case 7:
                if (array_length(array) > 0) {
                    (void) array_at(array, valid_index(array, data[i]));
                }
                break;
        }
    }

    array_free(array);
    return 0;
}
