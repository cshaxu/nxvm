#define _POSIX_C_SOURCE 200809L

#include "lib/host/sync.h"

#include "lib/types/linux/sync.h"
#include "lib/types/linux/clock.h"

struct host_sync_event {
    lib_bool signaled;
    lib_bool manual_reset;
};
struct host_sync_platform_task {
    lib_linux_pthread_t thread; lib_bool joined;
    host_sync_platform_task_entry entry;
    void *context;
};

/* One synchronization boundary for wait-any predicates, not a polling loop
 * or one condition per event which cannot wake a multi-event waiter. */
static lib_linux_pthread_mutex_t host_sync_lock = LIB_LINUX_PTHREAD_MUTEX_INITIALIZER;
static lib_linux_pthread_cond_t host_sync_changed;
static lib_linux_pthread_once_t host_sync_once = LIB_LINUX_PTHREAD_ONCE_INIT;
static int host_sync_init_status = -1;

static void host_sync_initialize(void)
{
    lib_linux_pthread_condattr_t attributes;
    if (lib_linux_pthread_condattr_init(&attributes) != 0) return;
    if (lib_linux_pthread_condattr_setclock(&attributes, LIB_LINUX_CLOCK_MONOTONIC) == 0)
        host_sync_init_status = lib_linux_pthread_cond_init(&host_sync_changed, &attributes);
    (void)lib_linux_pthread_condattr_destroy(&attributes);
}

static lib_bool host_sync_platform_deadline(lib_u32 milliseconds,
    lib_linux_timespec *deadline)
{
    if (lib_linux_clock_gettime(LIB_LINUX_CLOCK_MONOTONIC, deadline) != 0) return LIB_FALSE;
    deadline->tv_sec += (lib_linux_time_t)(milliseconds / 1000u);
    deadline->tv_nsec += (long)(milliseconds % 1000u) * 1000000L;
    if (deadline->tv_nsec >= 1000000000L) {
        ++deadline->tv_sec; deadline->tv_nsec -= 1000000000L;
    }
    return LIB_TRUE;
}

static void *host_sync_platform_main(void *opaque)
{
    host_sync_platform_task *task = opaque;
    task->entry(task->context);
    return LIB_NULL;
}

void host_sync_platform_sleep_milliseconds(lib_u32 milliseconds)
{
    lib_linux_timespec duration = { (lib_linux_time_t)(milliseconds / 1000u),
        (long)(milliseconds % 1000u) * 1000000L };
    while (lib_linux_nanosleep(&duration, &duration) != 0 &&
        lib_linux_errno == LIB_LINUX_EINTR) { }
}
void host_sync_platform_yield(void) { (void)lib_linux_sched_yield(); }

lib_status host_sync_platform_event_create(lib_bool manual_reset,
    host_sync_event **out_event)
{
    host_sync_event *event;
    if (out_event == LIB_NULL) return LIB_STATUS_INVALID_ARGUMENT;
    *out_event = LIB_NULL;
    if (lib_linux_pthread_once(&host_sync_once, host_sync_initialize) != 0 ||
        host_sync_init_status != 0) return LIB_STATUS_IO_ERROR;
    event = lib_allocate_zero(1u, sizeof(*event));
    if (event == LIB_NULL) return LIB_STATUS_NO_MEMORY;
    event->manual_reset = manual_reset != LIB_FALSE;
    *out_event = event;
    return LIB_STATUS_OK;
}
void host_sync_platform_event_destroy(host_sync_event *event)
{
    if (event == LIB_NULL) return;
    lib_release(event);
}
void host_sync_platform_event_signal(host_sync_event *event)
{
    if (event == LIB_NULL) return;
    (void)lib_linux_pthread_mutex_lock(&host_sync_lock);
    event->signaled = LIB_TRUE;
    (void)lib_linux_pthread_cond_broadcast(&host_sync_changed);
    (void)lib_linux_pthread_mutex_unlock(&host_sync_lock);
}
void host_sync_platform_event_reset(host_sync_event *event)
{
    if (event == LIB_NULL) return;
    (void)lib_linux_pthread_mutex_lock(&host_sync_lock);
    event->signaled = LIB_FALSE;
    (void)lib_linux_pthread_mutex_unlock(&host_sync_lock);
}

lib_status host_sync_platform_event_wait_many(
    const host_sync_event *const *events, lib_u32 event_count,
    lib_u32 timeout_milliseconds, lib_bool *out_signaled,
    lib_u32 *out_event_index)
{
    lib_u32 index;
    lib_linux_timespec deadline;
    int result = 0;
    if (events == LIB_NULL || event_count == 0u || out_signaled == LIB_NULL)
        return LIB_STATUS_INVALID_ARGUMENT;
    *out_signaled = LIB_FALSE;
    for (index = 0u; index < event_count; ++index)
        if (events[index] == LIB_NULL) return LIB_STATUS_INVALID_ARGUMENT;
    if (timeout_milliseconds != 0u && timeout_milliseconds != LIB_UINT32_MAX &&
        !host_sync_platform_deadline(timeout_milliseconds, &deadline)) return LIB_STATUS_IO_ERROR;
    if (lib_linux_pthread_mutex_lock(&host_sync_lock) != 0) return LIB_STATUS_IO_ERROR;
    for (;;) {
        if (result != 0 && result != LIB_LINUX_ETIMEDOUT) break;
        for (index = 0u; index < event_count; ++index) {
            host_sync_event *event = (host_sync_event *)events[index];
            if (event->signaled != LIB_FALSE) {
                if (!event->manual_reset) event->signaled = LIB_FALSE;
                if (out_event_index != LIB_NULL) *out_event_index = index;
                *out_signaled = LIB_TRUE;
                goto done;
            }
        }
        if (timeout_milliseconds == 0u || result != 0) break;
        result = timeout_milliseconds == LIB_UINT32_MAX ?
            lib_linux_pthread_cond_wait(&host_sync_changed, &host_sync_lock) :
            lib_linux_pthread_cond_timedwait(&host_sync_changed, &host_sync_lock, &deadline);
    }
done:
    (void)lib_linux_pthread_mutex_unlock(&host_sync_lock);
    return result == 0 || result == LIB_LINUX_ETIMEDOUT ? LIB_STATUS_OK : LIB_STATUS_IO_ERROR;
}

lib_status host_sync_platform_task_create(host_sync_platform_task_entry entry,
    void *context, host_sync_platform_task **out_task)
{
    host_sync_platform_task *task;
    if (entry == LIB_NULL || out_task == LIB_NULL) return LIB_STATUS_INVALID_ARGUMENT;
    *out_task = LIB_NULL;
    task = lib_allocate_zero(1u, sizeof(*task));
    if (task == LIB_NULL) return LIB_STATUS_NO_MEMORY;
    task->entry = entry; task->context = context;
    if (lib_linux_pthread_create(&task->thread, LIB_NULL, host_sync_platform_main, task) != 0) {
        lib_release(task); return LIB_STATUS_IO_ERROR;
    }
    *out_task = task; return LIB_STATUS_OK;
}
void host_sync_platform_task_join(host_sync_platform_task *task)
{ if (task != LIB_NULL && task->joined == LIB_FALSE) { (void)lib_linux_pthread_join(task->thread, LIB_NULL); task->joined = LIB_TRUE; } }
void host_sync_platform_task_destroy(host_sync_platform_task *task)
{ if (task != LIB_NULL) { lib_release(task); } }
