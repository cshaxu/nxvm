#include "lib/ui-console/console.h"

#include <stdlib.h>

static void ui_console_dispose(ui_console *console)
{
    if (console == LIB_NULL) return;
    if (console->logical_console != LIB_NULL) {
        (void)lib_console_set_event_sink(console->logical_console, LIB_NULL,
            LIB_NULL);
        lib_console_release(console->logical_console);
    }
    ui_component_mailboxes_destroy(&console->base.mailboxes);
    free(console);
}

static void ui_console_component_stop(ui_component *base)
{ ui_console_native_stop((ui_console *)base); }

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
    console = calloc(1u, sizeof(*console));
    if (console == LIB_NULL) return LIB_STATUS_NO_MEMORY;
    status = ui_component_initialize(&console->base, options,
        ui_console_component_stop, ui_console_component_dispose);
    if (status == LIB_STATUS_OK)
        status = lib_console_create(&console->logical_console);
    if (status == LIB_STATUS_OK) status = ui_console_native_start(console);
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
