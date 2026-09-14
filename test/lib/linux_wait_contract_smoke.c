#include "linux_wait_fakes.h"
#include "lib/host/linux/sync.c"
#include "lib/kvm-base/linux/mailbox.c"
#include "lib/host/linux/console.c"

static host_sync_event *signal_event;
static kvm_mailbox_wake *signal_wake;
static void signal_on_second_wait(void)
{
    if (wait_calls != 2) return; /* First return is spurious. */
    if (signal_event) host_sync_platform_event_signal(signal_event);
    if (signal_wake) kvm_mailbox_wake_signal(signal_wake);
}

int main(void)
{
    host_sync_event *events[2];
    lib_bool signaled;
    lib_u32 index;
    kvm_mailbox_wake_wait_result wake_result;
    int failure;
    host_console_backend *backend = (void *)1;
    host_sync_mutex *mutex = (void *)1;

    assert(host_sync_mutex_create(NULL) == LIB_STATUS_INVALID_ARGUMENT);
    fail_init_step = 1; init_step = 0;
    assert(host_sync_mutex_create(&mutex) == LIB_STATUS_IO_ERROR);
    assert(mutex == NULL && live_mutexes == 0);
    fail_init_step = 0; init_step = 0;
    assert(host_sync_mutex_create(&mutex) == LIB_STATUS_OK);
    assert(live_mutexes == 1);
    host_sync_mutex_lock(mutex);
    assert(mutex->gate.locked);
    host_sync_mutex_unlock(mutex);
    assert(!mutex->gate.locked);
    host_sync_mutex_destroy(mutex);
    host_sync_mutex_destroy(NULL);
    assert(live_mutexes == 0);

    for (failure = 1; failure <= 4; ++failure) {
        fail_init_step = failure; init_step = 0;
        signal_wake = NULL;
        assert(kvm_mailbox_wake_create(&signal_wake) == LIB_STATUS_IO_ERROR);
        assert(signal_wake == NULL);
        assert(live_mutexes == 0 && live_conditions == 0 && live_attributes == 0);
    }
    /* Host one-time condition preparation has three fallible stages. */
    for (failure = 1; failure <= 3; ++failure) {
        fail_init_step = failure; init_step = 0;
        host_sync_once = LIB_LINUX_PTHREAD_ONCE_INIT;
        host_sync_init_status = -1;
        assert(host_sync_platform_event_create(0, &events[0]) == LIB_STATUS_IO_ERROR);
        assert(events[0] == NULL && live_conditions == 0 && live_attributes == 0);
    }
    host_sync_once = LIB_LINUX_PTHREAD_ONCE_INIT;
    fail_init_step = 0; init_step = 0;
    signal_wake = NULL;
    assert(kvm_mailbox_wake_create(&signal_wake) == LIB_STATUS_OK);
    assert(signal_wake != NULL);
    assert(kvm_mailbox_wake_wait(signal_wake, 0, &wake_result) == LIB_STATUS_OK &&
        wake_result == KVM_MAILBOX_WAKE_WAIT_TIMED_OUT);
    assert(kvm_mailbox_wake_signal(signal_wake)==LIB_STATUS_OK);
    assert(kvm_mailbox_wake_wait(signal_wake, 0, &wake_result) == LIB_STATUS_OK &&
        wake_result == KVM_MAILBOX_WAKE_WAIT_WAKE);
    assert(wait_calls == 0);
    wait_hook = signal_on_second_wait;
    assert(kvm_mailbox_wake_wait(signal_wake, 250, &wake_result) == LIB_STATUS_OK &&
        wake_result == KVM_MAILBOX_WAKE_WAIT_WAKE);
    assert(wait_calls == 2 && observed_deadline.tv_sec == 101 &&
        observed_deadline.tv_nsec == 150000000L);
    wait_calls = 0;
    assert(kvm_mailbox_wake_wait(signal_wake, LIB_UINT32_MAX, &wake_result) == LIB_STATUS_OK &&
        wake_result == KVM_MAILBOX_WAKE_WAIT_WAKE);
    assert(wait_calls == 2);
    wait_hook = NULL; wait_calls = 0; wait_result = LIB_LINUX_ETIMEDOUT;
    assert(kvm_mailbox_wake_wait(signal_wake, 1, &wake_result) == LIB_STATUS_OK &&
        wake_result == KVM_MAILBOX_WAKE_WAIT_TIMED_OUT);
    wait_calls = 0; wait_result = 5;
    assert(kvm_mailbox_wake_wait(signal_wake, LIB_UINT32_MAX, &wake_result) == LIB_STATUS_IO_ERROR);
    clock_failure = 1;
    assert(kvm_mailbox_wake_wait(signal_wake, 1, &wake_result) == LIB_STATUS_IO_ERROR);
    clock_failure = 0;
    assert(kvm_mailbox_wake_signal(NULL)==LIB_STATUS_INVALID_ARGUMENT);
    fail_lock=1;
    assert(kvm_mailbox_wake_signal(signal_wake)==LIB_STATUS_IO_ERROR);
    assert(!signal_wake->lock.locked);
    fail_lock=0;fail_signal=1;
    assert(kvm_mailbox_wake_signal(signal_wake)==LIB_STATUS_IO_ERROR);
    assert(!signal_wake->lock.locked);
    fail_signal=0;fail_unlock=1;
    assert(kvm_mailbox_wake_signal(signal_wake)==LIB_STATUS_IO_ERROR);
    fail_unlock=0;
    kvm_mailbox_wake_destroy(signal_wake); signal_wake = NULL;
    assert(live_mutexes == 0 && live_conditions == 0);

    assert(host_sync_platform_event_create(0, &events[0]) == LIB_STATUS_OK);
    assert(host_sync_platform_event_create(1, &events[1]) == LIB_STATUS_OK);
    signal_event = events[1]; wait_calls = 0; wait_result = 0;
    wait_hook = signal_on_second_wait;
    assert(host_sync_platform_event_wait_many((const host_sync_event *const *)events,
        2, 250, &signaled, &index) == LIB_STATUS_OK && signaled && index == 1);
    assert(wait_calls == 2 && observed_deadline.tv_sec == 101 &&
        observed_deadline.tv_nsec == 150000000L);
    assert(host_sync_platform_event_wait_many((const host_sync_event *const *)events,
        2, 0, &signaled, &index) == LIB_STATUS_OK && signaled && index == 1);
    host_sync_platform_event_reset(events[1]);
    signal_event = events[0]; wait_calls = 0;
    assert(host_sync_platform_event_wait_many((const host_sync_event *const *)events,
        2, LIB_UINT32_MAX, &signaled, &index) == LIB_STATUS_OK && signaled && index == 0);
    assert(wait_calls == 2);
    assert(host_sync_platform_event_wait_many((const host_sync_event *const *)events,
        2, 0, &signaled, &index) == LIB_STATUS_OK && !signaled);
    wait_hook = NULL; wait_calls = 0; wait_result = LIB_LINUX_ETIMEDOUT;
    assert(host_sync_platform_event_wait_many((const host_sync_event *const *)events,
        2, 1, &signaled, &index) == LIB_STATUS_OK && !signaled);
    wait_calls = 0; wait_result = 5;
    assert(host_sync_platform_event_wait_many((const host_sync_event *const *)events,
        2, LIB_UINT32_MAX, &signaled, &index) == LIB_STATUS_IO_ERROR);
    clock_failure = 1;
    assert(host_sync_platform_event_wait_many((const host_sync_event *const *)events,
        2, 1, &signaled, &index) == LIB_STATUS_IO_ERROR);
    assert(!host_sync_lock.locked && sleep_calls == 0);
    interrupt_sleep = 1;
    host_sync_platform_sleep_milliseconds(250);
    assert(sleep_calls == 2);
    host_sync_platform_event_destroy(events[0]);
    host_sync_platform_event_destroy(events[1]);
    assert(host_console_backend_create(&backend) == LIB_STATUS_UNSUPPORTED && !backend);
    assert(host_console_backend_request_cooked_line(NULL) == LIB_STATUS_UNSUPPORTED);
    return 0;
}
