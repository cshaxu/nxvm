#ifndef NTVDM64_CORE_PLATFORM_DISPLAY_FRAME_H
#define NTVDM64_CORE_PLATFORM_DISPLAY_FRAME_H

#include <stdint.h>

#define CORE_PLATFORM_DISPLAY_MAX_COLUMNS 80u
#define CORE_PLATFORM_DISPLAY_MAX_ROWS 25u
#define CORE_PLATFORM_DISPLAY_MAX_CELLS \
    (CORE_PLATFORM_DISPLAY_MAX_COLUMNS * CORE_PLATFORM_DISPLAY_MAX_ROWS)

typedef struct core_platform_display_frame {
    uint8_t characters[CORE_PLATFORM_DISPLAY_MAX_CELLS];
    uint8_t attributes[CORE_PLATFORM_DISPLAY_MAX_CELLS];
    uint16_t columns;
    uint16_t rows;
    uint8_t cursor_top;
    uint8_t cursor_bottom;
    uint8_t cursor_x;
    uint8_t cursor_y;
    int cursor_visible;
    int buffer_changed;
    int cursor_changed;
    uint64_t generation;
} core_platform_display_frame;

void core_platform_display_initialize(void);
void core_platform_display_publish(const core_platform_display_frame *frame);
void core_platform_display_capture(core_platform_display_frame *out_frame);

#endif
