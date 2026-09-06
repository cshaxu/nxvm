#ifndef UX_FRAME_H
#define UX_FRAME_H

#include "lib/base/base.h"

/* Value-only ABI shared between a project runtime and the Win32 presenter.
 * It intentionally carries no renderer pointer, machine pointer, or lock. */
#define UX_TEXT_COLUMNS 80u
#define UX_TEXT_ROWS 25u
#define UX_GRAPHICS_MAX_WIDTH 1280u
#define UX_GRAPHICS_MAX_HEIGHT 768u
#define UX_GRAPHICS_MAX_PIXELS (UX_GRAPHICS_MAX_WIDTH * UX_GRAPHICS_MAX_HEIGHT)
#define UX_GRAPHICS_PALETTE_ENTRIES 256u

typedef struct ux_frame {
    lib_u32 sequence;
    lib_u32 graphics;
    lib_u32 valid;
    lib_u16 text_columns;
    lib_u16 text_rows;
    lib_i32 cursor_column;
    lib_i32 cursor_row;
    lib_u8 cursor_top;
    lib_u8 cursor_bottom;
    lib_u8 cursor_visible;
    lib_u8 cursor_phase;
    lib_u8 text[UX_TEXT_COLUMNS * UX_TEXT_ROWS];
    lib_u16 attributes[UX_TEXT_COLUMNS * UX_TEXT_ROWS];
    lib_u32 text_palette[16u];
    lib_u8 font[256u * 16u];
    lib_u8 secondary_font[256u * 16u];
    lib_u32 font_height;
    lib_u32 attribute_font_select;
    lib_u32 graphics_width;
    lib_u32 graphics_height;
    lib_u32 graphics_stride;
    lib_i32 dirty_left;
    lib_i32 dirty_top;
    lib_i32 dirty_right;
    lib_i32 dirty_bottom;
    lib_u32 graphics_palette[UX_GRAPHICS_PALETTE_ENTRIES];
    lib_u8 graphics_pixels[UX_GRAPHICS_MAX_PIXELS];
} ux_frame;

static inline lib_bool ux_frame_is_valid(const ux_frame *frame)
{
    if (frame == LIB_NULL || frame->valid == 0u) return LIB_FALSE;
    if (frame->graphics != 0u) {
        return frame->graphics_width != 0u &&
            frame->graphics_width <= UX_GRAPHICS_MAX_WIDTH &&
            frame->graphics_height != 0u &&
            frame->graphics_height <= UX_GRAPHICS_MAX_HEIGHT &&
            frame->graphics_stride >= frame->graphics_width &&
            frame->graphics_stride <= UX_GRAPHICS_MAX_WIDTH;
    }
    return frame->text_columns != 0u && frame->text_columns <= UX_TEXT_COLUMNS &&
        frame->text_rows != 0u && frame->text_rows <= UX_TEXT_ROWS;
}

#endif
