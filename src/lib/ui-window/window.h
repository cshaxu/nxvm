#ifndef UI_WINDOW_H
#define UI_WINDOW_H

#include "lib/ui-window/window_interface.h"
#include "lib/ui-base/component.h"

struct ui_window {
    ui_component base;
    char initial_title[UI_WINDOW_TITLE_CAPACITY];
    lib_bool initial_frozen;
    void *native_state;
};

lib_status ui_window_native_start(ui_window *window);
void ui_window_native_stop(ui_window *window);

#endif
