#ifndef UI_WINDOW_MOTION_H
#define UI_WINDOW_MOTION_H
#include "lib/types/types_interface.h"
typedef struct ui_window_motion {
    int x, y, valid;
    lib_i64 remainder_x, remainder_y;
    int client_width, client_height;
    lib_u32 content_width, content_height;
} ui_window_motion;
int ui_window_motion_move(ui_window_motion *motion, int x, int y,
    int client_width, int client_height, lib_u32 content_width,
    lib_u32 content_height, int *dx, int *dy);
#endif
