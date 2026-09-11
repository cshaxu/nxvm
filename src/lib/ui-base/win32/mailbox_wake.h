#ifndef UI_WIN32_MAILBOX_WAKE_H
#define UI_WIN32_MAILBOX_WAKE_H

#ifdef _WIN32
#include <windows.h>

HANDLE ui_win32_mailbox_wait_handle(const ui_mailbox_wake *wake);
#endif

#endif
