#include "linux_wait_fakes.h"
static lib_u32 allocations, releases;
static void *allocate(lib_size count, lib_size size)
{ ++allocations; return lib_allocate_zero(count, size); }
static void *allocate_plain(lib_size size)
{ ++allocations; return lib_allocate(size); }
static void release(void *memory)
{ if (memory) ++releases; lib_release(memory); }
#undef lib_allocate_zero
#undef lib_allocate
#undef lib_release
#define lib_allocate_zero allocate
#define lib_allocate allocate_plain
#define lib_release release
#include "lib/base/linux/sync.c"
#include "lib/base/sync.c"
#include "lib/console-broker/linux/console.c"

static base_sync_event *signal_event;
static void task_entry(void *context, const base_sync_task *task)
{
    assert(task == *(base_sync_task **)context);
    assert(base_sync_task_wait_cancel(task, 0) == BASE_SYNC_WAIT_CANCELLED);
}
static void signal_on_second_wait(void)
{
    if (wait_calls != 2) return; /* First return is spurious. */
    if (signal_event) base_sync_platform_event_signal(signal_event);
}

int main(void)
{
    base_sync_event *events[2];
    lib_bool signaled;
    lib_u32 index;
    lib_i32 failure;
    console_broker_backend *backend = (void *)1;
    base_sync_mutex *mutex = (void *)1;

    assert(base_sync_mutex_create(NULL) == LIB_STATUS_INVALID_ARGUMENT);
    fail_init_step = 1; init_step = 0;
    assert(base_sync_mutex_create(&mutex) == LIB_STATUS_IO_ERROR);
    assert(mutex == NULL && live_mutexes == 0);
    fail_init_step = 0; init_step = 0;
    assert(base_sync_mutex_create(&mutex) == LIB_STATUS_OK);
    assert(live_mutexes == 1);
    base_sync_mutex_lock(mutex);
    assert(mutex->gate.locked);
    base_sync_mutex_unlock(mutex);
    assert(!mutex->gate.locked);
    base_sync_mutex_destroy(mutex);
    base_sync_mutex_destroy(NULL);
    assert(live_mutexes == 0);

    /* Base one-time condition preparation has three fallible stages. */
    for (failure = 1; failure <= 3; ++failure) {
        fail_init_step = failure; init_step = 0;
        base_sync_once = LIB_LINUX_PTHREAD_ONCE_INIT;
        base_sync_init_status = -1;
        assert(base_sync_platform_event_create(0, &events[0]) == LIB_STATUS_IO_ERROR);
        assert(events[0] == NULL && live_conditions == 0 && live_attributes == 0);
    }
    base_sync_once = LIB_LINUX_PTHREAD_ONCE_INIT;
    fail_init_step = 0; init_step = 0;
    assert(base_sync_event_create(BASE_SYNC_EVENT_AUTO_RESET, &signal_event) == LIB_STATUS_OK);
    assert(base_sync_event_wait(signal_event, 0) == BASE_SYNC_WAIT_TIMED_OUT);
    assert(base_sync_event_signal(signal_event) == LIB_STATUS_OK);
    assert(base_sync_event_signal(signal_event) == LIB_STATUS_OK);
    assert(base_sync_event_wait(signal_event, 0) == BASE_SYNC_WAIT_SIGNALED);
    assert(base_sync_event_wait(signal_event, 0) == BASE_SYNC_WAIT_TIMED_OUT);
    wait_hook = signal_on_second_wait;
    assert(base_sync_event_wait(signal_event, 250) == BASE_SYNC_WAIT_SIGNALED);
    assert(wait_calls == 2 && observed_deadline.tv_sec == 101 &&
        observed_deadline.tv_nsec == 150000000L);
    wait_calls = 0;
    assert(base_sync_event_wait(signal_event, LIB_UINT32_MAX) == BASE_SYNC_WAIT_SIGNALED);
    assert(wait_calls == 2);
    wait_hook = NULL; wait_calls = 0; wait_result = LIB_LINUX_ETIMEDOUT;
    assert(base_sync_event_wait(signal_event, 1) == BASE_SYNC_WAIT_TIMED_OUT);
    wait_calls = 0; wait_result = 5;
    assert(base_sync_event_wait(signal_event, LIB_UINT32_MAX) == BASE_SYNC_WAIT_FAULT);
    clock_failure = 1;
    assert(base_sync_event_wait(signal_event, 1) == BASE_SYNC_WAIT_FAULT);
    clock_failure = 0;
    assert(base_sync_event_signal(NULL) == LIB_STATUS_INVALID_ARGUMENT);
    assert(base_sync_event_reset(NULL) == LIB_STATUS_INVALID_ARGUMENT);
    fail_lock = 1;
    assert(base_sync_event_signal(signal_event) == LIB_STATUS_IO_ERROR);
    assert(base_sync_event_reset(signal_event) == LIB_STATUS_IO_ERROR);
    assert(!base_sync_lock.locked);
    fail_lock = 0; fail_signal = 1;
    assert(base_sync_event_signal(signal_event) == LIB_STATUS_IO_ERROR);
    assert(!base_sync_lock.locked);
    fail_signal = 0; fail_unlock = 1;
    assert(base_sync_event_signal(signal_event) == LIB_STATUS_IO_ERROR);
    assert(base_sync_event_reset(signal_event) == LIB_STATUS_IO_ERROR);
    assert(base_sync_event_wait(signal_event, 0) == BASE_SYNC_WAIT_FAULT);
    fail_unlock = 0;
    base_sync_event_destroy(signal_event); signal_event = NULL;
    assert(live_mutexes == 0 && live_conditions == 1 && live_attributes == 0);

    assert(base_sync_platform_event_create(0, &events[0]) == LIB_STATUS_OK);
    assert(base_sync_platform_event_create(1, &events[1]) == LIB_STATUS_OK);
    signal_event = events[1]; wait_calls = 0; wait_result = 0;
    wait_hook = signal_on_second_wait;
    assert(base_sync_platform_event_wait_many((const base_sync_event *const *)events,
        2, 250, &signaled, &index) == LIB_STATUS_OK && signaled && index == 1);
    assert(wait_calls == 2 && observed_deadline.tv_sec == 101 &&
        observed_deadline.tv_nsec == 150000000L);
    assert(base_sync_platform_event_wait_many((const base_sync_event *const *)events,
        2, 0, &signaled, &index) == LIB_STATUS_OK && signaled && index == 1);
    base_sync_platform_event_reset(events[1]);
    signal_event = events[0]; wait_calls = 0;
    assert(base_sync_platform_event_wait_many((const base_sync_event *const *)events,
        2, LIB_UINT32_MAX, &signaled, &index) == LIB_STATUS_OK && signaled && index == 0);
    assert(wait_calls == 2);
    assert(base_sync_platform_event_wait_many((const base_sync_event *const *)events,
        2, 0, &signaled, &index) == LIB_STATUS_OK && !signaled);
    wait_hook = NULL; wait_calls = 0; wait_result = LIB_LINUX_ETIMEDOUT;
    assert(base_sync_platform_event_wait_many((const base_sync_event *const *)events,
        2, 1, &signaled, &index) == LIB_STATUS_OK && !signaled);
    wait_calls = 0; wait_result = 5;
    assert(base_sync_platform_event_wait_many((const base_sync_event *const *)events,
        2, LIB_UINT32_MAX, &signaled, &index) == LIB_STATUS_IO_ERROR);
    clock_failure = 1;
    assert(base_sync_platform_event_wait_many((const base_sync_event *const *)events,
        2, 1, &signaled, &index) == LIB_STATUS_IO_ERROR);
    assert(!base_sync_lock.locked && sleep_calls == 0);
    interrupt_sleep = 1;
    base_sync_platform_sleep_milliseconds(250);
    assert(sleep_calls == 2);
    base_sync_platform_event_destroy(events[0]);
    base_sync_platform_event_destroy(events[1]);
    assert(console_broker_backend_create(&backend) == LIB_STATUS_UNSUPPORTED && !backend);
    assert(console_broker_backend_request_cooked_line(NULL) == LIB_STATUS_UNSUPPORTED);
    {
        base_sync_task *task;
        lib_u32 before = allocations;
        assert(base_sync_task_create(task_entry, &task, &task) == LIB_STATUS_OK);
        assert(allocations == before + 2); /* Task + existing cancellation Event. */
        assert(!base_sync_task_cancelled(task));
        assert(base_sync_task_request_cancel(task) == LIB_STATUS_OK);
        fail_join = 1;
        assert(base_sync_task_destroy(task) == LIB_STATUS_IO_ERROR);
        assert(allocations == releases + 2 && thread_joins == 1);
        assert(!((struct base_sync_linux_task *)task)->joined);
        fail_join = 0;
        assert(base_sync_task_join(task) == LIB_STATUS_OK);
        assert(base_sync_task_destroy(task) == LIB_STATUS_OK);
        assert(thread_joins == 2 && allocations == releases);
        fail_thread = 1;
        assert(base_sync_task_create(task_entry, &task, &task) == LIB_STATUS_IO_ERROR);
        assert(task == NULL && allocations == releases);
    }
    return 0;
}
