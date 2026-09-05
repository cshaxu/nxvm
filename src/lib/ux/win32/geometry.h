#ifndef UX_WIN32_GEOMETRY_H
#define UX_WIN32_GEOMETRY_H

#include <stdint.h>

#ifdef _WIN32
#include <windows.h>

/* Host-only geometry helpers.  They map copied source coordinates to a Win32
 * client surface and own no guest rendering or lifecycle policy. */
int ux_win32_display_rect(int client_width, int client_height,
    uint32_t source_width, uint32_t source_height, RECT *display);
void ux_win32_map_dirty_rect(const RECT *source, const RECT *display,
    uint32_t source_width, uint32_t source_height, RECT *target);
uint32_t ux_win32_dib_pixel(COLORREF colour);
int ux_win32_resize_client(HWND window, uint32_t width,
    uint32_t height);
void ux_win32_constrain_sizing(HWND window, WPARAM edge,
    RECT *outer, uint32_t source_width, uint32_t source_height);
#endif

#endif
