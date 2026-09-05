#include "geometry.h"

#ifdef _WIN32
int ux_win32_display_rect(int client_width, int client_height,
    uint32_t source_width, uint32_t source_height, RECT *display)
{
    if (display == NULL || source_width == 0u || source_height == 0u ||
        client_width <= 0 || client_height <= 0) return 0;
    display->left = 0;
    display->top = 0;
    display->right = client_width;
    display->bottom = client_height;
    return 1;
}

void ux_win32_map_dirty_rect(const RECT *source, const RECT *display,
    uint32_t source_width, uint32_t source_height, RECT *target)
{
    int width;
    int height;

    if (source == NULL || display == NULL || target == NULL ||
        source_width == 0u || source_height == 0u) return;
    width = display->right - display->left;
    height = display->bottom - display->top;
    target->left = display->left + source->left * width / (int)source_width;
    target->top = display->top + source->top * height / (int)source_height;
    target->right = display->left + (source->right * width +
        (int)source_width - 1) / (int)source_width;
    target->bottom = display->top + (source->bottom * height +
        (int)source_height - 1) / (int)source_height;
    if (target->right <= target->left) target->right = target->left + 1;
    if (target->bottom <= target->top) target->bottom = target->top + 1;
}

uint32_t ux_win32_dib_pixel(COLORREF colour)
{
    return ((uint32_t)GetRValue(colour) << 16) |
        ((uint32_t)GetGValue(colour) << 8) | (uint32_t)GetBValue(colour);
}

int ux_win32_resize_client(HWND window, uint32_t width,
    uint32_t height)
{
    RECT outer;
    DWORD style;
    DWORD extended_style;

    if (window == NULL || width == 0u || height == 0u) return 0;
    SetRect(&outer, 0, 0, (int)width, (int)height);
    style = (DWORD)GetWindowLongPtrA(window, GWL_STYLE);
    extended_style = (DWORD)GetWindowLongPtrA(window, GWL_EXSTYLE);
    if (!AdjustWindowRectEx(&outer, style, FALSE, extended_style)) return 0;
    SetWindowPos(window, NULL, 0, 0, outer.right - outer.left,
        outer.bottom - outer.top, SWP_NOMOVE | SWP_NOZORDER | SWP_NOACTIVATE);
    return 1;
}

void ux_win32_constrain_sizing(HWND window, WPARAM edge,
    RECT *outer, uint32_t source_width, uint32_t source_height)
{
    RECT current_window;
    RECT current_client;
    int frame_width;
    int frame_height;
    int client_width;
    int client_height;
    int target_width;
    int target_height;

    if (window == NULL || outer == NULL || source_width == 0u ||
        source_height == 0u) return;
    GetWindowRect(window, &current_window);
    GetClientRect(window, &current_client);
    frame_width = (current_window.right - current_window.left) -
        (current_client.right - current_client.left);
    frame_height = (current_window.bottom - current_window.top) -
        (current_client.bottom - current_client.top);
    target_width = outer->right - outer->left;
    target_height = outer->bottom - outer->top;
    client_width = target_width - frame_width;
    client_height = target_height - frame_height;
    if (client_width <= 0 || client_height <= 0) return;
    if (edge == WMSZ_LEFT || edge == WMSZ_RIGHT) {
        client_height = (int)((uint64_t)client_width * source_height /
            source_width);
    } else if (edge == WMSZ_TOP || edge == WMSZ_BOTTOM) {
        client_width = (int)((uint64_t)client_height * source_width /
            source_height);
    } else if ((uint64_t)client_width * source_height >=
        (uint64_t)client_height * source_width) {
        client_height = (int)((uint64_t)client_width * source_height /
            source_width);
    } else {
        client_width = (int)((uint64_t)client_height * source_width /
            source_height);
    }
    target_width = client_width + frame_width;
    target_height = client_height + frame_height;
    if (edge == WMSZ_LEFT || edge == WMSZ_TOPLEFT || edge == WMSZ_BOTTOMLEFT)
        outer->left = outer->right - target_width;
    else
        outer->right = outer->left + target_width;
    if (edge == WMSZ_TOP || edge == WMSZ_TOPLEFT || edge == WMSZ_TOPRIGHT)
        outer->top = outer->bottom - target_height;
    else
        outer->bottom = outer->top + target_height;
}
#endif
