#ifndef UI_WIN32_GEOMETRY_H
#define UI_WIN32_GEOMETRY_H

#include "lib/types/types_interface.h"

#include "lib/types/win32/window.h"

#include "lib/ui-window/geometry.h"

/* Native rectangle marshalling only; all arithmetic belongs to geometry.c. */
static inline ui_window_rect ui_win32_rect_value(const lib_win32_rect *r)
{
    return (ui_window_rect) { r->left, r->top, r->right, r->bottom };
}
static inline void ui_win32_rect_store(lib_win32_rect *to, const ui_window_rect *r)
{
    to->left = r->left;
    to->top = r->top;
    to->right = r->right;
    to->bottom = r->bottom;
}
int ui_win32_resize_client(lib_win32_hwnd window, lib_u32 width,
    lib_u32 height);
int ui_win32_enforce_client_aspect(lib_win32_hwnd window, lib_u32 source_width,
    lib_u32 source_height);
int ui_win32_maximize_client(lib_win32_hwnd window, lib_u32 source_width,
    lib_u32 source_height);
void ui_win32_constrain_sizing(lib_win32_hwnd window, lib_win32_wparam edge,
    lib_win32_rect *outer, lib_u32 source_width, lib_u32 source_height);
#endif
