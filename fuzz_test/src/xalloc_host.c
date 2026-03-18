#include <stdlib.h>
#include <string.h>

#include "xalloc.h"

void *xalloc(uint32_t size) {
    if (size == 0) {
        return NULL;
    }

    void *mem = malloc(size);
    if (mem == NULL) {
        abort();
    }
    return mem;
}

void *xalloc_try_alloc(uint32_t size) {
    if (size == 0) {
        return NULL;
    }

    return malloc(size);
}

void *xalloc0(uint32_t size) {
    void *mem = xalloc(size);
    if (mem != NULL) {
        memset(mem, 0, size);
    }
    return mem;
}

void xfree(void *mem) {
    free(mem);
}

void *xrealloc(void *mem, uint32_t size) {
    if (size == 0) {
        free(mem);
        return NULL;
    }

    void *new_mem = realloc(mem, size);
    if (new_mem == NULL) {
        abort();
    }
    return new_mem;
}
