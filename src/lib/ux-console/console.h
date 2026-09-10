#ifndef UX_CONSOLE_H
#define UX_CONSOLE_H

#include "lib/ux-console/console_interface.h"
#include "lib/ux-base/component.h"

struct ux_console {
    ux_component base;
    lib_console *logical_console;
    void *native_state;
};

lib_status ux_console_native_start(ux_console *console);
void ux_console_native_stop(ux_console *console);

#endif
