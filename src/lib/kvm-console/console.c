#include "lib/kvm-console/console.h"

static lib_bool kvm_console_emit_normalized(void *context, const kvm_input_event *event)
{
    kvm_console *console = context;
    return console == LIB_NULL ? LIB_FALSE : kvm_component_emit(&console->base, event);
}

static lib_u8 kvm_console_hotkey_modifiers(lib_u8 modifiers)
{
    lib_u8 result = 0u;
    if ((modifiers & LIB_CONSOLE_MODIFIER_CONTROL) != 0u)
        result |= KVM_HOTKEY_MODIFIER_CONTROL;
    if ((modifiers & LIB_CONSOLE_MODIFIER_ALT) != 0u)
        result |= KVM_HOTKEY_MODIFIER_ALT;
    if ((modifiers & LIB_CONSOLE_MODIFIER_SHIFT) != 0u)
        result |= KVM_HOTKEY_MODIFIER_SHIFT;
    return result;
}

void kvm_console_receive_event(void *context,
    const lib_console_event *event)
{
    kvm_console *console = (kvm_console *)context;
    kvm_input_event input = { 0 };

    if (console == LIB_NULL || event == LIB_NULL ||
        lib_atomic_i32_load_explicit(&console->base.stopping,
            LIB_MEMORY_ORDER_ACQUIRE) != 0 ||
        console->worker_state == LIB_NULL)
        return;
    if (event->kind == LIB_CONSOLE_EVENT_INPUT_RESET) {
        kvm_hotkey_matcher_discard(&console->base.hotkey_matcher);
        lib_memory_set(&console->keyboard, 0, sizeof(console->keyboard));
        console->previous_mouse_valid = LIB_FALSE;
    } else if (event->kind == LIB_CONSOLE_EVENT_ACTIVATED) {
        lib_status status = kvm_component_mailboxes_notify(&console->base.mailboxes);
        if (status != LIB_STATUS_OK) kvm_component_fail(&console->base, status);
    } else if (event->kind == LIB_CONSOLE_EVENT_IO_FAILURE) {
        kvm_component_fail(&console->base, LIB_STATUS_IO_ERROR);
    } else if (event->kind == LIB_CONSOLE_EVENT_RAW_KEY) {
        const lib_console_raw_key *key = &event->value.raw_key;

        kvm_keyboard_record record = {
            KVM_KEYBOARD_COMBINED, key->scan_code, (lib_u16)key->key,
            (lib_u16)key->unicode,
            key->extended != LIB_FALSE ? KVM_INPUT_FLAG_EXTENDED : 0u,
            kvm_console_hotkey_modifiers(key->modifiers), key->pressed, key->repeat_count };
        (void)kvm_keyboard_submit_record(&console->keyboard,
            &console->base.hotkey_matcher, console,
            kvm_console_emit_normalized, &record);
    } else if (event->kind == LIB_CONSOLE_EVENT_RAW_MOUSE) {
        const lib_console_raw_mouse *mouse = &event->value.raw_mouse;

        input.type = KVM_EVENT_MOUSE;
        input.data.mouse.relative = 1u;
        if (console->previous_mouse_valid) {
            input.data.mouse.delta_x = (mouse->delta_x - console->previous_mouse_x) * 8;
            input.data.mouse.delta_y = (mouse->delta_y - console->previous_mouse_y) * 16;
        }
        console->previous_mouse_x = mouse->delta_x;
        console->previous_mouse_y = mouse->delta_y;
        console->previous_mouse_valid = LIB_TRUE;
        input.data.mouse.buttons = kvm_console_mouse_buttons(mouse->buttons);
        (void)kvm_console_emit_normalized(console, &input);
    }
}

static void kvm_console_component_dispose(kvm_component *base)
{
    kvm_console *console = (kvm_console *)base;
    if (console == LIB_NULL) return;
    kvm_component_mailboxes_destroy(&console->base.mailboxes);
    if (console->logical_console != LIB_NULL) {
        /* The worker (or failed start) already detached and drained its sink. */
        lib_console_release(console->logical_console);
    }
    lib_release(console);
}

static lib_status kvm_console_component_stop(kvm_component *base, lib_u32 timeout_ms)
{ return kvm_console_worker_join((kvm_console *)base, timeout_ms); }

lib_status kvm_console_create(kvm_console **out_console,
    const kvm_console_options *options)
{
    kvm_console *console;
    lib_status status;

    if (out_console == LIB_NULL) return LIB_STATUS_INVALID_ARGUMENT;
    *out_console = LIB_NULL;
    if (options == LIB_NULL ||
        options->input_sink == LIB_NULL || options->failure_sink == LIB_NULL)
        return LIB_STATUS_INVALID_ARGUMENT;
    console = lib_allocate_zero(1u, sizeof(*console));
    if (console == LIB_NULL) return LIB_STATUS_NO_MEMORY;
    status = kvm_component_initialize(&console->base, options,
        kvm_console_component_stop, kvm_console_component_dispose,
        &console->pending_frame, sizeof(console->pending_frame));
    if (status == LIB_STATUS_OK)
        status = kvm_component_mailboxes_select_notify(&console->base.mailboxes,
            LIB_NULL, LIB_NULL);
    if (status == LIB_STATUS_OK)
        status = lib_console_create(&console->logical_console);
    if (status == LIB_STATUS_OK) status = kvm_console_worker_start(console);
    if (status != LIB_STATUS_OK) {
        if (console->worker_state == LIB_NULL)
            kvm_console_component_dispose(&console->base);
        return status;
    }
    *out_console = console;
    return LIB_STATUS_OK;
}

lib_status kvm_console_publish_frame(kvm_console *console, const kvm_console_text_frame *frame)
{
    lib_status status;
    if (console == LIB_NULL) return LIB_STATUS_INVALID_ARGUMENT;
    status = kvm_console_text_frame_validate(frame);
    if (status != LIB_STATUS_OK) return status;
    return kvm_component_publish_frame(&console->base, frame, sizeof(*frame));
}

lib_status kvm_console_destroy(kvm_console *console)
{
    return console == LIB_NULL ? LIB_STATUS_OK : kvm_component_destroy(&console->base);
}

lib_console *kvm_console_get_console(const kvm_console *console)
{
    return console == LIB_NULL ? LIB_NULL : console->logical_console;
}

lib_status kvm_console_publish_text_frame(kvm_console *console,
    const kvm_console_text_frame *frame)
{
    lib_console_text_frame text_frame = { 0 };
    const kvm_text_frame *text;
    lib_size index;

    if (console == LIB_NULL || frame == LIB_NULL) return LIB_STATUS_INVALID_ARGUMENT;
    text = &frame->base;
    text_frame.columns = text->text_columns;
    text_frame.rows = text->text_rows;
    for (index = 0u; index < KVM_TEXT_COLUMNS * KVM_TEXT_ROWS; ++index) {
        const kvm_text_cell *cell = &text->cells[index];
        const lib_u16 *map = cell->glyph_bank != 0u ?
            frame->characters.secondary : frame->characters.primary;
        text_frame.text[index] = map[cell->glyph_index];
        text_frame.foreground[index] = cell->foreground;
        text_frame.background[index] = cell->background;
    }
    lib_memory_copy(text_frame.palette, text->text_palette, sizeof(text_frame.palette));
    text_frame.cursor_column = text->cursor_column;
    text_frame.cursor_row = text->cursor_row;
    text_frame.cursor_top = text->cursor_top;
    text_frame.cursor_bottom = text->cursor_bottom;
    text_frame.cursor_visible = text->cursor_visible;
    text_frame.cursor_phase = text->cursor_phase;
    text_frame.font_height = text->font_height ? text->font_height : 16u;
    if (text->cursor_bottom >= text->cursor_top) {
        if (text->cursor_top >= text_frame.font_height)
            text_frame.cursor_visible = 0u;
        if (text->cursor_bottom >= text_frame.font_height)
            text_frame.cursor_bottom = (lib_u8)(text_frame.font_height - 1u);
    }
    return lib_console_write_text_frame(console->logical_console, &text_frame);
}
