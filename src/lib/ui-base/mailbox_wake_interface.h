#ifndef UI_MAILBOX_WAKE_INTERFACE_H
#define UI_MAILBOX_WAKE_INTERFACE_H

#include "lib/types/types_interface.h"

typedef struct ui_mailbox_wake ui_mailbox_wake;

typedef enum ui_mailbox_wake_wait_result {
    UI_MAILBOX_WAKE_WAIT_WAKE,
    UI_MAILBOX_WAKE_WAIT_TIMED_OUT
} ui_mailbox_wake_wait_result;

lib_status ui_mailbox_wake_create(ui_mailbox_wake **out_wake);
void ui_mailbox_wake_destroy(ui_mailbox_wake *wake);
lib_status ui_mailbox_wake_signal(ui_mailbox_wake *wake);
lib_status ui_mailbox_wake_wait(const ui_mailbox_wake *wake,
    lib_u32 timeout_milliseconds, ui_mailbox_wake_wait_result *out_result);
#endif
