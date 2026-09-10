#include "lib/ux-console/console.h"

#ifdef _WIN32
#include "lib/ux-base/win32/input.h"
#include "lib/ux-base/win32/mailbox_wake.h"

#include <windows.h>
#include <stdlib.h>
#include <string.h>

typedef struct ux_console_win32_state {
    HANDLE worker;
    COORD previous_mouse;
    int previous_mouse_valid;
} ux_console_win32_state;

static int ux_console_emit(ux_console *console, const ux_input_event *event)
{
    return console == LIB_NULL ? 0 : ux_component_emit(&console->base, event);
}

static int ux_console_emit_normalized(void *context, const ux_event *event)
{
    return ux_console_emit((ux_console *)context, event);
}

static lib_u8 ux_console_hotkey_modifiers(lib_u8 modifiers)
{
    lib_u8 result = 0u;
    if ((modifiers & LIB_CONSOLE_MODIFIER_CONTROL) != 0u)
        result |= UX_HOTKEY_MODIFIER_CONTROL;
    if ((modifiers & LIB_CONSOLE_MODIFIER_ALT) != 0u)
        result |= UX_HOTKEY_MODIFIER_ALT;
    if ((modifiers & LIB_CONSOLE_MODIFIER_SHIFT) != 0u)
        result |= UX_HOTKEY_MODIFIER_SHIFT;
    return result;
}

static void ux_console_receive_event(void *context,
    const lib_console_event *event)
{
    ux_console *console = (ux_console *)context;
    ux_console_win32_state *state;
    ux_input_event input = { 0 };

    if (console == LIB_NULL || event == LIB_NULL ||
        atomic_load_explicit(&console->base.stopping, memory_order_acquire) != 0 ||
        (state = (ux_console_win32_state *)console->native_state) == LIB_NULL)
        return;
    if (event->kind == LIB_CONSOLE_EVENT_RAW_KEY) {
        const lib_console_raw_key *key = &event->value.raw_key;

        /* Console INPUT_RECORD packets and Window messages must take the
         * same normalization path.  In particular, RDP can provide a
         * virtual key while omitting its physical scan code; passing zero to
         * zero physical scan code would drop the first post-handoff key in a
         * consumer key mapper.
         * ux-base recovers the scan code through the active Win32 layout. */
        (void)ux_win32_keyboard_submit_transition(console,
            ux_console_emit_normalized,
            (WORD)(key->scan_code | (key->extended != LIB_FALSE ?
                0x0100u : 0u)), (WORD)key->key,
            key->extended != LIB_FALSE ? ENHANCED_KEY : 0u,
            ux_console_hotkey_modifiers(key->modifiers),
            key->pressed != LIB_FALSE);
    } else if (event->kind == LIB_CONSOLE_EVENT_RAW_MOUSE) {
        const lib_console_raw_mouse *mouse = &event->value.raw_mouse;

        input.type = UX_EVENT_MOUSE;
        input.data.mouse.relative = 1u;
        if (state->previous_mouse_valid) {
            input.data.mouse.delta_x = (mouse->delta_x - state->previous_mouse.X) * 8;
            input.data.mouse.delta_y = (mouse->delta_y - state->previous_mouse.Y) * 16;
        }
        state->previous_mouse.X = (SHORT)mouse->delta_x;
        state->previous_mouse.Y = (SHORT)mouse->delta_y;
        state->previous_mouse_valid = 1;
        input.data.mouse.buttons = (mouse->buttons & FROM_LEFT_1ST_BUTTON_PRESSED) != 0u ?
            UX_MOUSE_BUTTON_LEFT : 0u;
        if ((mouse->buttons & RIGHTMOST_BUTTON_PRESSED) != 0u)
            input.data.mouse.buttons |= UX_MOUSE_BUTTON_RIGHT;
        (void)ux_console_emit(console, &input);
    }
}

static void ux_console_publish_text_frame(ux_console *console,
    const ux_frame *frame)
{
    lib_console_text_frame text_frame = { 0 };

    if (console == LIB_NULL || frame == LIB_NULL || frame->graphics != 0u) return;
    text_frame.columns = frame->text_columns;
    text_frame.rows = frame->text_rows;
    memcpy(text_frame.text, frame->text, sizeof(text_frame.text));
    memcpy(text_frame.attributes, frame->attributes, sizeof(text_frame.attributes));
    memcpy(text_frame.palette, frame->text_palette, sizeof(text_frame.palette));
    text_frame.cursor_column = frame->cursor_column;
    text_frame.cursor_row = frame->cursor_row;
    text_frame.cursor_top = frame->cursor_top;
    text_frame.cursor_bottom = frame->cursor_bottom;
    text_frame.cursor_visible = frame->cursor_visible;
    text_frame.cursor_phase = frame->cursor_phase;
    text_frame.font_height = frame->font_height;
    (void)lib_console_write_text_frame(console->logical_console, &text_frame);
}

static DWORD WINAPI ux_console_worker(void *opaque)
{
    ux_console *console = (ux_console *)opaque;
    lib_u32 generation = 0u;

    while (atomic_load_explicit(&console->base.stopping, memory_order_acquire) == 0) {
        ux_component_control control;
        ux_frame frame;
        HANDLE wake = ux_win32_mailbox_wait_handle(
            ux_component_mailboxes_wake(&console->base.mailboxes));

        if (wake == NULL || WaitForSingleObject(wake, INFINITE) != WAIT_OBJECT_0)
            break;
        while (ux_component_mailboxes_take_control(&console->base.mailboxes, &control)) {
            if (control.kind == UX_COMPONENT_CONTROL_STOP) {
                /* Detach waits for any in-flight native callback.  Retirement
                 * is therefore the final input fact from this source. */
                (void)lib_console_set_event_sink(console->logical_console,
                    LIB_NULL, LIB_NULL);
                ux_component_emit_source_retired(&console->base);
                return 0u;
            }
            /* ux-console has no title or mouse surface. Unsupported Window
             * control entries are intentionally consumed as no-ops. */
        }
        if (ux_component_mailboxes_capture_frame(&console->base.mailboxes,
                &generation, &frame)) ux_console_publish_text_frame(console, &frame);
    }
    return 0u;
}

lib_status ux_console_native_start(ux_console *console)
{
    ux_console_win32_state *state;

    if (console == LIB_NULL || console->logical_console == LIB_NULL)
        return LIB_STATUS_INVALID_ARGUMENT;
    state = calloc(1u, sizeof(*state));
    if (state == LIB_NULL) return LIB_STATUS_NO_MEMORY;
    console->native_state = state;
    state->worker = CreateThread(NULL, 0u, ux_console_worker, console, 0u, NULL);
    if (state->worker == NULL) {
        console->native_state = LIB_NULL;
        free(state);
        return LIB_STATUS_NO_MEMORY;
    }
    if (lib_console_set_event_sink(console->logical_console,
            ux_console_receive_event, console) != LIB_STATUS_OK) {
        atomic_store_explicit(&console->base.stopping, 1, memory_order_release);
        ux_mailbox_wake_signal(ux_component_mailboxes_wake(&console->base.mailboxes));
        (void)WaitForSingleObject(state->worker, INFINITE);
        CloseHandle(state->worker);
        console->native_state = LIB_NULL;
        free(state);
        return LIB_STATUS_INVALID_STATE;
    }
    return LIB_STATUS_OK;
}

void ux_console_native_stop(ux_console *console)
{
    ux_console_win32_state *state;

    if (console == LIB_NULL || (state = (ux_console_win32_state *)
            console->native_state) == LIB_NULL) return;
    /* ux_component_destroy has queued STOP; wait for the Console worker to
     * consume it before detaching the event sink or releasing state. */
    (void)WaitForSingleObject(state->worker, INFINITE);
    CloseHandle(state->worker);
    console->native_state = LIB_NULL;
    free(state);
}
#endif
