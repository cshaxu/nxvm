#ifndef UI_CONSOLE_H
#define UI_CONSOLE_H

#include "lib/ui-console/console_interface.h"
#include "lib/ui-base/component.h"

struct ui_console {
    ui_component base;
    lib_console *logical_console;
    void *native_state;
};

lib_status ui_console_native_start(ui_console *console);
void ui_console_native_stop(ui_console *console);

#endif
