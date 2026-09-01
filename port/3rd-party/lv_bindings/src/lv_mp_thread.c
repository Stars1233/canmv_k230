#include "lv_mp_thread.h"

#include <pthread.h>
#include <stdint.h>

#include "py/mpthread.h"

static pthread_mutex_t lv_mp_mutex;
static pthread_once_t lv_mp_mutex_once = PTHREAD_ONCE_INIT;
static pthread_key_t lv_mp_python_call_key;

static void lv_mp_thread_init(void)
{
    pthread_mutexattr_t attr;

    pthread_mutexattr_init(&attr);
    /* LVGL callbacks may call back into the binding. */
    pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_RECURSIVE);
    pthread_mutex_init(&lv_mp_mutex, &attr);
    pthread_mutexattr_destroy(&attr);
    pthread_key_create(&lv_mp_python_call_key, NULL);
}

void lv_mp_thread_lock(void)
{
    pthread_once(&lv_mp_mutex_once, lv_mp_thread_init);
    pthread_mutex_lock(&lv_mp_mutex);
}

void lv_mp_thread_unlock(void)
{
    pthread_mutex_unlock(&lv_mp_mutex);
}

void lv_mp_thread_python_call_enter(void)
{
    pthread_once(&lv_mp_mutex_once, lv_mp_thread_init);
    uintptr_t depth = (uintptr_t)pthread_getspecific(lv_mp_python_call_key);
    pthread_setspecific(lv_mp_python_call_key, (void *)(depth + 1));
}

void lv_mp_thread_python_call_exit(void)
{
    pthread_once(&lv_mp_mutex_once, lv_mp_thread_init);
    uintptr_t depth = (uintptr_t)pthread_getspecific(lv_mp_python_call_key);
    if (depth > 0) {
        pthread_setspecific(lv_mp_python_call_key, (void *)(depth - 1));
    }
}

int lv_mp_thread_python_call_active(void)
{
    pthread_once(&lv_mp_mutex_once, lv_mp_thread_init);
    return pthread_getspecific(lv_mp_python_call_key) != NULL;
}

void lv_mp_thread_gil_yield(void)
{
    /* Only the binding entry paths hold the GIL, and they are the only ones
     * that raise the Python-call depth. A native caller must not touch it. */
    if (!lv_mp_thread_python_call_active()) {
        return;
    }

    MP_THREAD_GIL_EXIT();
    MP_THREAD_GIL_ENTER();
}
