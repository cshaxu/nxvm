#ifndef UI_CONSOLE_H
#define UI_CONSOLE_H

#include "lib/ui-console/console_interface.h"
#include "lib/ui-base/worker_interface.h"

struct ui_console {
    ui_component base;
    lib_console *logical_console;
    void *worker_state;
};

lib_status ui_console_publish_text_frame(ui_console *console, const ui_frame *frame);

lib_status ui_console_worker_start(ui_console *console);
void ui_console_worker_join(ui_console *console);

#endif
