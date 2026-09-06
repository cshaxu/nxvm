#ifndef UX_LINUX_MAILBOX_H
#define UX_LINUX_MAILBOX_H

#include "lib/ux/mailbox.h"

#if !defined(_WIN32)
int ux_linux_mailbox_wait_fd(const ux_mailbox *mailbox);
void ux_linux_mailbox_consume(const ux_mailbox *mailbox);
#endif

#endif
