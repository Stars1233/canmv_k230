#ifndef AIDEMO_CLEANUP_H
#define AIDEMO_CLEANUP_H

#include "py/nlr.h"

typedef void (*aidemo_cleanup_fun_t)(void *context);

typedef struct {
    nlr_jump_callback_node_t nlr_node;
    aidemo_cleanup_fun_t cleanup;
    void *context;
} aidemo_nlr_cleanup_t;

typedef struct {
    void *ptr;
} aidemo_malloc_resource_t;

typedef struct {
    void *first;
    void *second;
} aidemo_malloc_pair_t;

void aidemo_nlr_cleanup_push(aidemo_nlr_cleanup_t *cleanup,
    aidemo_cleanup_fun_t cleanup_fun, void *context);
void aidemo_nlr_cleanup_finish(aidemo_nlr_cleanup_t *cleanup);

void aidemo_free_malloc_resource(void *context);
void aidemo_free_malloc_pair(void *context);

#endif
