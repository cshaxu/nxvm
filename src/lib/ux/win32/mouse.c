#include "mouse.h"

#ifdef _WIN32
void ux_win32_mouse_reset(ux_win32_mouse *mouse)
{
    if (mouse == NULL) return;
    mouse->x = 0;
    mouse->y = 0;
    mouse->valid = 0;
    mouse->captured = 0;
    mouse->host_cursor_hidden = 0;
}

void ux_win32_mouse_release(ux_win32_mouse *mouse)
{
    if (mouse == NULL || !mouse->captured) return;
    ClipCursor(NULL);
    ReleaseCapture();
    mouse->host_cursor_hidden = 0;
    SetCursor(LoadCursorA(NULL, IDC_ARROW));
    mouse->captured = 0;
    mouse->valid = 0;
}

int ux_win32_mouse_capture(ux_win32_mouse *mouse,
    HWND window, LPARAM position)
{
    RECT client;
    POINT upper_left;
    POINT lower_right;
    RECT bounds;

    if (mouse == NULL || window == NULL) return 0;
    SetFocus(window);
    SetCapture(window);
    GetClientRect(window, &client);
    upper_left.x = client.left;
    upper_left.y = client.top;
    lower_right.x = client.right;
    lower_right.y = client.bottom;
    if (ClientToScreen(window, &upper_left) &&
        ClientToScreen(window, &lower_right)) {
        bounds.left = upper_left.x;
        bounds.top = upper_left.y;
        bounds.right = lower_right.x;
        bounds.bottom = lower_right.y;
        (void)ClipCursor(&bounds);
    }
    mouse->x = (int)(short)LOWORD(position);
    mouse->y = (int)(short)HIWORD(position);
    mouse->valid = 1;
    mouse->captured = 1;
    mouse->host_cursor_hidden = 1;
    return 1;
}

int ux_win32_mouse_move(ux_win32_mouse *mouse,
    LPARAM position, int client_width, int client_height,
    unsigned int guest_width, unsigned int guest_height, int *dx, int *dy)
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
    if (client_width > 0 && guest_width != 0u)
        *dx = (int)((long long)*dx * (long long)guest_width / client_width);
    if (client_height > 0 && guest_height != 0u)
        *dy = (int)((long long)*dy * (long long)guest_height / client_height);
    return 1;
}

int ux_win32_mouse_captured(const ux_win32_mouse *mouse)
{
    return mouse != NULL && mouse->captured;
}

int ux_win32_mouse_hides_host_cursor(
    const ux_win32_mouse *mouse)
{
    return mouse != NULL && mouse->host_cursor_hidden;
}
#endif
