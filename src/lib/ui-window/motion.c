#include "lib/ui-window/motion.h"

int ui_window_motion_move(ui_window_motion *mouse, int x, int y, int client_width, int client_height,
    lib_u32 content_width, lib_u32 content_height, int *dx, int *dy)
{
    if (mouse == LIB_NULL || dx == LIB_NULL || dy == LIB_NULL) return 0;
    *dx = mouse->valid ? x - mouse->x : 0;
    *dy = mouse->valid ? y - mouse->y : 0;
    mouse->x = x;
    mouse->y = y;
    mouse->valid = 1;
    if (mouse->client_width != client_width || mouse->content_width != content_width)
        mouse->remainder_x = 0;
    if (mouse->client_height != client_height || mouse->content_height != content_height)
        mouse->remainder_y = 0;
    mouse->client_width = client_width; mouse->client_height = client_height;
    mouse->content_width = content_width; mouse->content_height = content_height;
    if (client_width > 0 && content_width != 0u) {
        lib_i64 total = (lib_i64)*dx * content_width + mouse->remainder_x;
        *dx = (int)(total / client_width);
        mouse->remainder_x = total % client_width;
    }
    if (client_height > 0 && content_height != 0u) {
        lib_i64 total = (lib_i64)*dy * content_height + mouse->remainder_y;
        *dy = (int)(total / client_height);
        mouse->remainder_y = total % client_height;
    }
    return 1;
}
