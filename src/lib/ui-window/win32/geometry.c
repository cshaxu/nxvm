#include "lib/types/win32/scalar.h"
#include "lib/types/win32/sync.h"
#include "lib/ui-window/win32/geometry.h"

static int ui_win32_window_decoration(lib_win32_hwnd window, int *width, int *height)
{
    lib_win32_rect outer;
    lib_win32_rect client;

    if (window == LIB_NULL || width == LIB_NULL || height == LIB_NULL ||
        !lib_win32_get_window_rect(window, &outer) || !lib_win32_get_client_rect(window, &client))
        return 0;
    *width = (outer.right - outer.left) - (client.right - client.left);
    *height = (outer.bottom - outer.top) - (client.bottom - client.top);
    return *width >= 0 && *height >= 0;
}

int ui_win32_enforce_client_aspect(lib_win32_hwnd window, lib_u32 source_width,
    lib_u32 source_height)
{
    lib_win32_rect client;
    int decoration_width;
    int decoration_height;
    int target_width;
    int target_height;

    if (window == LIB_NULL || source_width == 0u || source_height == 0u ||
        !lib_win32_get_client_rect(window, &client) || !ui_win32_window_decoration(window,
            &decoration_width, &decoration_height) ||
        !ui_window_fit_aspect_size(client.right - client.left,
            client.bottom - client.top, source_width, source_height,
            &target_width, &target_height)) return 0;
    if (target_width == client.right - client.left &&
        target_height == client.bottom - client.top) return 1;
    return lib_win32_set_window_pos(window, LIB_NULL, 0, 0, target_width + decoration_width,
        target_height + decoration_height,
        LIB_WIN32_SWP_NOMOVE | LIB_WIN32_SWP_NOZORDER | LIB_WIN32_SWP_NOACTIVATE) != LIB_WIN32_FALSE;
}

int ui_win32_maximize_client(lib_win32_hwnd window, lib_u32 source_width,
    lib_u32 source_height)
{
    lib_win32_monitorinfo monitor_info;
    lib_win32_hmonitor monitor;
    int decoration_width;
    int decoration_height;
    int client_width;
    int client_height;
    int outer_width;
    int outer_height;
    int available_width;
    int available_height;

    if (window == LIB_NULL || source_width == 0u || source_height == 0u ||
        !ui_win32_window_decoration(window, &decoration_width,
            &decoration_height)) return 0;
    monitor = lib_win32_monitor_from_window(window, LIB_WIN32_MONITOR_DEFAULTTONEAREST);
    lib_win32_zero_memory(&monitor_info, sizeof(monitor_info));
    monitor_info.cbSize = sizeof(monitor_info);
    if (monitor == LIB_NULL || !lib_win32_get_monitor_info_a(monitor, &monitor_info)) return 0;
    available_width = (monitor_info.rcWork.right - monitor_info.rcWork.left) -
        decoration_width;
    available_height = (monitor_info.rcWork.bottom - monitor_info.rcWork.top) -
        decoration_height;
    if (!ui_window_fit_aspect_size(available_width, available_height,
            source_width, source_height, &client_width, &client_height)) return 0;
    outer_width = client_width + decoration_width;
    outer_height = client_height + decoration_height;
    return lib_win32_set_window_pos(window, LIB_NULL,
        monitor_info.rcWork.left + ((monitor_info.rcWork.right -
            monitor_info.rcWork.left) - outer_width) / 2,
        monitor_info.rcWork.top + ((monitor_info.rcWork.bottom -
            monitor_info.rcWork.top) - outer_height) / 2,
        outer_width, outer_height, LIB_WIN32_SWP_NOZORDER | LIB_WIN32_SWP_NOACTIVATE) != LIB_WIN32_FALSE;
}

int ui_win32_resize_client(lib_win32_hwnd window, lib_u32 width,
    lib_u32 height)
{
    lib_win32_rect outer;
    lib_win32_monitorinfo monitor_info;
    lib_win32_hmonitor monitor;
    lib_win32_dword style;
    lib_win32_dword extended_style;

    if (window == LIB_NULL || width == 0u || height == 0u) return 0;
    lib_win32_set_rect(&outer, 0, 0, (int)width, (int)height);
    style = (lib_win32_dword)lib_win32_get_window_long_ptr_a(window, LIB_WIN32_GWL_STYLE);
    extended_style = (lib_win32_dword)lib_win32_get_window_long_ptr_a(window, LIB_WIN32_GWL_EXSTYLE);
    if (!lib_win32_adjust_window_rect_ex(&outer, style, LIB_WIN32_FALSE, extended_style)) return 0;
    monitor = lib_win32_monitor_from_window(window, LIB_WIN32_MONITOR_DEFAULTTONEAREST);
    lib_win32_zero_memory(&monitor_info, sizeof(monitor_info));
    monitor_info.cbSize = sizeof(monitor_info);
    if (monitor != LIB_NULL && lib_win32_get_monitor_info_a(monitor, &monitor_info)) {
        int decoration_width = (outer.right - outer.left) - (int)width;
        int decoration_height = (outer.bottom - outer.top) - (int)height;
        int fitted_width;
        int fitted_height;
        ui_window_rect work = ui_win32_rect_value(&monitor_info.rcWork);
        if (ui_window_fit_client_size(&work, decoration_width,
                decoration_height, (int)width, (int)height, &fitted_width,
                &fitted_height))
            lib_win32_set_rect(&outer, 0, 0, fitted_width + decoration_width,
                fitted_height + decoration_height);
    }
    return lib_win32_set_window_pos(window, LIB_NULL, 0, 0, outer.right - outer.left,
        outer.bottom - outer.top, LIB_WIN32_SWP_NOMOVE | LIB_WIN32_SWP_NOZORDER |
        LIB_WIN32_SWP_NOACTIVATE) != LIB_WIN32_FALSE;
}

void ui_win32_constrain_sizing(lib_win32_hwnd window, lib_win32_wparam edge,
    lib_win32_rect *outer, lib_u32 source_width, lib_u32 source_height)
{
    int frame_width, frame_height;
    ui_window_edge direction;
    ui_window_rect value;
    if (!outer || !ui_win32_window_decoration(window, &frame_width, &frame_height)) return;
    switch (edge) {
    case LIB_WIN32_WMSZ_LEFT: direction = UI_WINDOW_EDGE_LEFT; break;
    case LIB_WIN32_WMSZ_RIGHT: direction = UI_WINDOW_EDGE_RIGHT; break;
    case LIB_WIN32_WMSZ_TOP: direction = UI_WINDOW_EDGE_TOP; break;
    case LIB_WIN32_WMSZ_BOTTOM: direction = UI_WINDOW_EDGE_BOTTOM; break;
    case LIB_WIN32_WMSZ_TOPLEFT: direction = UI_WINDOW_EDGE_TOPLEFT; break;
    case LIB_WIN32_WMSZ_TOPRIGHT: direction = UI_WINDOW_EDGE_TOPRIGHT; break;
    case LIB_WIN32_WMSZ_BOTTOMLEFT: direction = UI_WINDOW_EDGE_BOTTOMLEFT; break;
    case LIB_WIN32_WMSZ_BOTTOMRIGHT: direction = UI_WINDOW_EDGE_BOTTOMRIGHT; break;
    default: return;
    }
    value = ui_win32_rect_value(outer);
    ui_window_constrain_sizing(&value, direction, frame_width, frame_height,
        source_width, source_height);
    ui_win32_rect_store(outer, &value);
}
