#ifndef X86_NXVM_TEST_GUEST_DISPLAY_FRAME_H
#define X86_NXVM_TEST_GUEST_DISPLAY_FRAME_H
#include "lib/types/types_interface.h"
#include "x86-nxvm/chips/video/video_values_interface.h"



#define X86_NXVM_TEST_GUEST_DISPLAY_MAX_COLUMNS 80u
#define X86_NXVM_TEST_GUEST_DISPLAY_MAX_ROWS X86_VIDEO_MAX_ROWS
#define X86_NXVM_TEST_GUEST_DISPLAY_MAX_CELLS \
    (X86_NXVM_TEST_GUEST_DISPLAY_MAX_COLUMNS * X86_NXVM_TEST_GUEST_DISPLAY_MAX_ROWS)
#define X86_NXVM_TEST_GUEST_DISPLAY_GRAPHICS_WIDTH 320u
#define X86_NXVM_TEST_GUEST_DISPLAY_CGA_HIGH_RES_WIDTH 640u
#define X86_NXVM_TEST_GUEST_DISPLAY_GRAPHICS_HEIGHT 200u
#define X86_NXVM_TEST_GUEST_DISPLAY_EGA_HIGH_RES_HEIGHT 350u
#define X86_NXVM_TEST_GUEST_DISPLAY_MAX_PIXELS \
    (X86_NXVM_TEST_GUEST_DISPLAY_CGA_HIGH_RES_WIDTH * \
        X86_NXVM_TEST_GUEST_DISPLAY_EGA_HIGH_RES_HEIGHT)
#define X86_NXVM_TEST_GUEST_DISPLAY_PALETTE_ENTRIES 256u
#define X86_NXVM_TEST_GUEST_DISPLAY_TEXT_GLYPH_COUNT 256u
#define X86_NXVM_TEST_GUEST_DISPLAY_TEXT_GLYPH_ROWS X86_VIDEO_TEXT_GLYPH_ROWS
#define X86_NXVM_TEST_GUEST_DISPLAY_TEXT_GLYPH_BYTES \
    (X86_NXVM_TEST_GUEST_DISPLAY_TEXT_GLYPH_COUNT * X86_NXVM_TEST_GUEST_DISPLAY_TEXT_GLYPH_ROWS)

typedef enum x86_nxvm_test_guest_display_kind {
    X86_NXVM_TEST_GUEST_DISPLAY_KIND_TEXT,
    X86_NXVM_TEST_GUEST_DISPLAY_KIND_INDEXED_PIXELS
} x86_nxvm_test_guest_display_kind;

typedef struct x86_nxvm_test_guest_display_frame {
    x86_nxvm_test_guest_display_kind kind;
    lib_u8 characters[X86_NXVM_TEST_GUEST_DISPLAY_MAX_CELLS];
    lib_u8 attributes[X86_NXVM_TEST_GUEST_DISPLAY_MAX_CELLS];
    lib_u16 columns;
    lib_u16 rows;
    lib_u8 text_cell_height;
    lib_u8 cursor_top;
    lib_u8 cursor_bottom;
    /* Copied text coordinates are column then row. */
    lib_u8 cursor_x;
    lib_u8 cursor_y;
    lib_i32 cursor_visible;
    lib_i32 buffer_changed;
    lib_i32 cursor_changed;
    lib_u8 text_glyphs_present;
    lib_u8 text_glyphs[X86_NXVM_TEST_GUEST_DISPLAY_TEXT_GLYPH_BYTES];
    lib_u16 pixel_width;
    lib_u16 pixel_height;
    lib_u8 pixels[X86_NXVM_TEST_GUEST_DISPLAY_MAX_PIXELS];
    lib_u32 palette_rgb[X86_NXVM_TEST_GUEST_DISPLAY_PALETTE_ENTRIES];
    lib_u64 generation;
} x86_nxvm_test_guest_display_frame;

#endif
