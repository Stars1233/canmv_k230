#include <stdlib.h>

#include "aidemo_cleanup.h"

static void aidemo_run_nlr_cleanup(void *context)
{
    aidemo_nlr_cleanup_t *cleanup = context;
    cleanup->cleanup(cleanup->context);
}

void aidemo_nlr_cleanup_push(aidemo_nlr_cleanup_t *cleanup,
    aidemo_cleanup_fun_t cleanup_fun, void *context)
{
    cleanup->cleanup = cleanup_fun;
    cleanup->context = context;
    nlr_push_jump_callback(&cleanup->nlr_node, aidemo_run_nlr_cleanup);
}

void aidemo_nlr_cleanup_finish(aidemo_nlr_cleanup_t *cleanup)
{
    nlr_pop_jump_callback(false);
    cleanup->cleanup(cleanup->context);
}

void aidemo_free_malloc_resource(void *context)
{
    aidemo_malloc_resource_t *resource = context;
    free(resource->ptr);
    resource->ptr = NULL;
}

void aidemo_free_malloc_pair(void *context)
{
    aidemo_malloc_pair_t *resources = context;
    free(resources->second);
    resources->second = NULL;
    free(resources->first);
    resources->first = NULL;
}
