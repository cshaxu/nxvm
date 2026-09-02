#include "type.h"

#include "core/platform/win32/keyboard.h"

typedef struct keyboard_capture {
    core_platform_input_event events[8u];
    STD_SIZE_T count;
} keyboard_capture;

static type_status keyboard_capture_submit(C_VOID *opaque,
    const core_platform_input_event *event)
{
    keyboard_capture *capture = opaque;

    if (capture == STD_NULL || event == STD_NULL || capture->count >= 8u) {
        return TYPE_STATUS_NO_MEMORY;
    }
    capture->events[capture->count++] = *event;
    return TYPE_STATUS_OK;
}

C_INT main(C_VOID)
{
    keyboard_capture capture = {0};

    if (core_platform_win32_keyboard_submit_character(&capture,
            keyboard_capture_submit, 'a') != TYPE_STATUS_OK || capture.count < 2u ||
        !capture.events[capture.count - 2u].data.key.pressed ||
        capture.events[capture.count - 1u].data.key.pressed ||
        core_platform_win32_keyboard_submit_character(&capture,
            keyboard_capture_submit, 0xd800u) != TYPE_STATUS_UNSUPPORTED) return 1;
    return 0;
}
