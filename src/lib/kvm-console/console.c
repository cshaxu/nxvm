#include "lib/kvm-console/console.h"

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
        kvm_console_component_stop, kvm_console_component_dispose);
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

lib_status kvm_console_publish_frame(kvm_console *console, const kvm_frame *frame)
{
    return console == LIB_NULL ? LIB_STATUS_INVALID_ARGUMENT :
        kvm_component_publish_frame(&console->base, frame);
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
    const kvm_frame *frame)
{
    lib_console_text_frame text_frame = { 0 };

    if (console == LIB_NULL || frame == LIB_NULL) return LIB_STATUS_INVALID_ARGUMENT;
    if (frame->graphics != 0u) return LIB_STATUS_OK;
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
