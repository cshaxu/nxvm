#ifndef UI_FRAME_INTERFACE_H
#define UI_FRAME_INTERFACE_H

#include "lib/types/types_interface.h"

/* Value-only ABI shared between a project runtime and the Win32 presenter.
 * It intentionally carries no renderer pointer, application-state pointer, or lock. */
/* Palette entries are platform-neutral 0x00RRGGBB values. They never carry
 * Win32 COLORREF byte order; a native presenter converts only at its boundary. */
#define UI_TEXT_COLUMNS 80u
#define UI_TEXT_ROWS 25u
#define UI_GRAPHICS_MAX_WIDTH 1280u
#define UI_GRAPHICS_MAX_HEIGHT 768u
#define UI_GRAPHICS_MAX_PIXELS (UI_GRAPHICS_MAX_WIDTH * UI_GRAPHICS_MAX_HEIGHT)
#define UI_GRAPHICS_PALETTE_ENTRIES 256u

typedef struct ui_frame {
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
    lib_u8 text[UI_TEXT_COLUMNS * UI_TEXT_ROWS];
    lib_u16 attributes[UI_TEXT_COLUMNS * UI_TEXT_ROWS];
    lib_u32 text_palette[16u]; /* 0x00RRGGBB */
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
    lib_u32 graphics_palette[UI_GRAPHICS_PALETTE_ENTRIES]; /* 0x00RRGGBB */
    lib_u8 graphics_pixels[UI_GRAPHICS_MAX_PIXELS];
} ui_frame;

static inline lib_bool ui_frame_is_valid(const ui_frame *frame)
{
    if (frame == LIB_NULL || frame->valid == 0u) return LIB_FALSE;
    if (frame->graphics != 0u) {
        return frame->graphics_width != 0u &&
            frame->graphics_width <= UI_GRAPHICS_MAX_WIDTH &&
            frame->graphics_height != 0u &&
            frame->graphics_height <= UI_GRAPHICS_MAX_HEIGHT &&
            frame->graphics_stride >= frame->graphics_width &&
            frame->graphics_stride <= UI_GRAPHICS_MAX_WIDTH;
    }
    return frame->text_columns != 0u && frame->text_columns <= UI_TEXT_COLUMNS &&
        frame->text_rows != 0u && frame->text_rows <= UI_TEXT_ROWS;
}

#endif
