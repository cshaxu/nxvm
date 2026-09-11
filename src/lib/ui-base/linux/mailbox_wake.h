#ifndef UI_LINUI_MAILBOX_WAKE_H
#define UI_LINUI_MAILBOX_WAKE_H

#if !defined(_WIN32)
int ui_linui_mailbox_wait_fd(const ui_mailbox_wake *wake);
void ui_linui_mailbox_consume(const ui_mailbox_wake *wake);
#endif

#endif
