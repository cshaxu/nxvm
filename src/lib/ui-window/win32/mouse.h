#ifndef UI_WIN32_MOUSE_H
#define UI_WIN32_MOUSE_H

#include "lib/ui-window/motion.h"
#include "lib/types/win32/window.h"

/* Host-only relative-pointer state. The caller owns input-device buttons and
 * lifecycle policy; this component owns explicit Win32 client capture only. */
typedef struct ui_win32_mouse {
    ui_window_motion motion;
    lib_bool captured;
    lib_win32_hwnd window;
} ui_win32_mouse;

void ui_win32_mouse_reset(ui_win32_mouse *mouse);
void ui_win32_mouse_release(ui_win32_mouse *mouse);
int ui_win32_mouse_refresh_bounds(ui_win32_mouse *mouse);
int ui_win32_mouse_capture(ui_win32_mouse *mouse,
    lib_win32_hwnd window, lib_win32_lparam position);
int ui_win32_mouse_move(ui_win32_mouse *mouse,
    lib_win32_lparam position, int client_width, int client_height,
    unsigned int content_width, unsigned int content_height, int *dx, int *dy);
int ui_win32_mouse_captured(const ui_win32_mouse *mouse);
#endif
