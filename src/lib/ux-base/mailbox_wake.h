#ifndef UX_MAILBOX_WAKE_H
#define UX_MAILBOX_WAKE_H

typedef struct ux_mailbox_wake ux_mailbox_wake;

ux_mailbox_wake *ux_mailbox_wake_create(void);
void ux_mailbox_wake_destroy(ux_mailbox_wake *wake);
void ux_mailbox_wake_signal(ux_mailbox_wake *wake);
#endif
