#include "common/ui/console_host.h"

#include "lib/host/console_interface.h"

struct common_ui_console_host {
    lib_console *monitor;
    host_console_broker *broker;
    void *line_context;
    common_ui_console_line_sink line_sink;
};

static void common_ui_console_host_event(void *opaque,
    const lib_console_event *event)
{
    common_ui_console_host *host = opaque;

    if (host == LIB_NULL || event == LIB_NULL ||
        event->kind != LIB_CONSOLE_EVENT_COOKED_LINE ||
        host->line_sink == LIB_NULL) return;
    (void)host->line_sink(host->line_context, event->value.line.text);
}

lib_status common_ui_console_host_claim_guest(common_ui_console_host *host,
    lib_console *guest_console)
{
    return host == LIB_NULL || guest_console == LIB_NULL ? LIB_STATUS_INVALID_ARGUMENT :
        host_console_broker_replace(host->broker, host->monitor, guest_console,
            HOST_CONSOLE_RAW_EVENTS);
}

lib_status common_ui_console_host_release_guest(common_ui_console_host *host,
    lib_console *guest_console)
{
    return host == LIB_NULL || guest_console == LIB_NULL ? LIB_STATUS_INVALID_ARGUMENT :
        host_console_broker_replace(host->broker, guest_console, host->monitor,
            HOST_CONSOLE_COOKED_LINES);
}

lib_status common_ui_console_host_create(common_ui_console_host **out_host,
    void *line_context, common_ui_console_line_sink line_sink)
{
    common_ui_console_host *host;
    lib_status status;

    if (out_host == LIB_NULL || line_sink == LIB_NULL) return LIB_STATUS_INVALID_ARGUMENT;
    *out_host = LIB_NULL;
    host = lib_allocate_zero(1u, sizeof(*host));
    if (host == LIB_NULL) return LIB_STATUS_NO_MEMORY;
    host->line_context = line_context;
    host->line_sink = line_sink;
    status = lib_console_create(&host->monitor);
    if (status == LIB_STATUS_OK)
        status = lib_console_set_event_sink(host->monitor, common_ui_console_host_event,
            host);
    if (status == LIB_STATUS_OK)
        status = host_console_broker_create(&host->broker, host->monitor,
            HOST_CONSOLE_COOKED_LINES);
    if (status != LIB_STATUS_OK) {
        common_ui_console_host_destroy(host);
        return status;
    }
    *out_host = host;
    return LIB_STATUS_OK;
}

void common_ui_console_host_destroy(common_ui_console_host *host)
{
    if (host == LIB_NULL) return;
    host_console_broker_destroy(host->broker);
    if (host->monitor != LIB_NULL) {
        (void)lib_console_set_event_sink(host->monitor, LIB_NULL, LIB_NULL);
        lib_console_release(host->monitor);
    }
    lib_release(host);
}

lib_status common_ui_console_host_request_line(common_ui_console_host *host)
{
    return host == LIB_NULL ? LIB_STATUS_INVALID_ARGUMENT :
        host_console_broker_request_cooked_line(host->broker, host->monitor);
}

lib_status common_ui_console_host_write(common_ui_console_host *host, const char *text)
{
    return host == LIB_NULL || text == LIB_NULL ? LIB_STATUS_INVALID_ARGUMENT :
        lib_console_write_text(host->monitor, text, lib_text_length(text));
}
