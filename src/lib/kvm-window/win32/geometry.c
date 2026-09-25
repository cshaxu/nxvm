#include "lib/types/win32/scalar.h"
#include "lib/types/win32/sync.h"
#include "lib/kvm-window/win32/geometry.h"

static lib_bool kvm_win32_window_decoration(lib_win32_hwnd window, lib_i32 *width, lib_i32 *height)
{
    lib_win32_rect outer;
    lib_win32_rect client;

    if (window == LIB_NULL || width == LIB_NULL || height == LIB_NULL ||
        !lib_win32_get_window_rect(window, &outer) || !lib_win32_get_client_rect(window, &client))
        return LIB_FALSE;
    *width = (outer.right - outer.left) - (client.right - client.left);
    *height = (outer.bottom - outer.top) - (client.bottom - client.top);
    return *width >= 0 && *height >= 0;
}

lib_status kvm_win32_enforce_client_aspect(lib_win32_hwnd window, lib_u32 source_width,
    lib_u32 source_height)
{
    lib_win32_rect client;
    lib_i32 decoration_width;
    lib_i32 decoration_height;
    lib_i32 target_width;
    lib_i32 target_height;

    if (window == LIB_NULL || !lib_win32_get_client_rect(window, &client)) return LIB_STATUS_IO_ERROR;
    if (client.right <= client.left || client.bottom <= client.top) return LIB_STATUS_OK;
    if (source_width == 0u || source_height == 0u || !kvm_win32_window_decoration(window,
            &decoration_width, &decoration_height) ||
        !kvm_window_fit_aspect_size(client.right - client.left,
            client.bottom - client.top, source_width, source_height,
            &target_width, &target_height)) return LIB_STATUS_IO_ERROR;
    if (target_width == client.right - client.left &&
        target_height == client.bottom - client.top) return LIB_STATUS_OK;
    return lib_win32_set_window_pos(window, LIB_NULL, 0, 0, target_width + decoration_width,
        target_height + decoration_height,
        LIB_WIN32_SWP_NOMOVE | LIB_WIN32_SWP_NOZORDER | LIB_WIN32_SWP_NOACTIVATE) ? LIB_STATUS_OK : LIB_STATUS_IO_ERROR;
}

lib_status kvm_win32_maximize_client(lib_win32_hwnd window, lib_u32 source_width,
    lib_u32 source_height)
{
    lib_win32_monitorinfo monitor_info;
    lib_win32_hmonitor monitor;
    lib_i32 decoration_width;
    lib_i32 decoration_height;
    lib_i32 client_width;
    lib_i32 client_height;
    lib_i32 outer_width;
    lib_i32 outer_height;
    lib_i32 available_width;
    lib_i32 available_height;

    if (window == LIB_NULL || source_width == 0u || source_height == 0u ||
        !kvm_win32_window_decoration(window, &decoration_width,
            &decoration_height)) return LIB_STATUS_IO_ERROR;
    monitor = lib_win32_monitor_from_window(window, LIB_WIN32_MONITOR_DEFAULTTONEAREST);
    lib_win32_zero_memory(&monitor_info, sizeof(monitor_info));
    monitor_info.cbSize = sizeof(monitor_info);
    if (monitor == LIB_NULL || !lib_win32_get_monitor_info_a(monitor, &monitor_info)) return LIB_STATUS_IO_ERROR;
    available_width = (monitor_info.rcWork.right - monitor_info.rcWork.left) -
        decoration_width;
    available_height = (monitor_info.rcWork.bottom - monitor_info.rcWork.top) -
        decoration_height;
    if (!kvm_window_fit_aspect_size(available_width, available_height,
            source_width, source_height, &client_width, &client_height)) return LIB_STATUS_IO_ERROR;
    outer_width = client_width + decoration_width;
    outer_height = client_height + decoration_height;
    return lib_win32_set_window_pos(window, LIB_NULL,
        monitor_info.rcWork.left + ((monitor_info.rcWork.right -
            monitor_info.rcWork.left) - outer_width) / 2,
        monitor_info.rcWork.top + ((monitor_info.rcWork.bottom -
            monitor_info.rcWork.top) - outer_height) / 2,
        outer_width, outer_height, LIB_WIN32_SWP_NOZORDER | LIB_WIN32_SWP_NOACTIVATE) ? LIB_STATUS_OK : LIB_STATUS_IO_ERROR;
}

lib_status kvm_win32_resize_client(lib_win32_hwnd window, lib_u32 width,
    lib_u32 height)
{
    lib_win32_rect outer;
    lib_win32_monitorinfo monitor_info;
    lib_win32_hmonitor monitor;
    lib_win32_dword style;
    lib_win32_dword extended_style;

    if (window == LIB_NULL || width == 0u || height == 0u) return LIB_STATUS_IO_ERROR;
    lib_win32_set_rect(&outer, 0, 0, (lib_i32)width, (lib_i32)height);
    style = (lib_win32_dword)lib_win32_get_window_long_ptr_a(window, LIB_WIN32_GWL_STYLE);
    extended_style = (lib_win32_dword)lib_win32_get_window_long_ptr_a(window, LIB_WIN32_GWL_EXSTYLE);
    if (!lib_win32_adjust_window_rect_ex(&outer, style, LIB_WIN32_FALSE, extended_style)) return LIB_STATUS_IO_ERROR;
    monitor = lib_win32_monitor_from_window(window, LIB_WIN32_MONITOR_DEFAULTTONEAREST);
    lib_win32_zero_memory(&monitor_info, sizeof(monitor_info));
    monitor_info.cbSize = sizeof(monitor_info);
    if (monitor != LIB_NULL && lib_win32_get_monitor_info_a(monitor, &monitor_info)) {
        lib_i32 decoration_width = (outer.right - outer.left) - (lib_i32)width;
        lib_i32 decoration_height = (outer.bottom - outer.top) - (lib_i32)height;
        lib_i32 fitted_width;
        lib_i32 fitted_height;
        kvm_window_rect work = kvm_win32_rect_value(&monitor_info.rcWork);
        if (kvm_window_fit_client_size(&work, decoration_width,
                decoration_height, (lib_i32)width, (lib_i32)height, &fitted_width,
                &fitted_height))
            lib_win32_set_rect(&outer, 0, 0, fitted_width + decoration_width,
                fitted_height + decoration_height);
    }
    return lib_win32_set_window_pos(window, LIB_NULL, 0, 0, outer.right - outer.left,
        outer.bottom - outer.top, LIB_WIN32_SWP_NOMOVE | LIB_WIN32_SWP_NOZORDER |
        LIB_WIN32_SWP_NOACTIVATE) ? LIB_STATUS_OK : LIB_STATUS_IO_ERROR;
}

lib_status kvm_win32_constrain_sizing(lib_win32_hwnd window, lib_win32_wparam edge,
    lib_win32_rect *outer, lib_u32 source_width, lib_u32 source_height)
{
    lib_i32 frame_width, frame_height;
    kvm_window_edge direction;
    kvm_window_rect value;
    if (source_width == 0u || source_height == 0u) return LIB_STATUS_OK;
    if (!outer || !kvm_win32_window_decoration(window, &frame_width, &frame_height)) return LIB_STATUS_IO_ERROR;
    switch (edge) {
    case LIB_WIN32_WMSZ_LEFT: direction = KVM_WINDOW_EDGE_LEFT; break;
    case LIB_WIN32_WMSZ_RIGHT: direction = KVM_WINDOW_EDGE_RIGHT; break;
    case LIB_WIN32_WMSZ_TOP: direction = KVM_WINDOW_EDGE_TOP; break;
    case LIB_WIN32_WMSZ_BOTTOM: direction = KVM_WINDOW_EDGE_BOTTOM; break;
    case LIB_WIN32_WMSZ_TOPLEFT: direction = KVM_WINDOW_EDGE_TOPLEFT; break;
    case LIB_WIN32_WMSZ_TOPRIGHT: direction = KVM_WINDOW_EDGE_TOPRIGHT; break;
    case LIB_WIN32_WMSZ_BOTTOMLEFT: direction = KVM_WINDOW_EDGE_BOTTOMLEFT; break;
    case LIB_WIN32_WMSZ_BOTTOMRIGHT: direction = KVM_WINDOW_EDGE_BOTTOMRIGHT; break;
    default: return LIB_STATUS_IO_ERROR;
    }
    value = kvm_win32_rect_value(outer);
    kvm_window_constrain_sizing(&value, direction, frame_width, frame_height,
        source_width, source_height);
    kvm_win32_rect_store(outer, &value);
    return LIB_STATUS_OK;
}
