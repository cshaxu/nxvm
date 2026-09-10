#include "lib/ux-console/internal.h"

static void ux_console_dispose(ux_console *console)
{
    if (console == LIB_NULL) return;
    if (console->logical_console != LIB_NULL) {
        (void)lib_console_set_event_sink(console->logical_console, LIB_NULL,
            LIB_NULL);
        lib_console_release(console->logical_console);
    }
    ux_component_mailboxes_destroy(&console->base.mailboxes);
    free(console);
}

static void ux_console_component_stop(ux_component *base)
{ ux_console_native_stop((ux_console *)base); }

static void ux_console_component_dispose(ux_component *base)
{ ux_console_dispose((ux_console *)base); }

lib_status ux_console_create(ux_console **out_console,
    const ux_console_options *options)
{
    ux_console *console;
    lib_status status;

    if (out_console == LIB_NULL || options == LIB_NULL ||
        options->input_sink == LIB_NULL || options->failure_sink == LIB_NULL)
        return LIB_STATUS_INVALID_ARGUMENT;
    *out_console = LIB_NULL;
    console = calloc(1u, sizeof(*console));
    if (console == LIB_NULL) return LIB_STATUS_NO_MEMORY;
    status = ux_component_initialize(&console->base, options,
        ux_console_component_stop, ux_console_component_dispose);
    if (status == LIB_STATUS_OK)
        status = lib_console_create(&console->logical_console);
    if (status == LIB_STATUS_OK) status = ux_console_native_start(console);
    if (status != LIB_STATUS_OK) {
        ux_console_dispose(console);
        return status;
    }
    *out_console = console;
    return LIB_STATUS_OK;
}

lib_status ux_console_publish_frame(ux_console *console, const ux_frame *frame)
{
    return console == LIB_NULL ? LIB_STATUS_INVALID_ARGUMENT :
        ux_component_publish_frame(&console->base, frame);
}

void ux_console_destroy(ux_console *console)
{
    if (console != LIB_NULL) ux_component_destroy(&console->base);
}

lib_console *ux_console_get_console(const ux_console *console)
{
    return console == LIB_NULL ? LIB_NULL : console->logical_console;
}
