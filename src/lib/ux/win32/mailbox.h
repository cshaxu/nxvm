#ifndef UX_WIN32_MAILBOX_H
#define UX_WIN32_MAILBOX_H

#include "lib/ux/mailbox.h"

#ifdef _WIN32
#include <windows.h>

HANDLE ux_win32_mailbox_wait_handle(const ux_mailbox *mailbox);
#endif

#endif
