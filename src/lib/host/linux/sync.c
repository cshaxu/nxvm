#define _POSIX_C_SOURCE 200809L

#include "lib/base/base.h"
#include "lib/host/sync.h"

#include <sched.h>
#include <errno.h>
#include <pthread.h>
#include <time.h>

struct host_sync_event {
    pthread_mutex_t mutex;
    pthread_cond_t condition;
    lib_bool signaled;
};

struct host_sync_task {
    pthread_t thread;
    host_sync_event cancellation;
    host_sync_task_entry entry;
    void *context;
    lib_bool joined;
};

static void *host_sync_task_main(void *opaque)
{
    host_sync_task *task = opaque;

    if (task != LIB_NULL && task->entry != LIB_NULL) {
        task->entry(task->context, task);
    }
    return LIB_NULL;
}

static int host_sync_deadline(lib_u32 milliseconds, struct timespec *deadline)
{
    if (clock_gettime(CLOCK_REALTIME, deadline) != 0) return LIB_FALSE;
    deadline->tv_sec += (time_t)(milliseconds / 1000u);
    deadline->tv_nsec += (long)(milliseconds % 1000u) * 1000000L;
    if (deadline->tv_nsec >= 1000000000L) {
        ++deadline->tv_sec;
        deadline->tv_nsec -= 1000000000L;
    }
    return LIB_TRUE;
}

static lib_status host_sync_event_initialize(host_sync_event *event)
{
    if (event == LIB_NULL || pthread_mutex_init(&event->mutex, LIB_NULL) != 0) {
        return LIB_STATUS_IO_ERROR;
    }
    if (pthread_cond_init(&event->condition, LIB_NULL) != 0) {
        (void)pthread_mutex_destroy(&event->mutex);
        return LIB_STATUS_IO_ERROR;
    }
    event->signaled = LIB_FALSE;
    return LIB_STATUS_OK;
}

static void host_sync_event_finalize(host_sync_event *event)
{
    if (event == LIB_NULL) return;
    (void)pthread_cond_destroy(&event->condition);
    (void)pthread_mutex_destroy(&event->mutex);
}

void host_sync_sleep_milliseconds(lib_u32 milliseconds)
{
    struct timespec duration = {
        (time_t)(milliseconds / 1000u),
        (long)(milliseconds % 1000u) * 1000000L
    };

    (void)nanosleep(&duration, LIB_NULL);
}

void host_sync_yield(void)
{
    sched_yield();
}

lib_status host_sync_event_create(host_sync_event **out_event)
{
    host_sync_event *event;

    if (out_event == LIB_NULL) return LIB_STATUS_INVALID_ARGUMENT;
    *out_event = LIB_NULL;
    event = calloc(1u, sizeof(*event));
    if (event == LIB_NULL) return LIB_STATUS_NO_MEMORY;
    if (host_sync_event_initialize(event) != LIB_STATUS_OK) {
        free(event);
        return LIB_STATUS_IO_ERROR;
    }
    *out_event = event;
    return LIB_STATUS_OK;
}

void host_sync_event_destroy(host_sync_event *event)
{
    if (event == LIB_NULL) return;
    host_sync_event_finalize(event);
    free(event);
}

void host_sync_event_signal(host_sync_event *event)
{
    if (event == LIB_NULL) return;
    (void)pthread_mutex_lock(&event->mutex);
    event->signaled = LIB_TRUE;
    (void)pthread_cond_broadcast(&event->condition);
    (void)pthread_mutex_unlock(&event->mutex);
}

void host_sync_event_reset(host_sync_event *event)
{
    if (event == LIB_NULL) return;
    (void)pthread_mutex_lock(&event->mutex);
    event->signaled = LIB_FALSE;
    (void)pthread_mutex_unlock(&event->mutex);
}

host_sync_wait_result host_sync_event_wait(host_sync_event *event,
    lib_u32 timeout_milliseconds)
{
    struct timespec deadline;
    int result = 0;

    if (event == LIB_NULL) return HOST_SYNC_WAIT_INVALID_ARGUMENT;
    if (!host_sync_deadline(timeout_milliseconds, &deadline)) return HOST_SYNC_WAIT_FAULT;
    (void)pthread_mutex_lock(&event->mutex);
    while (!event->signaled && result == 0) {
        result = pthread_cond_timedwait(&event->condition, &event->mutex, &deadline);
    }
    (void)pthread_mutex_unlock(&event->mutex);
    return result == 0 ? HOST_SYNC_WAIT_SIGNALED : result == ETIMEDOUT ?
        HOST_SYNC_WAIT_TIMED_OUT : HOST_SYNC_WAIT_FAULT;
}

lib_status host_sync_task_create(host_sync_task_entry entry, void *context,
    host_sync_task **out_task)
{
    host_sync_task *task;

    if (entry == LIB_NULL || out_task == LIB_NULL) return LIB_STATUS_INVALID_ARGUMENT;
    *out_task = LIB_NULL;
    task = calloc(1u, sizeof(*task));
    if (task == LIB_NULL) return LIB_STATUS_NO_MEMORY;
    task->entry = entry;
    task->context = context;
    if (host_sync_event_initialize(&task->cancellation) != LIB_STATUS_OK) {
        free(task);
        return LIB_STATUS_IO_ERROR;
    }
    if (pthread_create(&task->thread, LIB_NULL, host_sync_task_main, task) != 0) {
        host_sync_event_finalize(&task->cancellation);
        free(task);
        return LIB_STATUS_IO_ERROR;
    }
    *out_task = task;
    return LIB_STATUS_OK;
}

void host_sync_task_request_cancel(host_sync_task *task)
{
    if (task != LIB_NULL) host_sync_event_signal(&task->cancellation);
}

int host_sync_task_cancelled(const host_sync_task *task)
{
    lib_bool cancelled;

    if (task == LIB_NULL) return LIB_FALSE;
    (void)pthread_mutex_lock((pthread_mutex_t *)&task->cancellation.mutex);
    cancelled = task->cancellation.signaled;
    (void)pthread_mutex_unlock((pthread_mutex_t *)&task->cancellation.mutex);
    return cancelled;
}

host_sync_wait_result host_sync_task_wait_cancel(const host_sync_task *task,
    lib_u32 timeout_milliseconds)
{
    host_sync_wait_result result;

    if (task == LIB_NULL) return HOST_SYNC_WAIT_INVALID_ARGUMENT;
    result = host_sync_event_wait((host_sync_event *)&task->cancellation,
        timeout_milliseconds);
    return result == HOST_SYNC_WAIT_SIGNALED ? HOST_SYNC_WAIT_CANCELLED : result;
}

void host_sync_task_join(host_sync_task *task)
{
    if (task != LIB_NULL && !task->joined) {
        (void)pthread_join(task->thread, LIB_NULL);
        task->joined = LIB_TRUE;
    }
}

void host_sync_task_destroy(host_sync_task *task)
{
    if (task == LIB_NULL) return;
    host_sync_task_join(task);
    host_sync_event_finalize(&task->cancellation);
    free(task);
}
