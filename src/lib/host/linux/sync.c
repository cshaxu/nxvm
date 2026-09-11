#define _POSIX_C_SOURCE 200809L

#include "lib/types/types_interface.h"
#include "lib/host/sync_interface.h"

#include <errno.h>
#include <pthread.h>
#include <sched.h>
#include <stdlib.h>
#include <time.h>

struct host_sync_event {
    pthread_mutex_t mutex;
    lib_bool signaled;
};

struct host_sync_task {
    pthread_t thread;
    host_sync_event cancellation;
    host_sync_task_entry entry;
    void *context;
    lib_bool joined;
};

typedef struct host_sync_wait_hub {
    pthread_mutex_t mutex;
    pthread_cond_t condition;
    lib_u64 generation;
    lib_status status;
} host_sync_wait_hub;

static host_sync_wait_hub host_sync_hub;
static pthread_once_t host_sync_hub_once = PTHREAD_ONCE_INIT;

static void host_sync_hub_initialize(void)
{
    pthread_condattr_t attributes;

    host_sync_hub.status = LIB_STATUS_IO_ERROR;
    if (pthread_mutex_init(&host_sync_hub.mutex, LIB_NULL) != 0 ||
        pthread_condattr_init(&attributes) != 0) return;
    if (pthread_condattr_setclock(&attributes, CLOCK_MONOTONIC) != 0 ||
        pthread_cond_init(&host_sync_hub.condition, &attributes) != 0) {
        (void)pthread_condattr_destroy(&attributes);
        return;
    }
    (void)pthread_condattr_destroy(&attributes);
    host_sync_hub.status = LIB_STATUS_OK;
}

static lib_status host_sync_hub_ready(void)
{
    (void)pthread_once(&host_sync_hub_once, host_sync_hub_initialize);
    return host_sync_hub.status;
}

static void host_sync_hub_notify(void)
{
    if (host_sync_hub_ready() != LIB_STATUS_OK) return;
    (void)pthread_mutex_lock(&host_sync_hub.mutex);
    ++host_sync_hub.generation;
    (void)pthread_cond_broadcast(&host_sync_hub.condition);
    (void)pthread_mutex_unlock(&host_sync_hub.mutex);
}

static lib_bool host_sync_deadline(lib_u32 milliseconds, struct timespec *deadline)
{
    if (clock_gettime(CLOCK_MONOTONIC, deadline) != 0) return LIB_FALSE;
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
    if (event == LIB_NULL || pthread_mutex_init(&event->mutex, LIB_NULL) != 0)
        return LIB_STATUS_IO_ERROR;
    event->signaled = LIB_FALSE;
    return LIB_STATUS_OK;
}

static void host_sync_event_finalize(host_sync_event *event)
{
    if (event != LIB_NULL) (void)pthread_mutex_destroy(&event->mutex);
}

static lib_bool host_sync_event_is_signaled(const host_sync_event *event)
{
    lib_bool signaled;

    if (event == LIB_NULL) return LIB_FALSE;
    (void)pthread_mutex_lock((pthread_mutex_t *)&event->mutex);
    signaled = event->signaled;
    (void)pthread_mutex_unlock((pthread_mutex_t *)&event->mutex);
    return signaled;
}

static void *host_sync_task_main(void *opaque)
{
    host_sync_task *task = opaque;

    if (task != LIB_NULL && task->entry != LIB_NULL)
        task->entry(task->context, task);
    return LIB_NULL;
}

void host_sync_sleep_milliseconds(lib_u32 milliseconds)
{
    struct timespec duration = { (time_t)(milliseconds / 1000u),
        (long)(milliseconds % 1000u) * 1000000L };

    (void)nanosleep(&duration, LIB_NULL);
}

void host_sync_yield(void) { sched_yield(); }

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
    (void)pthread_mutex_unlock(&event->mutex);
    host_sync_hub_notify();
}

void host_sync_event_reset(host_sync_event *event)
{
    if (event == LIB_NULL) return;
    (void)pthread_mutex_lock(&event->mutex);
    event->signaled = LIB_FALSE;
    (void)pthread_mutex_unlock(&event->mutex);
    host_sync_hub_notify();
}

host_sync_wait_result host_sync_wait_any(host_sync_event *const *events,
    lib_u32 event_count, const host_sync_task *cancel_task,
    lib_u32 timeout_milliseconds, lib_u32 *out_event_index)
{
    struct timespec deadline;
    lib_u32 index;

    if ((event_count != 0u && events == LIB_NULL) ||
        (event_count == 0u && cancel_task == LIB_NULL))
        return HOST_SYNC_WAIT_INVALID_ARGUMENT;
    for (index = 0u; index < event_count; ++index) {
        if (events[index] == LIB_NULL) return HOST_SYNC_WAIT_INVALID_ARGUMENT;
    }
    if (out_event_index != LIB_NULL) *out_event_index = UINT32_MAX;
    if (!host_sync_deadline(timeout_milliseconds, &deadline) ||
        host_sync_hub_ready() != LIB_STATUS_OK) return HOST_SYNC_WAIT_FAULT;
    for (;;) {
        lib_u64 observed;
        int result;

        (void)pthread_mutex_lock(&host_sync_hub.mutex);
        observed = host_sync_hub.generation;
        (void)pthread_mutex_unlock(&host_sync_hub.mutex);
        if (cancel_task != LIB_NULL && host_sync_task_cancelled(cancel_task))
            return HOST_SYNC_WAIT_CANCELLED;
        for (index = 0u; index < event_count; ++index) {
            if (host_sync_event_is_signaled(events[index])) {
                if (out_event_index != LIB_NULL) *out_event_index = index;
                return HOST_SYNC_WAIT_SIGNALED;
            }
        }
        if (timeout_milliseconds == 0u) return HOST_SYNC_WAIT_TIMED_OUT;
        (void)pthread_mutex_lock(&host_sync_hub.mutex);
        if (host_sync_hub.generation != observed) {
            (void)pthread_mutex_unlock(&host_sync_hub.mutex);
            continue;
        }
        result = pthread_cond_timedwait(&host_sync_hub.condition,
            &host_sync_hub.mutex, &deadline);
        (void)pthread_mutex_unlock(&host_sync_hub.mutex);
        if (result == ETIMEDOUT) return HOST_SYNC_WAIT_TIMED_OUT;
        if (result != 0) return HOST_SYNC_WAIT_FAULT;
    }
}

host_sync_wait_result host_sync_event_wait(host_sync_event *event,
    lib_u32 timeout_milliseconds)
{
    return host_sync_wait_any(&event, 1u, LIB_NULL, timeout_milliseconds,
        LIB_NULL);
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
    return task != LIB_NULL && host_sync_event_is_signaled(&task->cancellation);
}

host_sync_wait_result host_sync_task_wait_cancel(const host_sync_task *task,
    lib_u32 timeout_milliseconds)
{
    if (task == LIB_NULL) return HOST_SYNC_WAIT_INVALID_ARGUMENT;
    return host_sync_wait_any(LIB_NULL, 0u, task, timeout_milliseconds,
        LIB_NULL);
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
    host_sync_task_request_cancel(task);
    host_sync_task_join(task);
    host_sync_event_finalize(&task->cancellation);
    free(task);
}
