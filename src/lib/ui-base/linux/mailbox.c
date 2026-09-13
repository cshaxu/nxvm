#define _POSIX_C_SOURCE 200809L

#include "lib/ui-base/mailbox_wake_interface.h"

#include "lib/types/linux/sync.h"
#include "lib/types/linux/clock.h"

struct ui_mailbox_wake {
    lib_linux_pthread_mutex_t lock;
    lib_linux_pthread_cond_t changed;
    lib_bool signaled;
};

lib_status ui_mailbox_wake_create(ui_mailbox_wake **out_wake)
{
    lib_linux_pthread_condattr_t attributes;
    int result;
    ui_mailbox_wake *wake;
    if (out_wake == LIB_NULL) return LIB_STATUS_INVALID_ARGUMENT;
    *out_wake = LIB_NULL;
    wake = lib_allocate_zero(1u, sizeof(*wake));
    if (wake == LIB_NULL) return LIB_STATUS_NO_MEMORY;
    if (lib_linux_pthread_mutex_init(&wake->lock, LIB_NULL) != 0) goto release;
    if (lib_linux_pthread_condattr_init(&attributes) != 0) goto destroy_lock;
    result = lib_linux_pthread_condattr_setclock(&attributes, LIB_LINUX_CLOCK_MONOTONIC);
    if (result == 0) result = lib_linux_pthread_cond_init(&wake->changed, &attributes);
    (void)lib_linux_pthread_condattr_destroy(&attributes);
    if (result != 0) goto destroy_lock;
    *out_wake = wake;
    return LIB_STATUS_OK;
destroy_lock:
    (void)lib_linux_pthread_mutex_destroy(&wake->lock);
release:
    lib_release(wake);
    return LIB_STATUS_IO_ERROR;
}
void ui_mailbox_wake_destroy(ui_mailbox_wake *wake)
{
    if (wake == LIB_NULL) return;
    (void)lib_linux_pthread_cond_destroy(&wake->changed);
    (void)lib_linux_pthread_mutex_destroy(&wake->lock);
    lib_release(wake);
}
lib_status ui_mailbox_wake_signal(ui_mailbox_wake *wake)
{
    int result;
    if (wake == LIB_NULL) return LIB_STATUS_INVALID_ARGUMENT;
    if (lib_linux_pthread_mutex_lock(&wake->lock) != 0) return LIB_STATUS_IO_ERROR;
    wake->signaled = LIB_TRUE;
    result = lib_linux_pthread_cond_signal(&wake->changed);
    if (lib_linux_pthread_mutex_unlock(&wake->lock) != 0) result = -1;
    return result == 0 ? LIB_STATUS_OK : LIB_STATUS_IO_ERROR;
}
lib_status ui_mailbox_wake_wait(const ui_mailbox_wake *wake,
    lib_u32 timeout_milliseconds, ui_mailbox_wake_wait_result *out_result)
{
    lib_linux_timespec deadline;
    int result = 0;
    lib_status status = LIB_STATUS_OK;
    ui_mailbox_wake *value = (ui_mailbox_wake *)wake;
    if (value == LIB_NULL || out_result == LIB_NULL) return LIB_STATUS_INVALID_ARGUMENT;
    *out_result = UI_MAILBOX_WAKE_WAIT_TIMED_OUT;
    if (timeout_milliseconds != 0u && timeout_milliseconds != LIB_UINT32_MAX) {
        if (lib_linux_clock_gettime(LIB_LINUX_CLOCK_MONOTONIC, &deadline) != 0)
            return LIB_STATUS_IO_ERROR;
        deadline.tv_sec += (lib_linux_time_t)(timeout_milliseconds / 1000u);
        deadline.tv_nsec += (long)(timeout_milliseconds % 1000u) * 1000000L;
        if (deadline.tv_nsec >= 1000000000L) {
            ++deadline.tv_sec;
            deadline.tv_nsec -= 1000000000L;
        }
    }
    if (lib_linux_pthread_mutex_lock(&value->lock) != 0) return LIB_STATUS_IO_ERROR;
    while (!value->signaled && timeout_milliseconds != 0u && result == 0) {
        result = timeout_milliseconds == LIB_UINT32_MAX ?
            lib_linux_pthread_cond_wait(&value->changed, &value->lock) :
            lib_linux_pthread_cond_timedwait(&value->changed, &value->lock, &deadline);
    }
    if (result != 0 && result != LIB_LINUX_ETIMEDOUT) status = LIB_STATUS_IO_ERROR;
    else if (value->signaled) {
        value->signaled = LIB_FALSE;
        *out_result = UI_MAILBOX_WAKE_WAIT_WAKE;
    }
    if (lib_linux_pthread_mutex_unlock(&value->lock) != 0) status = LIB_STATUS_IO_ERROR;
    return status;
}
