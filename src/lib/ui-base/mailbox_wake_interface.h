#ifndef UI_MAILBOX_WAKE_INTERFACE_H
#define UI_MAILBOX_WAKE_INTERFACE_H

#include "lib/types/types_interface.h"

typedef struct ui_mailbox_wake ui_mailbox_wake;

typedef enum ui_mailbox_wake_wait_result {
    UI_MAILBOX_WAKE_WAIT_WAKE,
    UI_MAILBOX_WAKE_WAIT_MESSAGE,
    UI_MAILBOX_WAKE_WAIT_TIMED_OUT,
    UI_MAILBOX_WAKE_WAIT_FAULT
} ui_mailbox_wake_wait_result;

ui_mailbox_wake *ui_mailbox_wake_create(void);
void ui_mailbox_wake_destroy(ui_mailbox_wake *wake);
void ui_mailbox_wake_signal(ui_mailbox_wake *wake);
ui_mailbox_wake_wait_result ui_mailbox_wake_wait(
    const ui_mailbox_wake *wake, lib_u32 timeout_milliseconds);
ui_mailbox_wake_wait_result ui_mailbox_wake_wait_messages(
    const ui_mailbox_wake *wake, lib_u32 timeout_milliseconds);
#endif
