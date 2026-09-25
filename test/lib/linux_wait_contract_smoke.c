#include "lib/types/test.h"
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
    lib_test_assert(task == *(base_sync_task **)context);
    lib_test_assert(base_sync_task_wait_cancel(task, 0) == BASE_SYNC_WAIT_CANCELLED);
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

    lib_test_assert(base_sync_mutex_create(LIB_NULL) == LIB_STATUS_INVALID_ARGUMENT);
    fail_init_step = 1; init_step = 0;
    lib_test_assert(base_sync_mutex_create(&mutex) == LIB_STATUS_IO_ERROR);
    lib_test_assert(mutex == LIB_NULL && live_mutexes == 0);
    fail_init_step = 0; init_step = 0;
    lib_test_assert(base_sync_mutex_create(&mutex) == LIB_STATUS_OK);
    lib_test_assert(live_mutexes == 1);
    base_sync_mutex_lock(mutex);
    lib_test_assert(mutex->gate.locked);
    base_sync_mutex_unlock(mutex);
    lib_test_assert(!mutex->gate.locked);
    base_sync_mutex_destroy(mutex);
    base_sync_mutex_destroy(LIB_NULL);
    lib_test_assert(live_mutexes == 0);

    /* Base one-time condition preparation has three fallible stages. */
    for (failure = 1; failure <= 3; ++failure) {
        fail_init_step = failure; init_step = 0;
        base_sync_once = LIB_LINUX_PTHREAD_ONCE_INIT;
        base_sync_init_status = -1;
        lib_test_assert(base_sync_platform_event_create(0, &events[0]) == LIB_STATUS_IO_ERROR);
        lib_test_assert(events[0] == LIB_NULL && live_conditions == 0 && live_attributes == 0);
    }
    base_sync_once = LIB_LINUX_PTHREAD_ONCE_INIT;
    fail_init_step = 0; init_step = 0;
    lib_test_assert(base_sync_event_create(BASE_SYNC_EVENT_AUTO_RESET, &signal_event) == LIB_STATUS_OK);
    lib_test_assert(base_sync_event_wait(signal_event, 0) == BASE_SYNC_WAIT_TIMED_OUT);
    lib_test_assert(base_sync_event_signal(signal_event) == LIB_STATUS_OK);
    lib_test_assert(base_sync_event_signal(signal_event) == LIB_STATUS_OK);
    lib_test_assert(base_sync_event_wait(signal_event, 0) == BASE_SYNC_WAIT_SIGNALED);
    lib_test_assert(base_sync_event_wait(signal_event, 0) == BASE_SYNC_WAIT_TIMED_OUT);
    wait_hook = signal_on_second_wait;
    lib_test_assert(base_sync_event_wait(signal_event, 250) == BASE_SYNC_WAIT_SIGNALED);
    lib_test_assert(wait_calls == 2 && observed_deadline.tv_sec == 101 &&
        observed_deadline.tv_nsec == 150000000L);
    wait_calls = 0;
    lib_test_assert(base_sync_event_wait(signal_event, LIB_UINT32_MAX) == BASE_SYNC_WAIT_SIGNALED);
    lib_test_assert(wait_calls == 2);
    wait_hook = LIB_NULL; wait_calls = 0; wait_result = LIB_LINUX_ETIMEDOUT;
    lib_test_assert(base_sync_event_wait(signal_event, 1) == BASE_SYNC_WAIT_TIMED_OUT);
    wait_calls = 0; wait_result = 5;
    lib_test_assert(base_sync_event_wait(signal_event, LIB_UINT32_MAX) == BASE_SYNC_WAIT_FAULT);
    clock_failure = 1;
    lib_test_assert(base_sync_event_wait(signal_event, 1) == BASE_SYNC_WAIT_FAULT);
    clock_failure = 0;
    lib_test_assert(base_sync_event_signal(LIB_NULL) == LIB_STATUS_INVALID_ARGUMENT);
    lib_test_assert(base_sync_event_reset(LIB_NULL) == LIB_STATUS_INVALID_ARGUMENT);
    fail_lock = 1;
    lib_test_assert(base_sync_event_signal(signal_event) == LIB_STATUS_IO_ERROR);
    lib_test_assert(base_sync_event_reset(signal_event) == LIB_STATUS_IO_ERROR);
    lib_test_assert(!base_sync_lock.locked);
    fail_lock = 0; fail_signal = 1;
    lib_test_assert(base_sync_event_signal(signal_event) == LIB_STATUS_IO_ERROR);
    lib_test_assert(!base_sync_lock.locked);
    fail_signal = 0; fail_unlock = 1;
    lib_test_assert(base_sync_event_signal(signal_event) == LIB_STATUS_IO_ERROR);
    lib_test_assert(base_sync_event_reset(signal_event) == LIB_STATUS_IO_ERROR);
    lib_test_assert(base_sync_event_wait(signal_event, 0) == BASE_SYNC_WAIT_FAULT);
    fail_unlock = 0;
    base_sync_event_destroy(signal_event); signal_event = LIB_NULL;
    lib_test_assert(live_mutexes == 0 && live_conditions == 1 && live_attributes == 0);

    lib_test_assert(base_sync_platform_event_create(0, &events[0]) == LIB_STATUS_OK);
    lib_test_assert(base_sync_platform_event_create(1, &events[1]) == LIB_STATUS_OK);
    signal_event = events[1]; wait_calls = 0; wait_result = 0;
    wait_hook = signal_on_second_wait;
    lib_test_assert(base_sync_platform_event_wait_many((const base_sync_event *const *)events,
        2, 250, &signaled, &index) == LIB_STATUS_OK && signaled && index == 1);
    lib_test_assert(wait_calls == 2 && observed_deadline.tv_sec == 101 &&
        observed_deadline.tv_nsec == 150000000L);
    lib_test_assert(base_sync_platform_event_wait_many((const base_sync_event *const *)events,
        2, 0, &signaled, &index) == LIB_STATUS_OK && signaled && index == 1);
    base_sync_platform_event_reset(events[1]);
    signal_event = events[0]; wait_calls = 0;
    lib_test_assert(base_sync_platform_event_wait_many((const base_sync_event *const *)events,
        2, LIB_UINT32_MAX, &signaled, &index) == LIB_STATUS_OK && signaled && index == 0);
    lib_test_assert(wait_calls == 2);
    lib_test_assert(base_sync_platform_event_wait_many((const base_sync_event *const *)events,
        2, 0, &signaled, &index) == LIB_STATUS_OK && !signaled);
    wait_hook = LIB_NULL; wait_calls = 0; wait_result = LIB_LINUX_ETIMEDOUT;
    lib_test_assert(base_sync_platform_event_wait_many((const base_sync_event *const *)events,
        2, 1, &signaled, &index) == LIB_STATUS_OK && !signaled);
    wait_calls = 0; wait_result = 5;
    lib_test_assert(base_sync_platform_event_wait_many((const base_sync_event *const *)events,
        2, LIB_UINT32_MAX, &signaled, &index) == LIB_STATUS_IO_ERROR);
    clock_failure = 1;
    lib_test_assert(base_sync_platform_event_wait_many((const base_sync_event *const *)events,
        2, 1, &signaled, &index) == LIB_STATUS_IO_ERROR);
    lib_test_assert(!base_sync_lock.locked && sleep_calls == 0);
    interrupt_sleep = 1;
    base_sync_platform_sleep_milliseconds(250);
    lib_test_assert(sleep_calls == 2);
    base_sync_platform_event_destroy(events[0]);
    base_sync_platform_event_destroy(events[1]);
    lib_test_assert(console_broker_backend_create(&backend) == LIB_STATUS_UNSUPPORTED && !backend);
    lib_test_assert(console_broker_backend_request_cooked_line(LIB_NULL) == LIB_STATUS_UNSUPPORTED);
    {
        base_sync_task *task;
        lib_u32 before = allocations;
        lib_test_assert(base_sync_task_create(task_entry, &task, &task) == LIB_STATUS_OK);
        lib_test_assert(allocations == before + 2); /* Task + existing cancellation Event. */
        lib_test_assert(!base_sync_task_cancelled(task));
        lib_test_assert(base_sync_task_request_cancel(task) == LIB_STATUS_OK);
        fail_join = 1;
        lib_test_assert(base_sync_task_destroy(task) == LIB_STATUS_IO_ERROR);
        lib_test_assert(allocations == releases + 2 && thread_joins == 1);
        lib_test_assert(!((struct base_sync_linux_task *)task)->joined);
        fail_join = 0;
        lib_test_assert(base_sync_task_join(task) == LIB_STATUS_OK);
        lib_test_assert(base_sync_task_destroy(task) == LIB_STATUS_OK);
        lib_test_assert(thread_joins == 2 && allocations == releases);
        fail_thread = 1;
        lib_test_assert(base_sync_task_create(task_entry, &task, &task) == LIB_STATUS_IO_ERROR);
        lib_test_assert(task == LIB_NULL && allocations == releases);
    }
    return 0;
}
