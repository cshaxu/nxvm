#include "lib/ui-window/win32/mouse.h"

#ifdef _WIN32
void ui_win32_mouse_reset(ui_win32_mouse *mouse)
{
    if (mouse == NULL) return;
    mouse->x = 0;
    mouse->y = 0;
    mouse->valid = 0;
    ui_capture_initialize(&mouse->capture);
}

void ui_win32_mouse_release(ui_win32_mouse *mouse)
{
    if (mouse == NULL || !ui_capture_is_active(&mouse->capture)) return;
    ClipCursor(NULL);
    ReleaseCapture();
    SetCursor(LoadCursorA(NULL, IDC_ARROW));
    ui_capture_release(&mouse->capture);
    mouse->valid = 0;
}

int ui_win32_mouse_capture(ui_win32_mouse *mouse,
    HWND window, LPARAM position)
{
    RECT client;
    POINT upper_left;
    POINT lower_right;
    RECT bounds;

    if (mouse == NULL || window == NULL || !GetClientRect(window, &client) ||
        client.right <= client.left || client.bottom <= client.top) return 0;
    SetFocus(window);
    if (GetFocus() != window) return 0;
    SetCapture(window);
    if (GetCapture() != window) return 0;
    upper_left.x = client.left;
    upper_left.y = client.top;
    lower_right.x = client.right;
    lower_right.y = client.bottom;
    if (!ClientToScreen(window, &upper_left) ||
        !ClientToScreen(window, &lower_right)) {
        ReleaseCapture();
        return 0;
    }
    bounds.left = upper_left.x;
    bounds.top = upper_left.y;
    bounds.right = lower_right.x;
    bounds.bottom = lower_right.y;
    if (!ClipCursor(&bounds)) {
        ReleaseCapture();
        return 0;
    }
    mouse->x = (int)(short)LOWORD(position);
    mouse->y = (int)(short)HIWORD(position);
    mouse->valid = 1;
    ui_capture_activate(&mouse->capture);
    return 1;
}

int ui_win32_mouse_move(ui_win32_mouse *mouse,
    LPARAM position, int client_width, int client_height,
    unsigned int content_width, unsigned int content_height, int *dx, int *dy)
{
    int x;
    int y;

    if (mouse == NULL || dx == NULL || dy == NULL) return 0;
    x = (int)(short)LOWORD(position);
    y = (int)(short)HIWORD(position);
    *dx = mouse->valid ? x - mouse->x : 0;
    *dy = mouse->valid ? y - mouse->y : 0;
    mouse->x = x;
    mouse->y = y;
    mouse->valid = 1;
    if (client_width > 0 && content_width != 0u)
        *dx = (int)((long long)*dx * (long long)content_width / client_width);
    if (client_height > 0 && content_height != 0u)
        *dy = (int)((long long)*dy * (long long)content_height / client_height);
    return 1;
}

int ui_win32_mouse_captured(const ui_win32_mouse *mouse)
{
    return mouse != NULL && ui_capture_is_active(&mouse->capture);
}

#endif
