/* Copyright 2012-2026 Neko. */
#ifndef X86_VIDEO_VALUES_INTERFACE_H
#define X86_VIDEO_VALUES_INTERFACE_H
#include "lib/types/types_interface.h"

/* Synchronous borrowed backing-memory access. The reader and destination are
 * valid only for the capture call; failure leaves the capture unsuccessful. */
typedef lib_status (*x86_video_read_bytes)(void *context, lib_u32 address,
    lib_u8 *destination, lib_size byte_count);

typedef struct x86_video_memory_reader {
    x86_video_read_bytes read;
    void *context;
} x86_video_memory_reader;

#define X86_VIDEO_MAX_COLUMNS 80u
#define X86_VIDEO_MAX_ROWS 25u
#define X86_VIDEO_GRAPHICS_WIDTH 320u
#define X86_VIDEO_CGA_HIGH_RES_WIDTH 640u
#define X86_VIDEO_GRAPHICS_HEIGHT 200u
#define X86_VIDEO_EGA_HIGH_RES_HEIGHT 350u
#define X86_VIDEO_MAX_PIXELS \
    (X86_VIDEO_CGA_HIGH_RES_WIDTH * \
        X86_VIDEO_EGA_HIGH_RES_HEIGHT)
#define X86_VIDEO_PALETTE_ENTRIES 256u
#define X86_VIDEO_EGA_GRAPHICS_REGISTER_COUNT 9u
#define X86_VIDEO_EGA_ATTRIBUTE_REGISTER_COUNT 21u
#define X86_VIDEO_TEXT_GLYPH_COUNT 256u
#define X86_VIDEO_TEXT_GLYPH_ROWS 16u
#define X86_VIDEO_TEXT_GLYPH_BYTES \
    (X86_VIDEO_TEXT_GLYPH_COUNT * X86_VIDEO_TEXT_GLYPH_ROWS)

typedef struct x86_video_text_timing {
    lib_u32 active_display_ticks;
    lib_u32 horizontal_blank_ticks;
    lib_u32 vertical_retrace_ticks;
} x86_video_text_timing;

/* A construction-time character generator is normalized by the caller;
 * VADP thereafter owns the copied 8x16 glyph state exposed to presenters. */
typedef struct x86_video_text_glyph_config {
    lib_u8 present;
    lib_u8 bytes[X86_VIDEO_TEXT_GLYPH_BYTES];
} x86_video_text_glyph_config;

typedef struct x86_video_ega_sequencer_config {
    lib_u32 aperture_base;
    lib_u32 aperture_bytes;
    lib_u8 reset;
    lib_u8 clocking_mode;
    lib_u8 map_mask;
    lib_u8 memory_mode;
    lib_u8 planar_ega;
} x86_video_ega_sequencer_config;

typedef enum x86_video_ega_personality {
    X86_VIDEO_EGA_PERSONALITY_GENERIC = 0,
    X86_VIDEO_EGA_PERSONALITY_COMPAQ_ENHANCED_COLOR
} x86_video_ega_personality;

/* Composition declares board-fixed CECG switch state; VADP owns the
 * resulting register state and reset behavior. */
typedef struct x86_video_cecg_config {
    lib_u8 control_mode;
    lib_u8 environment;
    lib_u8 display_type;
    lib_u8 initial_mode;
    lib_u8 lightpen_switch_open;
    lib_u8 cpu_video_memory_disabled;
    lib_u8 color_io_base;
    lib_u8 sw1_closed_mask;
    lib_u8 clock_switch_select;
    lib_u8 special_features_present;
    lib_u8 vertical_retrace_irq_enabled;
    lib_u8 odd_even_high_page;
} x86_video_cecg_config;

typedef struct x86_video_ega_controller_config {
    lib_u8 graphics[X86_VIDEO_EGA_GRAPHICS_REGISTER_COUNT];
    lib_u8 attribute[X86_VIDEO_EGA_ATTRIBUTE_REGISTER_COUNT];
} x86_video_ega_controller_config;

typedef enum x86_video_kind {
    X86_VIDEO_KIND_TEXT,
    X86_VIDEO_KIND_CGA_320X200X4,
    X86_VIDEO_KIND_CGA_640X200X2,
    X86_VIDEO_KIND_EGA_320X200X16,
    X86_VIDEO_KIND_EGA_640X200X16,
    X86_VIDEO_KIND_EGA_640X350X16,
    X86_VIDEO_KIND_VGA_320X200X256
} x86_video_kind;

typedef struct x86_video_snapshot {
    x86_video_kind kind;
    lib_u16 columns;
    lib_u16 rows;
    /* Current CRTC character-cell scan-line count, not a font-asset default. */
    lib_u8 text_cell_height;
    lib_u8 cursor_top;
    lib_u8 cursor_bottom;
    /* Text coordinates are column then row, relative to display start. */
    lib_u8 cursor_x;
    lib_u8 cursor_y;
    lib_i32 cursor_visible;
    lib_i32 buffer_changed;
    lib_i32 cursor_changed;
    lib_u8 text_glyphs_present;
    lib_u8 text_glyphs[X86_VIDEO_TEXT_GLYPH_BYTES];
    lib_u8 characters[X86_VIDEO_MAX_COLUMNS * X86_VIDEO_MAX_ROWS];
    lib_u8 attributes[X86_VIDEO_MAX_COLUMNS * X86_VIDEO_MAX_ROWS];
    lib_u16 pixel_width;
    lib_u16 pixel_height;
    lib_u8 pixels[X86_VIDEO_MAX_PIXELS];
    lib_u32 palette_rgb[X86_VIDEO_PALETTE_ENTRIES];
} x86_video_snapshot;

/* A copied-frame consumer may acknowledge this opaque generation only after a
 * successful publish.  VADP offers it only when it owns and observes every
 * input to the selected frame (CGA/text VRAM, EGA planar, or VGA chain-4).
 * Other display paths retain normal capture. */
typedef struct x86_video_snapshot_observation {
    lib_u64 generation;
    lib_u8 generation_reliable;
    lib_u8 capture_required;
} x86_video_snapshot_observation;

#endif
