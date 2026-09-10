#include "lib/ux-base/mailbox_wake.h"
#include "lib/ux-base/win32/mailbox_wake.h"

#include <stdlib.h>

#ifdef _WIN32
#include <windows.h>

struct ux_mailbox_wake {
    HANDLE event;
};

ux_mailbox_wake *ux_mailbox_wake_create(void)
{
    ux_mailbox_wake *wake = calloc(1u, sizeof(*wake));

    if (wake == NULL) return NULL;
    wake->event = CreateEventA(NULL, FALSE, FALSE, NULL);
    if (wake->event != NULL) return wake;
    free(wake);
    return NULL;
}

void ux_mailbox_wake_destroy(ux_mailbox_wake *wake)
{
    if (wake == NULL) return;
    if (wake->event != NULL) CloseHandle(wake->event);
    free(wake);
}

void ux_mailbox_wake_signal(ux_mailbox_wake *wake)
{
    if (wake != NULL && wake->event != NULL)
        (void)SetEvent(wake->event);
}

HANDLE ux_win32_mailbox_wait_handle(const ux_mailbox_wake *wake)
{
    return wake == NULL ? NULL : wake->event;
}
#endif
