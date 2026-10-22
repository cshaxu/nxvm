#include "lib/kvm-console/console.h"

static lib_bool kvm_console_emit_normalized(void *context, const kvm_input_event *event)
{
    kvm_console *console = context;
    return console == LIB_NULL ? LIB_FALSE : kvm_component_emit(&console->base, event);
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
        if (console->previous_mouse_valid) {
            input.type = KVM_EVENT_MOUSE;
            input.data.mouse.relative = LIB_TRUE;
            (void)kvm_console_emit_normalized(console, &input);
        }
        if (!kvm_component_reset_input(&console->base)) return;
        console->previous_mouse_valid = LIB_FALSE;
    } else if (event->kind == LIB_CONSOLE_EVENT_ACTIVATED) {
        kvm_component_mailboxes_redraw_frame(&console->base.mailboxes);
        lib_status status = kvm_component_mailboxes_notify(&console->base.mailboxes);
        if (status != LIB_STATUS_OK) kvm_component_fail(&console->base, status);
    } else if (event->kind == LIB_CONSOLE_EVENT_IO_FAILURE) {
        kvm_component_fail(&console->base, LIB_STATUS_IO_ERROR);
    } else if (event->kind == LIB_CONSOLE_EVENT_RAW_KEY) {
        (void)kvm_keyboard_submit_input(&console->base.hotkey_matcher, console,
            kvm_console_emit_normalized, &event->value.raw_key);
    } else if (event->kind == LIB_CONSOLE_EVENT_RAW_MOUSE) {
        const lib_console_raw_mouse *mouse = &event->value.raw_mouse;

        input.type = KVM_EVENT_MOUSE;
        input.data.mouse.relative = LIB_TRUE;
        if (console->previous_mouse_valid) {
            input.data.mouse.delta_x = (mouse->column - console->previous_mouse_x) * 8;
            input.data.mouse.delta_y = (mouse->row - console->previous_mouse_y) * 16;
        }
        console->previous_mouse_x = mouse->column;
        console->previous_mouse_y = mouse->row;
        console->previous_mouse_valid = LIB_TRUE;
        input.data.mouse.buttons = mouse->buttons;
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
    lib_release(console->mapped_cells);
    lib_release(console);
}

static lib_status kvm_console_component_stop(kvm_component *base)
{ return kvm_console_worker_join((kvm_console *)base); }

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
        kvm_console_component_stop, kvm_console_component_dispose);
    if (status == LIB_STATUS_OK)
        status = kvm_component_mailboxes_select_notify(&console->base.mailboxes,
            LIB_NULL, LIB_NULL);
    if (status == LIB_STATUS_OK)
        status = lib_console_create(&console->logical_console);
    if (status == LIB_STATUS_OK) status = kvm_console_worker_start(console);
    if (status != LIB_STATUS_OK) {
        kvm_console_component_dispose(&console->base);
        return status;
    }
    *out_console = console;
    return LIB_STATUS_OK;
}

lib_status kvm_console_publish_frame(kvm_console *console, kvm_frame_exchange *frames)
{
    if (console == LIB_NULL || frames == LIB_NULL) return LIB_STATUS_INVALID_ARGUMENT;
    return kvm_component_publish_frame(&console->base, frames);
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
    const kvm_console_text_frame *frame, const kvm_console_text_frame *previous)
{
    lib_console_text_frame text_frame = { 0 };
    const kvm_text_frame *text;
    lib_u32 top = 0u, bottom = 0u;
    kvm_text_cursor_span spans[2];
    lib_u32 height;
    lib_size count;
    lib_u16 *characters;
    lib_u8 *foreground, *background, *underline;
    lib_status status;

    if (console == LIB_NULL || frame == LIB_NULL) return LIB_STATUS_INVALID_ARGUMENT;
    status = kvm_console_text_frame_validate(frame);
    if (status != LIB_STATUS_OK) return status;
    text = &frame->base;
    count = (lib_size)text->text_columns * text->text_rows;
    if (count > LIB_SIZE_MAX / (sizeof(lib_u16) + 3u)) return LIB_STATUS_INVALID_ARGUMENT;
    if (count != console->mapped_count) {
        void *mapped = lib_allocate(count * (sizeof(lib_u16) + 3u));
        if (mapped == LIB_NULL) return LIB_STATUS_NO_MEMORY;
        lib_release(console->mapped_cells);
        console->mapped_cells = mapped;
        console->mapped_count = count;
    }
    characters = console->mapped_cells;
    foreground = (lib_u8 *)(characters + count);
    background = foreground + count;
    underline = background + count;
    text_frame.text = characters;
    text_frame.foreground = foreground;
    text_frame.background = background;
    text_frame.underline = underline;
    text_frame.stride = text->text_columns;
    text_frame.capacity = count;
    height = text->font_height ? text->font_height : KVM_TEXT_FONT_DEFAULT_HEIGHT;
    text_frame.columns = text->text_columns;
    text_frame.rows = text->text_rows;
    text_frame.cells_unchanged = previous != LIB_NULL &&
        previous->base.text_columns == text->text_columns &&
        previous->base.text_rows == text->text_rows;
    for (lib_size row = 0u; row < text->text_rows; ++row) {
        for (lib_size column = 0u; column < text->text_columns; ++column) {
            lib_size index = row * text_frame.stride + column;
            const kvm_text_cell *cell = &text->cells[row * text->stride + column];
            const lib_u16 *map = cell->glyph_bank != 0u ?
                frame->characters->secondary : frame->characters->primary;
            characters[index] = map[cell->glyph_index];
            foreground[index] = cell->foreground;
            background[index] = cell->background;
            underline[index] = cell->underline && text->underline_row < height;
            if (text_frame.cells_unchanged) {
                const kvm_text_cell *old = &previous->base.cells[row * previous->base.stride + column];
                const lib_u16 *old_map = old->glyph_bank != 0u ?
                    previous->characters->secondary : previous->characters->primary;
                text_frame.cells_unchanged = old_map[old->glyph_index] == text_frame.text[index] &&
                    old->foreground == cell->foreground && old->background == cell->background &&
                    (old->underline && previous->base.underline_row <
                        (previous->base.font_height ? previous->base.font_height : KVM_TEXT_FONT_DEFAULT_HEIGHT)) ==
                        text_frame.underline[index];
            }
        }
    }
    lib_memory_copy(text_frame.palette, text->text_palette, sizeof(text_frame.palette));
    text_frame.cursor_column = text->cursor_column;
    text_frame.cursor_row = text->cursor_row;
    /* Native Console owns blinking; source phase cannot be reproduced exactly. */
    text_frame.cursor_visible = kvm_text_cursor_shape(text,
        &text_frame.font_height, spans) != 0u;
    /* Native Console approximates the first (topmost) resolved span by height;
     * it cannot represent the gap or scanline origin of a split cursor. */
    if (text_frame.cursor_visible) {
        top = spans[0].start;
        bottom = top + spans[0].height;
    }
    text_frame.cursor_phase = LIB_TRUE;
    /* The native contract represents a full cell by an inverted range, even
     * for logical fonts taller than its byte-sized cursor coordinates. */
    text_frame.cursor_top = bottom == text_frame.font_height && top == 0u ? 1u : (lib_u8)top;
    text_frame.cursor_bottom = bottom == 0u || (bottom == text_frame.font_height && top == 0u) ? 0u : (lib_u8)(bottom - 1u);
    return lib_console_write_text_frame(console->logical_console, &text_frame);
}
