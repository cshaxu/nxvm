#ifndef UX_LINUX_MAILBOX_WAKE_H
#define UX_LINUX_MAILBOX_WAKE_H

#if !defined(_WIN32)
int ux_linux_mailbox_wait_fd(const ux_mailbox_native *native_mailbox);
void ux_linux_mailbox_consume(const ux_mailbox_native *native_mailbox);
#endif

#endif
