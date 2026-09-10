#ifndef UX_WIN32_GEOMETRY_H
#define UX_WIN32_GEOMETRY_H

#include "lib/base/base_interface.h"

#ifdef _WIN32
#include <windows.h>

/* Host-only geometry helpers.  They map copied source coordinates to a Win32
 * client surface and own no product rendering or lifecycle policy. */
int ux_win32_display_rect(int client_width, int client_height,
    lib_u32 source_width, lib_u32 source_height, RECT *display);
void ux_win32_map_dirty_rect(const RECT *source, const RECT *display,
    lib_u32 source_width, lib_u32 source_height, RECT *target);
int ux_win32_fit_outer_rect(const RECT *work_area, int desired_width,
    int desired_height, RECT *fitted);
int ux_win32_fit_client_size(const RECT *work_area, int decoration_width,
    int decoration_height, int desired_width, int desired_height,
    int *fitted_width, int *fitted_height);
int ux_win32_fit_aspect_size(int available_width, int available_height,
    lib_u32 source_width, lib_u32 source_height, int *fitted_width,
    int *fitted_height);
int ux_win32_resize_client(HWND window, lib_u32 width,
    lib_u32 height);
int ux_win32_enforce_client_aspect(HWND window, lib_u32 source_width,
    lib_u32 source_height);
int ux_win32_maximize_client(HWND window, lib_u32 source_width,
    lib_u32 source_height);
void ux_win32_constrain_sizing(HWND window, WPARAM edge,
    RECT *outer, lib_u32 source_width, lib_u32 source_height);
#endif

#endif
