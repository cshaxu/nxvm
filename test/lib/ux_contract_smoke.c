#include "lib/ux/actions.h"
#include "lib/ux/internal/presenter_internal.h"
#include "lib/ux/presenter.h"

#include <string.h>

int main(void)
{
    ux_presenter *presenter = NULL;
    ux_presenter_control control;
    static ux_frame first;
    static ux_frame second;
    static ux_frame captured;
    ux_action_registry actions;
    lib_u32 index;

    first.valid = LIB_TRUE;
    first.text_columns = UX_TEXT_COLUMNS;
    first.text_rows = UX_TEXT_ROWS;
    first.text[0] = 'A';
    second = first;
    second.text[0] = 'B';
    if (ux_presenter_create(&presenter) != LIB_STATUS_OK ||
        ux_presenter_publish_frame(presenter, &first) != LIB_STATUS_OK ||
        ux_presenter_publish_frame(presenter, &second) != LIB_STATUS_OK ||
        ux_presenter_capture_frame(presenter, &captured) != LIB_STATUS_OK ||
        captured.sequence != 2u || captured.text[0] != 'B') goto fail;
    if (ux_presenter_set_window_title(presenter, "Neutral Window") !=
            LIB_STATUS_OK || ux_presenter_set_target(presenter, UX_TARGET_WINDOW) !=
            LIB_STATUS_OK || ux_presenter_stop(presenter) != LIB_STATUS_OK ||
        !ux_presenter_take_control(presenter, &control) ||
        control.kind != UX_PRESENTER_CONTROL_WINDOW_TITLE ||
        strcmp(control.title, "Neutral Window") != 0 ||
        !ux_presenter_take_control(presenter, &control) ||
        control.kind != UX_PRESENTER_CONTROL_TARGET ||
        control.target != UX_TARGET_WINDOW ||
        !ux_presenter_take_control(presenter, &control) ||
        control.kind != UX_PRESENTER_CONTROL_STOP ||
        ux_presenter_take_control(presenter, &control)) goto fail;
    for (index = 0u; index < UX_PRESENTER_CONTROL_CAPACITY; ++index) {
        if (ux_presenter_set_target(presenter, UX_TARGET_CONSOLE) != LIB_STATUS_OK)
            goto fail;
    }
    if (ux_presenter_stop(presenter) != LIB_STATUS_INVALID_STATE) goto fail;
    ux_actions_initialize(&actions);
    if (ux_actions_register(&actions, 'P', UX_MODIFIER_CONTROL | UX_MODIFIER_ALT,
            1u) != LIB_STATUS_OK || ux_actions_match(&actions, 'P',
            UX_MODIFIER_CONTROL | UX_MODIFIER_ALT) != 1u) goto fail;
    ux_presenter_destroy(presenter);
    return 0;

fail:
    ux_presenter_destroy(presenter);
    return 1;
}
