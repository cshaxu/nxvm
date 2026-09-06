#include "lib/ux/mailbox.h"

#ifdef _WIN32
#include "lib/ux/win32/mailbox.h"

#include <windows.h>

int main(void)
{
    ux_mailbox *mailbox = NULL;
    ux_frame frame = {0};
    HANDLE event;
    int failed = 0;

    if (ux_mailbox_create(&mailbox) != LIB_STATUS_OK) return 1;
    event = ux_win32_mailbox_wait_handle(mailbox);
    if (event == NULL || WaitForSingleObject(event, 0u) != WAIT_TIMEOUT)
        failed = 1;
    frame.valid = 1u;
    frame.text_columns = UX_TEXT_COLUMNS;
    frame.text_rows = UX_TEXT_ROWS;
    if (!failed && ux_mailbox_publish(mailbox, &frame) != LIB_STATUS_OK)
        failed = 1;
    if (!failed && WaitForSingleObject(event, 0u) != WAIT_OBJECT_0)
        failed = 1;
    if (!failed && WaitForSingleObject(event, 0u) != WAIT_TIMEOUT)
        failed = 1;
    if (!failed && ux_mailbox_generation(mailbox) != 1u)
        failed = 1;
    ux_mailbox_wake(mailbox);
    if (!failed && WaitForSingleObject(event, 0u) != WAIT_OBJECT_0)
        failed = 1;
    if (!failed && ux_mailbox_generation(mailbox) != 1u)
        failed = 1;
    ux_mailbox_destroy(mailbox);
    return failed;
}
#else
int main(void)
{
    return 0;
}
#endif
