#ifndef UX_WINDOW_H
#define UX_WINDOW_H

#include "lib/ux-window/window_interface.h"
#include "lib/ux-base/component.h"

struct ux_window {
    ux_component base;
    char initial_title[UX_WINDOW_TITLE_CAPACITY];
    lib_bool initial_frozen;
    void *native_state;
};

lib_status ux_window_native_start(ux_window *window);
void ux_window_native_stop(ux_window *window);

#endif
