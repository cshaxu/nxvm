#include "type.h"

#include "vm/product/console_host.h"

struct vm_product_console_host {
    C_VOID *line_context;
    void (*line_sink)(C_VOID *context, const C_CHAR *text);
};

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
    *out_host = host;
    return TYPE_STATUS_OK;
}

C_VOID vm_product_console_host_destroy(vm_product_console_host *host)
{
    STD_FREE(host);
}

type_status vm_product_console_host_request_line(vm_product_console_host *host)
{
    C_CHAR line[256];

    if (host == STD_NULL || host->line_sink == STD_NULL ||
        STD_FGETS(line, sizeof(line), STD_STDIN) == STD_NULL)
        return TYPE_STATUS_INVALID_STATE;
    host->line_sink(host->line_context, line);
    return TYPE_STATUS_OK;
}

type_status vm_product_console_host_write(vm_product_console_host *host,
    const C_CHAR *text)
{
    (C_VOID)host;
    return text == STD_NULL ? TYPE_STATUS_INVALID_ARGUMENT : TYPE_STATUS_OK;
}

const vm_platform_console_binding *vm_product_console_host_binding(
    const vm_product_console_host *host)
{
    (C_VOID)host;
    return STD_NULL;
}
