#ifndef CORE_PLATFORM_DISPLAY_FRAME_H
#define CORE_PLATFORM_DISPLAY_FRAME_H

#include "type.h"


#define CORE_PLATFORM_DISPLAY_MAX_COLUMNS 80u
#define CORE_PLATFORM_DISPLAY_MAX_ROWS 25u
#define CORE_PLATFORM_DISPLAY_MAX_CELLS \
    (CORE_PLATFORM_DISPLAY_MAX_COLUMNS * CORE_PLATFORM_DISPLAY_MAX_ROWS)
#define CORE_PLATFORM_DISPLAY_GRAPHICS_WIDTH 320u
#define CORE_PLATFORM_DISPLAY_CGA_HIGH_RES_WIDTH 640u
#define CORE_PLATFORM_DISPLAY_GRAPHICS_HEIGHT 200u
#define CORE_PLATFORM_DISPLAY_EGA_HIGH_RES_HEIGHT 350u
#define CORE_PLATFORM_DISPLAY_MAX_PIXELS \
    (CORE_PLATFORM_DISPLAY_CGA_HIGH_RES_WIDTH * \
        CORE_PLATFORM_DISPLAY_EGA_HIGH_RES_HEIGHT)
#define CORE_PLATFORM_DISPLAY_PALETTE_ENTRIES 256u
#define CORE_PLATFORM_DISPLAY_TEXT_GLYPH_COUNT 256u
#define CORE_PLATFORM_DISPLAY_TEXT_GLYPH_ROWS 16u
#define CORE_PLATFORM_DISPLAY_TEXT_GLYPH_BYTES \
    (CORE_PLATFORM_DISPLAY_TEXT_GLYPH_COUNT * CORE_PLATFORM_DISPLAY_TEXT_GLYPH_ROWS)

typedef enum core_platform_display_kind {
    CORE_PLATFORM_DISPLAY_KIND_TEXT,
    CORE_PLATFORM_DISPLAY_KIND_INDEXED_PIXELS
} core_platform_display_kind;

typedef struct core_platform_display_frame {
    core_platform_display_kind kind;
    type_unsigned_8 characters[CORE_PLATFORM_DISPLAY_MAX_CELLS];
    type_unsigned_8 attributes[CORE_PLATFORM_DISPLAY_MAX_CELLS];
    type_unsigned_16 columns;
    type_unsigned_16 rows;
    type_unsigned_8 cursor_top;
    type_unsigned_8 cursor_bottom;
    /* Copied text coordinates are column then row. */
    type_unsigned_8 cursor_x;
    type_unsigned_8 cursor_y;
    C_INT cursor_visible;
    C_INT buffer_changed;
    C_INT cursor_changed;
    type_bool text_glyphs_present;
    type_unsigned_8 text_glyphs[CORE_PLATFORM_DISPLAY_TEXT_GLYPH_BYTES];
    type_unsigned_16 pixel_width;
    type_unsigned_16 pixel_height;
    type_unsigned_8 pixels[CORE_PLATFORM_DISPLAY_MAX_PIXELS];
    type_unsigned_32 palette_rgb[CORE_PLATFORM_DISPLAY_PALETTE_ENTRIES];
    type_unsigned_64 generation;
} core_platform_display_frame;

#endif
