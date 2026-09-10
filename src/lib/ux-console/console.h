#ifndef UX_CONSOLE_H
#define UX_CONSOLE_H

#include "lib/base/console.h"
#include "lib/ux-base/component.h"

typedef struct ux_console ux_console;

typedef ux_component_options ux_console_options;

lib_status ux_console_create(ux_console **out_console,
    const ux_console_options *options);
lib_status ux_console_publish_frame(ux_console *console, const ux_frame *frame);
void ux_console_destroy(ux_console *console);
/* Borrowed logical Console object. The application passes it to host for
 * Current Console registration before it permits raw VM input. */
lib_console *ux_console_get_console(const ux_console *console);

#endif
