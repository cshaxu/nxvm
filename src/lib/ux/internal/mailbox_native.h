#ifndef UX_MAILBOX_NATIVE_H
#define UX_MAILBOX_NATIVE_H

#include "lib/ux/mailbox.h"

typedef struct ux_mailbox_native ux_mailbox_native;

ux_mailbox_native *ux_mailbox_native_create(void);
void ux_mailbox_native_destroy(ux_mailbox_native *native_mailbox);
void ux_mailbox_native_signal(ux_mailbox_native *native_mailbox);
void *ux_mailbox_native_wait_handle(const ux_mailbox_native *native_mailbox);
void *ux_mailbox_native_wait_handle_for_mailbox(const ux_mailbox *mailbox);

#endif
