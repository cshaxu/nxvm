#define _POSIX_C_SOURCE 200809L

#include "lib/base/sync.h"

#include "lib/types/linux/sync.h"
#include "lib/types/linux/clock.h"

struct base_sync_mutex { lib_linux_pthread_mutex_t gate; };

lib_status base_sync_mutex_create(base_sync_mutex **out_mutex)
{
    base_sync_mutex *mutex;
    if (out_mutex == LIB_NULL) return LIB_STATUS_INVALID_ARGUMENT;
    *out_mutex = LIB_NULL;
    mutex = lib_allocate(sizeof(*mutex));
    if (mutex == LIB_NULL) return LIB_STATUS_NO_MEMORY;
    if (lib_linux_pthread_mutex_init(&mutex->gate, LIB_NULL) != 0) {
        lib_release(mutex);
        return LIB_STATUS_IO_ERROR;
    }
    *out_mutex = mutex;
    return LIB_STATUS_OK;
}

void base_sync_mutex_destroy(base_sync_mutex *mutex)
{
    if (mutex == LIB_NULL) return;
    (void)lib_linux_pthread_mutex_destroy(&mutex->gate);
    lib_release(mutex);
}

void base_sync_mutex_lock(base_sync_mutex *mutex)
{ (void)lib_linux_pthread_mutex_lock(&mutex->gate); }
void base_sync_mutex_unlock(base_sync_mutex *mutex)
{ (void)lib_linux_pthread_mutex_unlock(&mutex->gate); }

struct base_sync_event {
    lib_bool signaled;
    lib_bool manual_reset;
};
struct base_sync_linux_task {
    base_sync_task task;
    lib_linux_pthread_t thread; lib_bool joined;
};

/* One synchronization boundary for wait-any predicates, not a polling loop
 * or one condition per event which cannot wake a multi-event waiter. */
static lib_linux_pthread_mutex_t base_sync_lock = LIB_LINUX_PTHREAD_MUTEX_INITIALIZER;
static lib_linux_pthread_cond_t base_sync_changed;
static lib_linux_pthread_once_t base_sync_once = LIB_LINUX_PTHREAD_ONCE_INIT;
static int base_sync_init_status = -1;

static void base_sync_initialize(void)
{
    lib_linux_pthread_condattr_t attributes;
    if (lib_linux_pthread_condattr_init(&attributes) != 0) return;
    if (lib_linux_pthread_condattr_setclock(&attributes, LIB_LINUX_CLOCK_MONOTONIC) == 0)
        base_sync_init_status = lib_linux_pthread_cond_init(&base_sync_changed, &attributes);
    (void)lib_linux_pthread_condattr_destroy(&attributes);
}

static lib_bool base_sync_platform_deadline(lib_u32 milliseconds,
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

static void *base_sync_platform_main(void *opaque)
{
    struct base_sync_linux_task *state = opaque;
    state->task.entry(state->task.context, &state->task);
    return LIB_NULL;
}

void base_sync_platform_sleep_milliseconds(lib_u32 milliseconds)
{
    lib_linux_timespec duration = { (lib_linux_time_t)(milliseconds / 1000u),
        (long)(milliseconds % 1000u) * 1000000L };
    while (lib_linux_nanosleep(&duration, &duration) != 0 &&
        lib_linux_errno == LIB_LINUX_EINTR) { }
}
void base_sync_platform_yield(void) { (void)lib_linux_sched_yield(); }

lib_status base_sync_platform_event_create(lib_bool manual_reset,
    base_sync_event **out_event)
{
    base_sync_event *event;
    if (out_event == LIB_NULL) return LIB_STATUS_INVALID_ARGUMENT;
    *out_event = LIB_NULL;
    if (lib_linux_pthread_once(&base_sync_once, base_sync_initialize) != 0 ||
        base_sync_init_status != 0) return LIB_STATUS_IO_ERROR;
    event = lib_allocate_zero(1u, sizeof(*event));
    if (event == LIB_NULL) return LIB_STATUS_NO_MEMORY;
    event->manual_reset = manual_reset != LIB_FALSE;
    *out_event = event;
    return LIB_STATUS_OK;
}
void base_sync_platform_event_destroy(base_sync_event *event)
{
    if (event == LIB_NULL) return;
    lib_release(event);
}
lib_status base_sync_platform_event_signal(base_sync_event *event)
{
    int result;
    if (lib_linux_pthread_mutex_lock(&base_sync_lock) != 0) return LIB_STATUS_IO_ERROR;
    event->signaled = LIB_TRUE;
    result = lib_linux_pthread_cond_broadcast(&base_sync_changed);
    if (lib_linux_pthread_mutex_unlock(&base_sync_lock) != 0) return LIB_STATUS_IO_ERROR;
    return result == 0 ? LIB_STATUS_OK : LIB_STATUS_IO_ERROR;
}
lib_status base_sync_platform_event_reset(base_sync_event *event)
{
    if (lib_linux_pthread_mutex_lock(&base_sync_lock) != 0) return LIB_STATUS_IO_ERROR;
    event->signaled = LIB_FALSE;
    return lib_linux_pthread_mutex_unlock(&base_sync_lock) == 0 ?
        LIB_STATUS_OK : LIB_STATUS_IO_ERROR;
}

lib_status base_sync_platform_event_wait_many(
    const base_sync_event *const *events, lib_u32 event_count,
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
        !base_sync_platform_deadline(timeout_milliseconds, &deadline)) return LIB_STATUS_IO_ERROR;
    if (lib_linux_pthread_mutex_lock(&base_sync_lock) != 0) return LIB_STATUS_IO_ERROR;
    for (;;) {
        if (result != 0 && result != LIB_LINUX_ETIMEDOUT) break;
        for (index = 0u; index < event_count; ++index) {
            base_sync_event *event = (base_sync_event *)events[index];
            if (event->signaled != LIB_FALSE) {
                if (!event->manual_reset) event->signaled = LIB_FALSE;
                if (out_event_index != LIB_NULL) *out_event_index = index;
                *out_signaled = LIB_TRUE;
                goto done;
            }
        }
        if (timeout_milliseconds == 0u || result != 0) break;
        result = timeout_milliseconds == LIB_UINT32_MAX ?
            lib_linux_pthread_cond_wait(&base_sync_changed, &base_sync_lock) :
            lib_linux_pthread_cond_timedwait(&base_sync_changed, &base_sync_lock, &deadline);
    }
done:
    if (lib_linux_pthread_mutex_unlock(&base_sync_lock) != 0) return LIB_STATUS_IO_ERROR;
    return result == 0 || result == LIB_LINUX_ETIMEDOUT ? LIB_STATUS_OK : LIB_STATUS_IO_ERROR;
}

lib_status base_sync_platform_task_create(base_sync_task_entry entry,
    void *context, base_sync_event *cancellation, base_sync_task **out_task)
{
    struct base_sync_linux_task *task;
    if (out_task == LIB_NULL) return LIB_STATUS_INVALID_ARGUMENT;
    *out_task = LIB_NULL;
    if (entry == LIB_NULL) return LIB_STATUS_INVALID_ARGUMENT;
    task = lib_allocate_zero(1u, sizeof(*task));
    if (task == LIB_NULL) return LIB_STATUS_NO_MEMORY;
    task->task.entry = entry; task->task.context = context;
    task->task.cancellation = cancellation;
    if (lib_linux_pthread_create(&task->thread, LIB_NULL, base_sync_platform_main, task) != 0) {
        lib_release(task); return LIB_STATUS_IO_ERROR;
    }
    *out_task = &task->task; return LIB_STATUS_OK;
}
void base_sync_platform_task_join(base_sync_task *task)
{
    struct base_sync_linux_task *state = (struct base_sync_linux_task *)task;
    if (state != LIB_NULL && state->joined == LIB_FALSE) {
        (void)lib_linux_pthread_join(state->thread, LIB_NULL); state->joined = LIB_TRUE;
    }
}
void base_sync_platform_task_destroy(base_sync_task *task)
{ if (task != LIB_NULL) lib_release(task); }
