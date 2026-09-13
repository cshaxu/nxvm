#include "lib/types/win32/scalar.h"
#include "lib/kvm-base/mailbox_wake_interface.h"

#include "lib/types/win32/sync.h"

struct kvm_mailbox_wake { lib_win32_handle handle; };

lib_status kvm_mailbox_wake_create(kvm_mailbox_wake **out_wake)
{
    kvm_mailbox_wake *wake;
    if (out_wake == LIB_NULL) return LIB_STATUS_INVALID_ARGUMENT;
    *out_wake = LIB_NULL;
    wake = lib_allocate_zero(1u, sizeof(*wake));
    if (wake == LIB_NULL) return LIB_STATUS_NO_MEMORY;
    wake->handle = lib_win32_create_event_a(LIB_NULL, LIB_WIN32_FALSE, LIB_WIN32_FALSE, LIB_NULL);
    if (wake->handle == LIB_NULL) { lib_release(wake); return LIB_STATUS_IO_ERROR; }
    *out_wake = wake;
    return LIB_STATUS_OK;
}
void kvm_mailbox_wake_destroy(kvm_mailbox_wake *wake)
{
    if (wake == LIB_NULL) return;
    (void)lib_win32_close_handle(wake->handle);
    lib_release(wake);
}
lib_status kvm_mailbox_wake_signal(kvm_mailbox_wake *wake)
{
    if (wake == LIB_NULL || wake->handle == LIB_NULL) return LIB_STATUS_INVALID_ARGUMENT;
    return lib_win32_set_event(wake->handle) ? LIB_STATUS_OK : LIB_STATUS_IO_ERROR;
}

lib_status kvm_mailbox_wake_wait(const kvm_mailbox_wake *wake,
    lib_u32 timeout_milliseconds, kvm_mailbox_wake_wait_result *out_result)
{
    lib_win32_dword result;
    if (wake == LIB_NULL || wake->handle == LIB_NULL || out_result == LIB_NULL)
        return LIB_STATUS_INVALID_ARGUMENT;
    *out_result = KVM_MAILBOX_WAKE_WAIT_TIMED_OUT;
    result = lib_win32_wait_for_single_object(wake->handle, (lib_win32_dword)timeout_milliseconds);
    if (result == LIB_WIN32_WAIT_OBJECT_0) *out_result = KVM_MAILBOX_WAKE_WAIT_WAKE;
    return result == LIB_WIN32_WAIT_OBJECT_0 || result == LIB_WIN32_WAIT_TIMEOUT ?
        LIB_STATUS_OK : LIB_STATUS_IO_ERROR;
}
