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
    core_platform_win32_keyboard_utf16 utf16 = {0};

    if (core_platform_win32_keyboard_submit_key(&capture, keyboard_capture_submit,
            0u, 0x70u, TYPE_TRUE) != TYPE_STATUS_OK || capture.count != 1u ||
        !capture.events[0].data.key.pressed ||
        core_platform_win32_keyboard_submit_character(&capture,
            keyboard_capture_submit, 'a') != TYPE_STATUS_OK || capture.count < 2u ||
        !capture.events[capture.count - 2u].data.key.pressed ||
        capture.events[capture.count - 1u].data.key.pressed ||
        !core_platform_win32_keyboard_character_matches_virtual_key('a',
            capture.events[capture.count - 1u].data.key.virtual_key) ||
        core_platform_win32_keyboard_submit_character(&capture,
            keyboard_capture_submit, 0xd800u) != TYPE_STATUS_UNSUPPORTED ||
        core_platform_win32_keyboard_submit_utf16(&utf16, &capture,
            keyboard_capture_submit, 0xdc00u) != TYPE_STATUS_UNSUPPORTED ||
        core_platform_win32_keyboard_submit_utf16(&utf16, &capture,
            keyboard_capture_submit, 0xd800u) != TYPE_STATUS_OK ||
        core_platform_win32_keyboard_submit_utf16(&utf16, &capture,
            keyboard_capture_submit, 0xdc00u) != TYPE_STATUS_UNSUPPORTED ||
        core_platform_win32_keyboard_resolve_scan(0x70u) == 0u) return 1;
    return 0;
}
