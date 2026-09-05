#ifndef UX_WIN32_MOUSE_H
#define UX_WIN32_MOUSE_H

#ifdef _WIN32
#include <windows.h>

/* Host-only relative-pointer state.  The caller owns guest-device buttons and
 * lifecycle policy; this component owns explicit Win32 client capture only. */
typedef struct ux_win32_mouse {
    int x;
    int y;
    int valid;
    int captured;
    int host_cursor_hidden;
} ux_win32_mouse;

void ux_win32_mouse_reset(ux_win32_mouse *mouse);
void ux_win32_mouse_release(ux_win32_mouse *mouse);
int ux_win32_mouse_capture(ux_win32_mouse *mouse,
    HWND window, LPARAM position);
int ux_win32_mouse_move(ux_win32_mouse *mouse,
    LPARAM position, int client_width, int client_height,
    unsigned int guest_width, unsigned int guest_height, int *dx, int *dy);
int ux_win32_mouse_captured(const ux_win32_mouse *mouse);
int ux_win32_mouse_hides_host_cursor(
    const ux_win32_mouse *mouse);
#endif

#endif
