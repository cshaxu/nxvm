#include "lib/types/win32/scalar.h"
#include "lib/types/win32/window.h"
#include "lib/ui-base/mailbox_wake_interface.h"

#include "lib/types/win32/sync.h"

struct ui_mailbox_wake { lib_win32_handle handle; };

ui_mailbox_wake *ui_mailbox_wake_create(void)
{
    ui_mailbox_wake *wake = lib_allocate_zero(1u, sizeof(*wake));
    if (wake == LIB_NULL) return LIB_NULL;
    wake->handle = lib_win32_create_event_a(LIB_NULL, LIB_WIN32_FALSE, LIB_WIN32_FALSE, LIB_NULL);
    if (wake->handle == LIB_NULL) { lib_release(wake); return LIB_NULL; }
    return wake;
}
void ui_mailbox_wake_destroy(ui_mailbox_wake *wake)
{ if (wake != LIB_NULL) { if (wake->handle != LIB_NULL) (void)lib_win32_close_handle(wake->handle); lib_release(wake); } }
void ui_mailbox_wake_signal(ui_mailbox_wake *wake)
{ if (wake != LIB_NULL && wake->handle != LIB_NULL) (void)lib_win32_set_event(wake->handle); }

ui_mailbox_wake_wait_result ui_mailbox_wake_wait(
    const ui_mailbox_wake *wake, lib_u32 timeout_milliseconds)
{
    lib_win32_dword result;
    if (wake == LIB_NULL || wake->handle == LIB_NULL) return UI_MAILBOX_WAKE_WAIT_FAULT;
    result = lib_win32_wait_for_single_object(wake->handle, (lib_win32_dword)timeout_milliseconds);
    return result == LIB_WIN32_WAIT_OBJECT_0 ? UI_MAILBOX_WAKE_WAIT_WAKE :
        result == LIB_WIN32_WAIT_TIMEOUT ? UI_MAILBOX_WAKE_WAIT_TIMED_OUT : UI_MAILBOX_WAKE_WAIT_FAULT;
}

ui_mailbox_wake_wait_result ui_mailbox_wake_wait_messages(
    const ui_mailbox_wake *wake, lib_u32 timeout_milliseconds)
{
    lib_win32_handle handle;
    lib_win32_dword result;
    if (wake == LIB_NULL || wake->handle == LIB_NULL) return UI_MAILBOX_WAKE_WAIT_FAULT;
    handle = wake->handle;
    result = lib_win32_msg_wait_for_multiple_objects(1u, &handle, LIB_WIN32_FALSE,
        (lib_win32_dword)timeout_milliseconds, LIB_WIN32_QS_ALLINPUT);
    return result == LIB_WIN32_WAIT_OBJECT_0 ? UI_MAILBOX_WAKE_WAIT_WAKE :
        result == LIB_WIN32_WAIT_OBJECT_0 + 1u ? UI_MAILBOX_WAKE_WAIT_MESSAGE :
        result == LIB_WIN32_WAIT_TIMEOUT ? UI_MAILBOX_WAKE_WAIT_TIMED_OUT : UI_MAILBOX_WAKE_WAIT_FAULT;
}
