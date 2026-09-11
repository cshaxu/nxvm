#ifndef UI_CONSOLE_INTERFACE_H
#define UI_CONSOLE_INTERFACE_H

#include "lib/console/console_interface.h"
#include "lib/ui-base/component_interface.h"

typedef struct ui_console ui_console;

typedef ui_component_options ui_console_options;

lib_status ui_console_create(ui_console **out_console,
    const ui_console_options *options);
lib_status ui_console_publish_frame(ui_console *console, const ui_frame *frame);
void ui_console_destroy(ui_console *console);
/* Borrowed logical Console object. The application passes it to host for
 * Current Console registration before it permits raw input. */
lib_console *ui_console_get_console(const ui_console *console);

#endif
