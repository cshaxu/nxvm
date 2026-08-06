#ifndef CORE_PLATFORM_DISPLAY_FRAME_H
#define CORE_PLATFORM_DISPLAY_FRAME_H

#include "type.h"


#define CORE_PLATFORM_DISPLAY_MAX_COLUMNS 80u
#define CORE_PLATFORM_DISPLAY_MAX_ROWS 25u
#define CORE_PLATFORM_DISPLAY_MAX_CELLS \
    (CORE_PLATFORM_DISPLAY_MAX_COLUMNS * CORE_PLATFORM_DISPLAY_MAX_ROWS)
#define CORE_PLATFORM_DISPLAY_GRAPHICS_WIDTH 320u
#define CORE_PLATFORM_DISPLAY_GRAPHICS_HEIGHT 200u
#define CORE_PLATFORM_DISPLAY_MAX_PIXELS \
    (CORE_PLATFORM_DISPLAY_GRAPHICS_WIDTH * CORE_PLATFORM_DISPLAY_GRAPHICS_HEIGHT)
#define CORE_PLATFORM_DISPLAY_PALETTE_ENTRIES 16u

typedef enum core_platform_display_kind {
    CORE_PLATFORM_DISPLAY_KIND_TEXT,
    CORE_PLATFORM_DISPLAY_KIND_INDEXED_PIXELS
} core_platform_display_kind;

typedef struct core_platform_display_frame {
    core_platform_display_kind kind;
    uint8_t characters[CORE_PLATFORM_DISPLAY_MAX_CELLS];
    uint8_t attributes[CORE_PLATFORM_DISPLAY_MAX_CELLS];
    uint16_t columns;
    uint16_t rows;
    uint8_t cursor_top;
    uint8_t cursor_bottom;
    uint8_t cursor_x;
    uint8_t cursor_y;
    C_INT cursor_visible;
    C_INT buffer_changed;
    C_INT cursor_changed;
    uint16_t pixel_width;
    uint16_t pixel_height;
    uint8_t pixels[CORE_PLATFORM_DISPLAY_MAX_PIXELS];
    uint32_t palette_rgb[CORE_PLATFORM_DISPLAY_PALETTE_ENTRIES];
    uint64_t generation;
} core_platform_display_frame;

#endif
