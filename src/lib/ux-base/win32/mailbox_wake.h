#ifndef UX_WIN32_MAILBOX_WAKE_H
#define UX_WIN32_MAILBOX_WAKE_H

#ifdef _WIN32
#include <windows.h>

HANDLE ux_win32_mailbox_wait_handle(const ux_mailbox_wake *wake);
#endif

#endif
