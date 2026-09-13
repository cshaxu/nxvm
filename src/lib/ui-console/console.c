#include "lib/ui-console/console.h"

static void ui_console_dispose(ui_console *console)
{
    if (console == LIB_NULL) return;
    if (console->logical_console != LIB_NULL) {
        /* The worker (or failed start) already detached and drained its sink. */
        lib_console_release(console->logical_console);
    }
    ui_component_mailboxes_destroy(&console->base.mailboxes);
    lib_release(console);
}

static void ui_console_component_stop(ui_component *base)
{ ui_console_worker_join((ui_console *)base); }

static void ui_console_component_dispose(ui_component *base)
{ ui_console_dispose((ui_console *)base); }

lib_status ui_console_create(ui_console **out_console,
    const ui_console_options *options)
{
    ui_console *console;
    lib_status status;

    if (out_console == LIB_NULL || options == LIB_NULL ||
        options->input_sink == LIB_NULL || options->failure_sink == LIB_NULL)
        return LIB_STATUS_INVALID_ARGUMENT;
    *out_console = LIB_NULL;
    console = lib_allocate_zero(1u, sizeof(*console));
    if (console == LIB_NULL) return LIB_STATUS_NO_MEMORY;
    status = ui_component_initialize(&console->base, options,
        ui_console_component_stop, ui_console_component_dispose);
    if (status == LIB_STATUS_OK)
        status = lib_console_create(&console->logical_console);
    if (status == LIB_STATUS_OK) status = ui_console_worker_start(console);
    if (status != LIB_STATUS_OK) {
        ui_console_dispose(console);
        return status;
    }
    *out_console = console;
    return LIB_STATUS_OK;
}

lib_status ui_console_publish_frame(ui_console *console, const ui_frame *frame)
{
    return console == LIB_NULL ? LIB_STATUS_INVALID_ARGUMENT :
        ui_component_publish_frame(&console->base, frame);
}

void ui_console_destroy(ui_console *console)
{
    if (console != LIB_NULL) ui_component_destroy(&console->base);
}

lib_console *ui_console_get_console(const ui_console *console)
{
    return console == LIB_NULL ? LIB_NULL : console->logical_console;
}

lib_status ui_console_publish_text_frame(ui_console *console,
    const ui_frame *frame)
{
    lib_console_text_frame text_frame = { 0 };

    if (console == LIB_NULL || frame == LIB_NULL || frame->graphics != 0u)
        return LIB_STATUS_OK;
    text_frame.columns = frame->text_columns;
    text_frame.rows = frame->text_rows;
    lib_memory_copy(text_frame.text, frame->text, sizeof(text_frame.text));
    lib_memory_copy(text_frame.attributes, frame->attributes, sizeof(text_frame.attributes));
    lib_memory_copy(text_frame.palette, frame->text_palette, sizeof(text_frame.palette));
    text_frame.cursor_column = frame->cursor_column;
    text_frame.cursor_row = frame->cursor_row;
    text_frame.cursor_top = frame->cursor_top;
    text_frame.cursor_bottom = frame->cursor_bottom;
    text_frame.cursor_visible = frame->cursor_visible;
    text_frame.cursor_phase = frame->cursor_phase;
    text_frame.font_height = frame->font_height;
    return lib_console_write_text_frame(console->logical_console, &text_frame);
}
