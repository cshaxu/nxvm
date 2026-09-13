#ifndef UI_WINDOW_H
#define UI_WINDOW_H

#include "lib/ui-window/window_interface.h"
#include "lib/ui-base/worker_interface.h"

struct ui_window {
    ui_component base;
    char initial_title[UI_WINDOW_TITLE_CAPACITY];
    lib_bool initial_frozen;
    void *worker_state;
};

lib_status ui_window_worker_start(ui_window *window);
void ui_window_worker_join(ui_window *window);

#endif
