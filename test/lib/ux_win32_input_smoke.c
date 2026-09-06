#include "lib/ux/win32/input.h"

typedef struct ux_keyboard_capture {
    ux_event events[24u];
    size_t count;
} ux_keyboard_capture;

static int ux_keyboard_capture_submit(void *opaque, const ux_event *event)
{
    ux_keyboard_capture *capture = opaque;

    if (capture == NULL || event == NULL || capture->count >= 24u) return 0;
    capture->events[capture->count++] = *event;
    return 1;
}

static int ux_keyboard_capture_has_no_stuck_key(const ux_keyboard_capture *capture,
    size_t first)
{
    size_t index;
    size_t later;

    for (index = first; index < capture->count; ++index) {
        if (capture->events[index].type != UX_EVENT_KEY ||
            !capture->events[index].data.key.pressed) continue;
        for (later = index + 1u; later < capture->count; ++later) {
            if (capture->events[later].type == UX_EVENT_KEY &&
                !capture->events[later].data.key.pressed &&
                capture->events[later].data.key.scan_code ==
                    capture->events[index].data.key.scan_code) break;
        }
        if (later == capture->count) return 0;
    }
    return 1;
}

int main(void)
{
    ux_keyboard_capture capture = {0};
    ux_win32_keyboard_normalizer normalizer = {0};

    if (!ux_win32_keyboard_submit_transition(&capture, ux_keyboard_capture_submit,
            0u, 0x70u, 0u, 1) || capture.count != 1u ||
        capture.events[0].type != UX_EVENT_KEY || !capture.events[0].data.key.pressed ||
        !ux_win32_keyboard_submit_transition(&capture, ux_keyboard_capture_submit,
            0x0148u, 0x26u, 0u, 1) ||
        capture.events[1u].data.key.scan_code != 0x0148u ||
        !ux_win32_keyboard_submit_utf16(&normalizer, &capture,
            ux_keyboard_capture_submit, 'a') || capture.count < 4u ||
        capture.events[capture.count - 2u].type != UX_EVENT_KEY ||
        !capture.events[capture.count - 2u].data.key.pressed ||
        capture.events[capture.count - 1u].data.key.pressed ||
        ux_win32_keyboard_submit_utf16(&normalizer, &capture,
            ux_keyboard_capture_submit, 0xdc00u) ||
        !ux_keyboard_capture_has_no_stuck_key(&capture, 2u)) return 1;
    ux_win32_keyboard_note_recovered_key(&normalizer, 0x41u);
    if (!ux_win32_keyboard_consume_duplicate_character(&normalizer, 'a') ||
        ux_win32_keyboard_consume_duplicate_character(&normalizer, 'a')) return 1;
    ux_win32_keyboard_note_recovered_key(&normalizer, 0x70u);
    if (ux_win32_keyboard_consume_duplicate_character(&normalizer, 'a')) return 1;
    ux_win32_keyboard_note_recovered_key(&normalizer, 0x41u);
    ux_win32_keyboard_release_recovered_key(&normalizer, 0x41u);
    if (ux_win32_keyboard_consume_duplicate_character(&normalizer, 'a')) return 1;
    puts("M5:T523:S7:UX-WIN32-KEYBOARD-OWNER:OK");
    return 0;
}
