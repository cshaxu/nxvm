#include "type.h"

#include "core/platform/win32/keyboard.h"

typedef struct keyboard_capture {
    core_platform_input_event events[24u];
    STD_SIZE_T count;
} keyboard_capture;

static type_status keyboard_capture_submit(C_VOID *opaque,
    const core_platform_input_event *event)
{
    keyboard_capture *capture = opaque;

    if (capture == STD_NULL || event == STD_NULL || capture->count >= 24u) {
        return TYPE_STATUS_NO_MEMORY;
    }
    capture->events[capture->count++] = *event;
    return TYPE_STATUS_OK;
}

static C_INT keyboard_capture_has_no_stuck_key(const keyboard_capture *capture,
    STD_SIZE_T first)
{
    STD_SIZE_T index;
    STD_SIZE_T later;

    for (index = first; index < capture->count; ++index) {
        if (!capture->events[index].data.key.pressed) continue;
        for (later = index + 1u; later < capture->count; ++later) {
            if (!capture->events[later].data.key.pressed &&
                capture->events[later].data.key.scan_code ==
                    capture->events[index].data.key.scan_code) break;
        }
        if (later == capture->count) return 0;
    }
    return 1;
}

C_INT main(C_VOID)
{
    keyboard_capture capture = {0};
    core_platform_win32_keyboard_normalizer normalizer = {0};

    if (core_platform_win32_keyboard_submit_key(&capture, keyboard_capture_submit,
            0u, 0x70u, TYPE_TRUE) != TYPE_STATUS_OK || capture.count != 1u ||
        !capture.events[0].data.key.pressed ||
        core_platform_win32_keyboard_submit_key(&capture, keyboard_capture_submit,
            0x0148u, 0x26u, TYPE_TRUE) != TYPE_STATUS_OK ||
        capture.events[1u].data.key.scan_code != 0x0148u ||
        core_platform_win32_keyboard_submit_character(&capture,
            keyboard_capture_submit, 'a') != TYPE_STATUS_OK || capture.count < 2u ||
        !capture.events[capture.count - 2u].data.key.pressed ||
        capture.events[capture.count - 1u].data.key.pressed ||
        !core_platform_win32_keyboard_character_matches_virtual_key('a',
            capture.events[capture.count - 1u].data.key.virtual_key) ||
        core_platform_win32_keyboard_submit_character(&capture,
            keyboard_capture_submit, 0xd800u) != TYPE_STATUS_UNSUPPORTED ||
        core_platform_win32_keyboard_submit_utf16(&normalizer, &capture,
            keyboard_capture_submit, 0xdc00u) != TYPE_STATUS_UNSUPPORTED ||
        core_platform_win32_keyboard_submit_utf16(&normalizer, &capture,
            keyboard_capture_submit, 0xd800u) != TYPE_STATUS_OK ||
        core_platform_win32_keyboard_submit_utf16(&normalizer, &capture,
            keyboard_capture_submit, 0xdc00u) != TYPE_STATUS_UNSUPPORTED ||
        core_platform_win32_keyboard_submit_character(&capture,
            keyboard_capture_submit, 'A') != TYPE_STATUS_OK ||
        !keyboard_capture_has_no_stuck_key(&capture, 2u)) return 1;
    core_platform_win32_keyboard_note_recovered_key(&normalizer, 0x41u);
    if (!core_platform_win32_keyboard_consume_duplicate_character(&normalizer, 'a') ||
        core_platform_win32_keyboard_consume_duplicate_character(&normalizer, 'a')) return 1;
    core_platform_win32_keyboard_note_recovered_key(&normalizer, 0x70u);
    if (core_platform_win32_keyboard_consume_duplicate_character(&normalizer, 'a')) return 1;
    core_platform_win32_keyboard_note_recovered_key(&normalizer, 0x41u);
    core_platform_win32_keyboard_release_recovered_key(&normalizer, 0x41u);
    if (core_platform_win32_keyboard_consume_duplicate_character(&normalizer, 'a') ||
        core_platform_win32_keyboard_resolve_scan(0x70u) == 0u) return 1;
    return 0;
}
