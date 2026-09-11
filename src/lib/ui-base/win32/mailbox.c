#include "lib/ui-base/mailbox_wake.h"
#include "lib/ui-base/win32/mailbox_wake.h"

#include <stdlib.h>

#ifdef _WIN32
#include <windows.h>

struct ui_mailbox_wake {
    HANDLE event;
};

ui_mailbox_wake *ui_mailbox_wake_create(void)
{
    ui_mailbox_wake *wake = calloc(1u, sizeof(*wake));

    if (wake == NULL) return NULL;
    wake->event = CreateEventA(NULL, FALSE, FALSE, NULL);
    if (wake->event != NULL) return wake;
    free(wake);
    return NULL;
}

void ui_mailbox_wake_destroy(ui_mailbox_wake *wake)
{
    if (wake == NULL) return;
    if (wake->event != NULL) CloseHandle(wake->event);
    free(wake);
}

void ui_mailbox_wake_signal(ui_mailbox_wake *wake)
{
    if (wake != NULL && wake->event != NULL)
        (void)SetEvent(wake->event);
}

HANDLE ui_win32_mailbox_wait_handle(const ui_mailbox_wake *wake)
{
    return wake == NULL ? NULL : wake->event;
}
#endif
