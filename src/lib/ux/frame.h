#ifndef UX_FRAME_H
#define UX_FRAME_H

#include "type.h"

/* Value-only ABI shared between a project runtime and the Win32 presenter.
 * It intentionally carries no renderer pointer, machine pointer, or lock. */
#define UX_TEXT_COLUMNS 80u
#define UX_TEXT_ROWS 25u
#define UX_DIB_MAX_WIDTH 1280u
#define UX_DIB_MAX_HEIGHT 768u
#define UX_DIB_MAX_BYTES (UX_DIB_MAX_WIDTH * UX_DIB_MAX_HEIGHT)
#define UX_DIB_INFO_BYTES 1064u

typedef struct ux_frame {
    type_unsigned_32 sequence;
    type_unsigned_32 graphics;
    type_unsigned_32 valid;
    type_signed_32 cursor_column;
    type_signed_32 cursor_row;
    type_unsigned_32 cursor_size;
    type_unsigned_8 text[UX_TEXT_COLUMNS * UX_TEXT_ROWS];
    type_unsigned_16 attributes[UX_TEXT_COLUMNS * UX_TEXT_ROWS];
    type_unsigned_32 text_palette[16u];
    type_unsigned_8 font[256u * 16u];
    type_unsigned_8 secondary_font[256u * 16u];
    type_unsigned_32 font_height;
    type_unsigned_32 attribute_font_select;
    type_unsigned_32 dib_width;
    type_unsigned_32 dib_height;
    type_signed_32 dirty_left;
    type_signed_32 dirty_top;
    type_signed_32 dirty_right;
    type_signed_32 dirty_bottom;
    type_unsigned_8 dib_info[UX_DIB_INFO_BYTES];
    type_unsigned_8 dib_bits[UX_DIB_MAX_BYTES];
} ux_frame;

#endif
