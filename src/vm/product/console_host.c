#include "type.h"

#include "lib/base/console_interface.h"
#include "lib/host/console_interface.h"
#include "vm/product/console_host.h"

struct vm_product_console_host {
    lib_console *monitor;
    host_console_broker *broker;
    C_VOID *line_context;
    void (*line_sink)(C_VOID *context, const C_CHAR *text);
};

static C_VOID vm_product_console_host_event(C_VOID *opaque,
    const lib_console_event *event)
{
    vm_product_console_host *host = opaque;

    if (host == STD_NULL || event == STD_NULL ||
        event->kind != LIB_CONSOLE_EVENT_COOKED_LINE ||
        host->line_sink == STD_NULL) return;
    host->line_sink(host->line_context, event->value.line.text);
}

type_status vm_product_console_host_claim_guest(vm_product_console_host *host,
    lib_console *guest_console)
{
    if (host == STD_NULL || guest_console == STD_NULL)
        return TYPE_STATUS_INVALID_ARGUMENT;
    return (type_status)host_console_broker_replace(host->broker, host->monitor,
        guest_console, HOST_CONSOLE_RAW_EVENTS);
}

type_status vm_product_console_host_release_guest(vm_product_console_host *host,
    lib_console *guest_console)
{
    if (host == STD_NULL || guest_console == STD_NULL)
        return TYPE_STATUS_INVALID_ARGUMENT;
    return (type_status)host_console_broker_replace(host->broker, guest_console,
        host->monitor, HOST_CONSOLE_COOKED_LINES);
}

type_status vm_product_console_host_create(vm_product_console_host **out_host,
    C_VOID *line_context, void (*line_sink)(C_VOID *context,
        const C_CHAR *text))
{
    vm_product_console_host *host;

    if (out_host == STD_NULL || line_sink == STD_NULL)
        return TYPE_STATUS_INVALID_ARGUMENT;
    *out_host = STD_NULL;
    host = STD_CALLOC(1u, sizeof(*host));
    if (host == STD_NULL) return TYPE_STATUS_NO_MEMORY;
    host->line_context = line_context;
    host->line_sink = line_sink;
    if (lib_console_create(&host->monitor) != LIB_STATUS_OK ||
        lib_console_set_event_sink(host->monitor, vm_product_console_host_event,
            host) != LIB_STATUS_OK ||
        host_console_broker_create(&host->broker, host->monitor,
            HOST_CONSOLE_COOKED_LINES) != LIB_STATUS_OK) {
        vm_product_console_host_destroy(host);
        return TYPE_STATUS_INVALID_STATE;
    }
    *out_host = host;
    return TYPE_STATUS_OK;
}

C_VOID vm_product_console_host_destroy(vm_product_console_host *host)
{
    if (host == STD_NULL) return;
    host_console_broker_destroy(host->broker);
    if (host->monitor != STD_NULL) {
        (C_VOID)lib_console_set_event_sink(host->monitor, STD_NULL, STD_NULL);
        lib_console_release(host->monitor);
    }
    STD_FREE(host);
}

type_status vm_product_console_host_request_line(vm_product_console_host *host)
{
    return host == STD_NULL ? TYPE_STATUS_INVALID_ARGUMENT :
        (type_status)host_console_broker_request_cooked_line(host->broker,
            host->monitor);
}

type_status vm_product_console_host_write(vm_product_console_host *host,
    const C_CHAR *text)
{
    return host == STD_NULL || text == STD_NULL ? TYPE_STATUS_INVALID_ARGUMENT :
        (type_status)lib_console_write_text(host->monitor, text, STD_STRLEN(text));
}
