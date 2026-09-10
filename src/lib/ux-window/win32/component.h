#ifndef UX_WIN32_WINDOW_COMPONENT_H
#define UX_WIN32_WINDOW_COMPONENT_H

#include "lib/ux-window/window.h"

#ifdef _WIN32
lib_status ux_window_native_start(ux_window *window);
void ux_window_native_stop(ux_window *window);
#endif

#endif
