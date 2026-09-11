#ifndef UI_MAILBOX_WAKE_H
#define UI_MAILBOX_WAKE_H

typedef struct ui_mailbox_wake ui_mailbox_wake;

ui_mailbox_wake *ui_mailbox_wake_create(void);
void ui_mailbox_wake_destroy(ui_mailbox_wake *wake);
void ui_mailbox_wake_signal(ui_mailbox_wake *wake);
#endif
