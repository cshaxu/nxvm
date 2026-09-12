#ifndef COMMON_UI_CONSOLE_HOST_H
#define COMMON_UI_CONSOLE_HOST_H

#include "common/ui/ui_interface.h"
#include "lib/console/console_interface.h"

typedef struct common_ui_console_host common_ui_console_host;

lib_status common_ui_console_host_create(common_ui_console_host **out_host,
    void *line_context, common_ui_console_line_sink line_sink);
void common_ui_console_host_destroy(common_ui_console_host *host);
lib_status common_ui_console_host_request_line(common_ui_console_host *host);
lib_status common_ui_console_host_write(common_ui_console_host *host, const char *text);
lib_status common_ui_console_host_claim_guest(common_ui_console_host *host,
    lib_console *guest_console);
lib_status common_ui_console_host_release_guest(common_ui_console_host *host,
    lib_console *guest_console);

#endif
