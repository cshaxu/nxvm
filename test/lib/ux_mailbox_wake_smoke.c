#include "lib/ux/internal/presenter_internal.h"
#include "lib/ux/presenter.h"

#ifdef _WIN32
#include "lib/ux/internal/win32_presenter_wake.h"

#include <windows.h>

int main(void)
{
    ux_presenter *presenter = NULL;
    ux_frame frame = {0};
    ux_presenter_control control;
    HANDLE event;
    int failed = 0;

    if (ux_presenter_create(&presenter) != LIB_STATUS_OK) return 1;
    event = ux_win32_presenter_wait_handle(presenter);
    if (event == NULL || WaitForSingleObject(event, 0u) != WAIT_TIMEOUT)
        failed = 1;
    frame.valid = 1u;
    frame.text_columns = UX_TEXT_COLUMNS;
    frame.text_rows = UX_TEXT_ROWS;
    if (!failed && ux_presenter_publish_frame(presenter, &frame) != LIB_STATUS_OK)
        failed = 1;
    if (!failed && WaitForSingleObject(event, 0u) != WAIT_OBJECT_0)
        failed = 1;
    if (!failed && ux_presenter_set_window_title(presenter, "title") !=
        LIB_STATUS_OK) failed = 1;
    if (!failed && WaitForSingleObject(event, 0u) != WAIT_OBJECT_0)
        failed = 1;
    if (!failed && (!ux_presenter_take_control(presenter, &control) ||
        control.kind != UX_PRESENTER_CONTROL_WINDOW_TITLE)) failed = 1;
    ux_presenter_destroy(presenter);
    return failed;
}
#else
int main(void)
{
    return 0;
}
#endif
