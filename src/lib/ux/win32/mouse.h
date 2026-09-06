#ifndef UX_WIN32_MOUSE_H
#define UX_WIN32_MOUSE_H

#ifdef _WIN32
#include "lib/ux/capture.h"
#include <windows.h>

/* Host-only relative-pointer state. The caller owns input-device buttons and
 * lifecycle policy; this component owns explicit Win32 client capture only. */
typedef struct ux_win32_mouse {
    int x;
    int y;
    int valid;
    ux_capture capture;
} ux_win32_mouse;

void ux_win32_mouse_reset(ux_win32_mouse *mouse);
void ux_win32_mouse_release(ux_win32_mouse *mouse);
int ux_win32_mouse_capture(ux_win32_mouse *mouse,
    HWND window, LPARAM position);
int ux_win32_mouse_move(ux_win32_mouse *mouse,
    LPARAM position, int client_width, int client_height,
    unsigned int content_width, unsigned int content_height, int *dx, int *dy);
int ux_win32_mouse_captured(const ux_win32_mouse *mouse);
#endif

#endif
