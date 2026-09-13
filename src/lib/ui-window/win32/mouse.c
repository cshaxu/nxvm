#include "lib/types/win32/window.h"
#include "lib/ui-window/win32/mouse.h"

void ui_win32_mouse_reset(ui_win32_mouse *mouse)
{
    if (mouse == LIB_NULL) return;
    lib_memory_set(mouse, 0, sizeof(*mouse));
}

void ui_win32_mouse_release(ui_win32_mouse *mouse)
{
    if (mouse == LIB_NULL || !mouse->captured) return;
    /* Clear ownership before ReleaseCapture synchronously notifies the window. */
    mouse->captured = LIB_FALSE;
    lib_win32_clip_cursor(LIB_NULL);
    if (lib_win32_get_capture() == mouse->window) lib_win32_release_capture();
    mouse->window = LIB_NULL;
    lib_win32_set_cursor(lib_win32_load_cursor_a(LIB_NULL, LIB_WIN32_IDC_ARROW));
    mouse->motion.valid = 0;
    mouse->motion.remainder_x = mouse->motion.remainder_y = 0;
}

int ui_win32_mouse_refresh_bounds(ui_win32_mouse *mouse)
{
    lib_win32_rect client, bounds;
    lib_win32_point upper_left, lower_right;
    if (mouse == LIB_NULL) return 0;
    if (!mouse->captured) return 1;
    if (lib_win32_get_capture() != mouse->window ||
        !lib_win32_get_client_rect(mouse->window, &client) ||
        client.right <= client.left || client.bottom <= client.top) return 0;
    upper_left.x = client.left; upper_left.y = client.top;
    lower_right.x = client.right; lower_right.y = client.bottom;
    if (!lib_win32_client_to_screen(mouse->window, &upper_left) ||
        !lib_win32_client_to_screen(mouse->window, &lower_right)) return 0;
    bounds.left = upper_left.x; bounds.top = upper_left.y;
    bounds.right = lower_right.x; bounds.bottom = lower_right.y;
    return lib_win32_clip_cursor(&bounds) != 0;
}

int ui_win32_mouse_capture(ui_win32_mouse *mouse,
    lib_win32_hwnd window, lib_win32_lparam position)
{
    if (mouse == LIB_NULL || window == LIB_NULL) return 0;
    lib_win32_set_focus(window);
    if (lib_win32_get_focus() != window) return 0;
    lib_win32_set_capture(window);
    if (lib_win32_get_capture() != window) return 0;
    mouse->window = window;
    mouse->captured = LIB_TRUE;
    if (!ui_win32_mouse_refresh_bounds(mouse)) {
        ui_win32_mouse_release(mouse);
        return 0;
    }
    mouse->motion.x = (int)(short)lib_win32_loword(position);
    mouse->motion.y = (int)(short)lib_win32_hiword(position);
    mouse->motion.valid = 1;
    mouse->motion.remainder_x = mouse->motion.remainder_y = 0;
    return 1;
}

int ui_win32_mouse_move(ui_win32_mouse *mouse,
    lib_win32_lparam position, int client_width, int client_height,
    unsigned int content_width, unsigned int content_height, int *dx, int *dy)
{
    return mouse != LIB_NULL && ui_window_motion_move(&mouse->motion,
        (int)(short)lib_win32_loword(position), (int)(short)lib_win32_hiword(position),
        client_width, client_height, content_width, content_height, dx, dy);
}

int ui_win32_mouse_captured(const ui_win32_mouse *mouse)
{
    return mouse != LIB_NULL && mouse->captured;
}
