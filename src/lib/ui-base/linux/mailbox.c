#define _POSIX_C_SOURCE 200809L

#include "lib/ui-base/mailbox_wake_interface.h"

#include "lib/types/linux/sync.h"
#include "lib/types/linux/clock.h"

struct ui_mailbox_wake {
    lib_linux_pthread_mutex_t lock;
    lib_linux_pthread_cond_t changed;
    lib_bool signaled;
};

ui_mailbox_wake *ui_mailbox_wake_create(void)
{
    lib_linux_pthread_condattr_t attributes;
    int result;
    ui_mailbox_wake *wake = lib_allocate_zero(1u, sizeof(*wake));
    if (wake == LIB_NULL) return LIB_NULL;
    if (lib_linux_pthread_mutex_init(&wake->lock, LIB_NULL) != 0) goto release;
    if (lib_linux_pthread_condattr_init(&attributes) != 0) goto destroy_lock;
    result = lib_linux_pthread_condattr_setclock(&attributes, LIB_LINUX_CLOCK_MONOTONIC);
    if (result == 0) result = lib_linux_pthread_cond_init(&wake->changed, &attributes);
    (void)lib_linux_pthread_condattr_destroy(&attributes);
    if (result != 0) goto destroy_lock;
    return wake;
destroy_lock:
    (void)lib_linux_pthread_mutex_destroy(&wake->lock);
release:
    lib_release(wake);
    return LIB_NULL;
}
void ui_mailbox_wake_destroy(ui_mailbox_wake *wake)
{ if (wake != LIB_NULL) { (void)lib_linux_pthread_cond_destroy(&wake->changed); (void)lib_linux_pthread_mutex_destroy(&wake->lock); lib_release(wake); } }
void ui_mailbox_wake_signal(ui_mailbox_wake *wake)
{ if (wake != LIB_NULL) { (void)lib_linux_pthread_mutex_lock(&wake->lock); wake->signaled = LIB_TRUE; (void)lib_linux_pthread_cond_signal(&wake->changed); (void)lib_linux_pthread_mutex_unlock(&wake->lock); } }
ui_mailbox_wake_wait_result ui_mailbox_wake_wait(
    const ui_mailbox_wake *wake, lib_u32 timeout_milliseconds)
{
    lib_linux_timespec deadline;
    int result = 0;
    ui_mailbox_wake_wait_result outcome = UI_MAILBOX_WAKE_WAIT_TIMED_OUT;
    ui_mailbox_wake *value = (ui_mailbox_wake *)wake;
    if (value == LIB_NULL) return UI_MAILBOX_WAKE_WAIT_FAULT;
    if (timeout_milliseconds != 0u && timeout_milliseconds != LIB_UINT32_MAX) {
        if (lib_linux_clock_gettime(LIB_LINUX_CLOCK_MONOTONIC, &deadline) != 0)
            return UI_MAILBOX_WAKE_WAIT_FAULT;
        deadline.tv_sec += (lib_linux_time_t)(timeout_milliseconds / 1000u);
        deadline.tv_nsec += (long)(timeout_milliseconds % 1000u) * 1000000L;
        if (deadline.tv_nsec >= 1000000000L) {
            ++deadline.tv_sec;
            deadline.tv_nsec -= 1000000000L;
        }
    }
    if (lib_linux_pthread_mutex_lock(&value->lock) != 0) return UI_MAILBOX_WAKE_WAIT_FAULT;
    while (!value->signaled && timeout_milliseconds != 0u && result == 0) {
        result = timeout_milliseconds == LIB_UINT32_MAX ?
            lib_linux_pthread_cond_wait(&value->changed, &value->lock) :
            lib_linux_pthread_cond_timedwait(&value->changed, &value->lock, &deadline);
    }
    if (result != 0 && result != LIB_LINUX_ETIMEDOUT) outcome = UI_MAILBOX_WAKE_WAIT_FAULT;
    else if (value->signaled) {
        value->signaled = LIB_FALSE;
        outcome = UI_MAILBOX_WAKE_WAIT_WAKE;
    }
    (void)lib_linux_pthread_mutex_unlock(&value->lock);
    return outcome;
}
ui_mailbox_wake_wait_result ui_mailbox_wake_wait_messages(
    const ui_mailbox_wake *wake, lib_u32 timeout_milliseconds)
{ return ui_mailbox_wake_wait(wake, timeout_milliseconds); }
