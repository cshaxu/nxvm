#ifndef UI_WIN32_WINDOW_COMPONENT_H
#define UI_WIN32_WINDOW_COMPONENT_H

#include "lib/ui-window/window.h"

#ifdef _WIN32
lib_status ui_window_native_start(ui_window *window);
void ui_window_native_stop(ui_window *window);
#endif

#endif
