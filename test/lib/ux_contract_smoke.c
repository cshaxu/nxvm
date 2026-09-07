#include "lib/ux/actions.h"
#include "lib/ux/internal/presenter_internal.h"
#include "lib/ux/presenter.h"

#include <string.h>

int main(void)
{
    ux_presenter *presenter = NULL;
    ux_target target;
    lib_bool mouse_capturable;
    char title[UX_WINDOW_TITLE_CAPACITY];
    static ux_frame first;
    static ux_frame second;
    static ux_frame captured;
    ux_action_registry actions;

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
    if (ux_presenter_mouse_capture_state(presenter) != UX_MOUSE_CAPTURE_RELEASED ||
        ux_presenter_set_mouse_capturable(presenter, LIB_TRUE) != LIB_STATUS_OK ||
        ux_presenter_capture_mouse_capturable(presenter, &mouse_capturable) == 0u ||
        mouse_capturable != LIB_TRUE || ux_presenter_release_mouse(presenter) !=
            LIB_STATUS_OK || !ux_presenter_take_mouse_release(presenter) ||
        ux_presenter_take_mouse_release(presenter) || ux_presenter_set_mouse_capturable(
            presenter, LIB_FALSE) != LIB_STATUS_OK ||
        ux_presenter_capture_mouse_capturable(presenter, &mouse_capturable) == 0u ||
        mouse_capturable != LIB_FALSE || ux_presenter_take_mouse_release(presenter) ||
        ux_presenter_set_mouse_capturable(presenter, LIB_TRUE) != LIB_STATUS_OK ||
        ux_presenter_capture_mouse_capturable(presenter, &mouse_capturable) == 0u ||
        mouse_capturable != LIB_TRUE) goto fail;
    if (ux_presenter_set_window_title(presenter, "first") != LIB_STATUS_OK ||
        ux_presenter_set_window_title(presenter, "Neutral Window") != LIB_STATUS_OK ||
        ux_presenter_capture_window_title(presenter, title) != 2u ||
        strcmp(title, "Neutral Window") != 0 ||
        ux_presenter_set_target(presenter, UX_TARGET_CONSOLE) != LIB_STATUS_OK ||
        ux_presenter_set_target(presenter, UX_TARGET_WINDOW) != LIB_STATUS_OK ||
        ux_presenter_set_target(presenter, UX_TARGET_NONE) != LIB_STATUS_OK ||
        ux_presenter_capture_target(presenter, &target) != 4u ||
        target != UX_TARGET_NONE) goto fail;
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
