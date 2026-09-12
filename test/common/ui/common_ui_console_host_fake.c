#include "type.h"

#include "common/ui/console_host.h"

struct common_ui_console_host {
    void *line_context;
    common_ui_console_line_sink line_sink;
};

lib_status common_ui_console_host_create(common_ui_console_host **out_host,
    void *line_context, common_ui_console_line_sink line_sink)
{
    common_ui_console_host *host;

    if (out_host == LIB_NULL || line_sink == LIB_NULL) return LIB_STATUS_INVALID_ARGUMENT;
    *out_host = LIB_NULL;
    host = lib_allocate_zero(1u, sizeof(*host));
    if (host == LIB_NULL) return LIB_STATUS_NO_MEMORY;
    host->line_context = line_context;
    host->line_sink = line_sink;
    *out_host = host;
    return LIB_STATUS_OK;
}

void common_ui_console_host_destroy(common_ui_console_host *host)
{ lib_release(host); }

lib_status common_ui_console_host_request_line(common_ui_console_host *host)
{
    C_CHAR line[256];

    if (host == LIB_NULL || host->line_sink == LIB_NULL ||
        STD_FGETS(line, sizeof(line), STD_STDIN) == STD_NULL)
        return LIB_STATUS_INVALID_STATE;
    return host->line_sink(host->line_context, line);
}

lib_status common_ui_console_host_write(common_ui_console_host *host, const char *text)
{
    (void)host;
    return text == LIB_NULL ? LIB_STATUS_INVALID_ARGUMENT :
        STD_FPUTS(text, STD_STDOUT) < 0 ? LIB_STATUS_IO_ERROR : LIB_STATUS_OK;
}

lib_status common_ui_console_host_claim_guest(common_ui_console_host *host,
    lib_console *guest_console)
{
    return host == LIB_NULL || guest_console == LIB_NULL ? LIB_STATUS_INVALID_ARGUMENT :
        LIB_STATUS_OK;
}

lib_status common_ui_console_host_release_guest(common_ui_console_host *host,
    lib_console *guest_console)
{
    return host == LIB_NULL || guest_console == LIB_NULL ? LIB_STATUS_INVALID_ARGUMENT :
        LIB_STATUS_OK;
}
