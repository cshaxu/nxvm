#include "lib/ux/internal/mailbox_native.h"
#include "mailbox.h"

#ifdef _WIN32
#include <windows.h>

struct ux_mailbox_native {
    HANDLE event;
};

ux_mailbox_native *ux_mailbox_native_create(void)
{
    ux_mailbox_native *native_mailbox = calloc(1u, sizeof(*native_mailbox));

    if (native_mailbox == NULL) return NULL;
    native_mailbox->event = CreateEventA(NULL, FALSE, FALSE, NULL);
    if (native_mailbox->event != NULL) return native_mailbox;
    free(native_mailbox);
    return NULL;
}

void ux_mailbox_native_destroy(ux_mailbox_native *native_mailbox)
{
    if (native_mailbox == NULL) return;
    if (native_mailbox->event != NULL) CloseHandle(native_mailbox->event);
    free(native_mailbox);
}

void ux_mailbox_native_signal(ux_mailbox_native *native_mailbox)
{
    if (native_mailbox != NULL && native_mailbox->event != NULL)
        (void)SetEvent(native_mailbox->event);
}

void *ux_mailbox_native_wait_handle(const ux_mailbox_native *native_mailbox)
{
    return native_mailbox == NULL ? NULL : native_mailbox->event;
}

HANDLE ux_win32_mailbox_wait_handle(const ux_mailbox *mailbox)
{
    return (HANDLE)ux_mailbox_native_wait_handle_for_mailbox(mailbox);
}
#endif
