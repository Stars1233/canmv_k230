#ifndef AIDEMO_SIZE_H
#define AIDEMO_SIZE_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

static inline bool aidemo_checked_image_size(int width, int height, size_t channels, size_t *result)
{
    if (result == NULL || width <= 0 || height <= 0 || channels == 0) {
        return false;
    }

    size_t size = (size_t)width;
    if (size > SIZE_MAX / (size_t)height) {
        return false;
    }
    size *= (size_t)height;
    if (size > SIZE_MAX / channels) {
        return false;
    }

    *result = size * channels;
    return true;
}

#endif
