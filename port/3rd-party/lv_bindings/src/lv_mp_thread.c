#include "lv_mp_thread.h"

#include <pthread.h>

static pthread_mutex_t lv_mp_mutex;
static pthread_once_t lv_mp_mutex_once = PTHREAD_ONCE_INIT;

static void lv_mp_thread_init(void)
{
    pthread_mutexattr_t attr;

    pthread_mutexattr_init(&attr);
    /* LVGL callbacks may call back into the binding. */
    pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_RECURSIVE);
    pthread_mutex_init(&lv_mp_mutex, &attr);
    pthread_mutexattr_destroy(&attr);
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
