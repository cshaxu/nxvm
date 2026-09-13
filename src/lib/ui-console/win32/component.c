#include "lib/types/types_interface.h"
#include "lib/types/win32/scalar.h"
#include "lib/types/win32/sync.h"
#include "lib/ui-console/console.h"

#include "lib/ui-base/input_interface.h"

#include "lib/types/win32/console.h"

typedef struct ui_console_win32_state {
    lib_win32_handle worker;
    ui_keyboard_normalizer keyboard;
    lib_win32_coord previous_mouse;
    int previous_mouse_valid;
} ui_console_win32_state;

static int ui_console_emit_normalized(void *context, const ui_input_event *event)
{
    ui_console *console = context;
    return console == LIB_NULL ? 0 : ui_component_emit(&console->base, event);
}

static lib_u8 ui_console_hotkey_modifiers(lib_u8 modifiers)
{
    lib_u8 result = 0u;
    if ((modifiers & LIB_CONSOLE_MODIFIER_CONTROL) != 0u)
        result |= UI_HOTKEY_MODIFIER_CONTROL;
    if ((modifiers & LIB_CONSOLE_MODIFIER_ALT) != 0u)
        result |= UI_HOTKEY_MODIFIER_ALT;
    if ((modifiers & LIB_CONSOLE_MODIFIER_SHIFT) != 0u)
        result |= UI_HOTKEY_MODIFIER_SHIFT;
    return result;
}

static void ui_console_receive_event(void *context,
    const lib_console_event *event)
{
    ui_console *console = (ui_console *)context;
    ui_console_win32_state *state;
    ui_input_event input = { 0 };

    if (console == LIB_NULL || event == LIB_NULL ||
        lib_atomic_i32_load_explicit(&console->base.stopping,
            LIB_MEMORY_ORDER_ACQUIRE) != 0 ||
        (state = (ui_console_win32_state *)console->worker_state) == LIB_NULL)
        return;
    if (event->kind == LIB_CONSOLE_EVENT_INPUT_RESET) {
        ui_hotkey_matcher_discard(&console->base.hotkey_matcher);
        lib_memory_set(&state->keyboard, 0, sizeof(state->keyboard));
        state->previous_mouse_valid = 0;
    } else if (event->kind == LIB_CONSOLE_EVENT_ACTIVATED) {
        ui_mailbox_wake_signal(ui_component_mailboxes_wake(&console->base.mailboxes));
    } else if (event->kind == LIB_CONSOLE_EVENT_IO_FAILURE) {
        ui_component_fail(&console->base, LIB_STATUS_IO_ERROR);
    } else if (event->kind == LIB_CONSOLE_EVENT_RAW_KEY) {
        const lib_console_raw_key *key = &event->value.raw_key;

        ui_keyboard_record record = {
            UI_KEYBOARD_COMBINED, key->scan_code, (lib_u16)key->key,
            (lib_u16)key->unicode,
            key->extended != LIB_FALSE ? UI_INPUT_FLAG_EXTENDED : 0u,
            ui_console_hotkey_modifiers(key->modifiers), key->pressed, key->repeat_count };
        (void)ui_keyboard_submit_record(&state->keyboard,
            &console->base.hotkey_matcher, console,
            ui_console_emit_normalized, &record);
    } else if (event->kind == LIB_CONSOLE_EVENT_RAW_MOUSE) {
        const lib_console_raw_mouse *mouse = &event->value.raw_mouse;

        input.type = UI_EVENT_MOUSE;
        input.data.mouse.relative = 1u;
        if (state->previous_mouse_valid) {
            input.data.mouse.delta_x = (mouse->delta_x - state->previous_mouse.X) * 8;
            input.data.mouse.delta_y = (mouse->delta_y - state->previous_mouse.Y) * 16;
        }
        state->previous_mouse.X = (lib_win32_short)mouse->delta_x;
        state->previous_mouse.Y = (lib_win32_short)mouse->delta_y;
        state->previous_mouse_valid = 1;
        input.data.mouse.buttons = (mouse->buttons & LIB_WIN32_FROM_LEFT_1ST_BUTTON_PRESSED) != 0u ?
            UI_MOUSE_BUTTON_LEFT : 0u;
        if ((mouse->buttons & LIB_WIN32_RIGHTMOST_BUTTON_PRESSED) != 0u)
            input.data.mouse.buttons |= UI_MOUSE_BUTTON_RIGHT;
        (void)ui_console_emit_normalized(console, &input);
    }
}

static lib_win32_dword LIB_WIN32_WINAPI ui_console_worker(void *opaque)
{
    ui_console *console = (ui_console *)opaque;
    lib_u32 generation = 0u;

    while (lib_atomic_i32_load_explicit(&console->base.stopping,
        LIB_MEMORY_ORDER_ACQUIRE) == 0) {
        ui_component_control control;
        ui_frame frame;
        ui_mailbox_wake_wait_result wake;

        wake = ui_mailbox_wake_wait(
            ui_component_mailboxes_wake(&console->base.mailboxes), LIB_UINT32_MAX);
        if (lib_atomic_i32_load_explicit(&console->base.stopping,
                LIB_MEMORY_ORDER_ACQUIRE) != 0) break;
        if (wake != UI_MAILBOX_WAKE_WAIT_WAKE) {
            ui_component_fail(&console->base, LIB_STATUS_IO_ERROR);
            break;
        }
        while (ui_component_mailboxes_take_control(&console->base.mailboxes, &control)) {
            if (control.kind == UI_COMPONENT_CONTROL_STOP) {
                goto retired;
            }
            /* ui-console has no title or mouse surface. Unsupported Window
             * control entries are intentionally consumed as no-ops. */
        }
        if (ui_component_mailboxes_capture_frame(&console->base.mailboxes,
                &generation, &frame)) {
            lib_status status = ui_console_publish_text_frame(console, &frame);
            if (status == LIB_STATUS_OK)
                ui_component_mailboxes_acknowledge_frame(&console->base.mailboxes,
                    generation);
            if (status != LIB_STATUS_OK && status != LIB_STATUS_NOT_CURRENT) {
                ui_component_fail(&console->base, status);
                break;
            }
        }
    }
retired:
    /* Detach waits for any in-flight callback on every exit, including a
     * failed wake. Retirement is the final input fact from this source. */
    lib_atomic_i32_store_explicit(&console->base.stopping, 1, LIB_MEMORY_ORDER_RELEASE);
    (void)lib_console_set_event_sink(console->logical_console, LIB_NULL, LIB_NULL);
    ui_component_retire(&console->base, LIB_STATUS_OK);
    return 0u;
}

lib_status ui_console_worker_start(ui_console *console)
{
    ui_console_win32_state *state;

    if (console == LIB_NULL || console->logical_console == LIB_NULL)
        return LIB_STATUS_INVALID_ARGUMENT;
    state = lib_allocate_zero(1u, sizeof(*state));
    if (state == LIB_NULL) return LIB_STATUS_NO_MEMORY;
    console->worker_state = state;
    /* Install before starting: an immediate worker failure must not be
     * followed by reattaching the retired source from this thread. */
    if (lib_console_set_event_sink(console->logical_console,
            ui_console_receive_event, console) != LIB_STATUS_OK) {
        console->worker_state = LIB_NULL;
        lib_release(state);
        return LIB_STATUS_INVALID_STATE;
    }
    state->worker = lib_win32_create_thread(LIB_NULL, 0u, ui_console_worker, console, 0u, LIB_NULL);
    if (state->worker == LIB_NULL) {
        (void)lib_console_set_event_sink(console->logical_console, LIB_NULL, LIB_NULL);
        console->worker_state = LIB_NULL;
        lib_release(state);
        return LIB_STATUS_NO_MEMORY;
    }
    return LIB_STATUS_OK;
}

void ui_console_worker_join(ui_console *console)
{
    ui_console_win32_state *state;

    if (console == LIB_NULL || (state = (ui_console_win32_state *)
            console->worker_state) == LIB_NULL) return;
    /* ui_component_destroy has queued STOP; wait for the Console worker to
     * consume it before detaching the event sink or releasing state. */
    (void)lib_win32_wait_for_single_object(state->worker, LIB_WIN32_INFINITE);
    lib_win32_close_handle(state->worker);
    console->worker_state = LIB_NULL;
    lib_release(state);
}
