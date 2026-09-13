#ifndef UI_WINDOW_RENDER_H
#define UI_WINDOW_RENDER_H
#include "lib/ui-window/geometry.h"
int ui_window_frame_size(const ui_frame *frame, lib_u32 *width, lib_u32 *height);

void ui_window_render_text(const ui_frame *frame, lib_u32 *pixels,
    lib_u32 width, lib_u32 height);
int ui_window_render_graphics(const ui_frame *frame, lib_u32 *pixels,
    lib_u32 width, lib_u32 height, lib_u32 *palette, int *valid,
    ui_window_rect *changed);
#endif
