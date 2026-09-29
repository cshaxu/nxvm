#ifndef X86_VIDEO_H
#define X86_VIDEO_H
#include "x86/devices/video/video_interface.h"
#define X86_VIDEO_VIDEO_BASE 0x000b8000u
#define X86_VIDEO_VIDEO_BYTES 0x00004000u
#define X86_VIDEO_TEXT_BASE X86_VIDEO_VIDEO_BASE
#define X86_VIDEO_TEXT_BYTES X86_VIDEO_VIDEO_BYTES
#define X86_VIDEO_CRTC_EGA_LAST 0x18u
#define X86_VIDEO_CRTC_REGISTER_COUNT \
    (X86_VIDEO_CRTC_EGA_LAST + 1u)
#define X86_VIDEO_EGA_APERTURE_BASE 0x000a0000u
#define X86_VIDEO_EGA_APERTURE_BYTES 0x00010000u
/* Register the full EGA CPU-decode span.  Graphics Controller register 6
 * selects a smaller active window inside this span. */
#define X86_VIDEO_EGA_CPU_DECODE_BYTES 0x00020000u
#define X86_VIDEO_SEQUENCER_REGISTER_COUNT 5u
#define X86_VIDEO_GRAPHICS_REGISTER_COUNT \
    X86_VIDEO_EGA_GRAPHICS_REGISTER_COUNT
#define X86_VIDEO_ATTRIBUTE_REGISTER_COUNT \
    X86_VIDEO_EGA_ATTRIBUTE_REGISTER_COUNT
#define X86_VIDEO_EGA_PLANES 4u
#define X86_VIDEO_EGA_PLANE_BYTES X86_VIDEO_EGA_APERTURE_BYTES
#define X86_VIDEO_EGA_ODD_EVEN_PAGE_BYTES 0x00004000u


typedef void *(*x86_video_allocate_zero)(void *context,
    lib_size count, lib_size byte_count);

typedef struct x86_video_data {
    x86_video_allocate_zero allocate_zero;
    void *allocate_context;
    lib_u8 crtc_index;
    lib_u8 crtc[X86_VIDEO_CRTC_REGISTER_COUNT];
    lib_u8 mode_control;
    lib_u8 color_select;
    x86_video_ega_personality ega_personality;
    lib_u8 ega_external_configured;
    lib_u8 ega_miscellaneous_output;
    lib_u8 ega_feature_control;
    x86_video_ega_sequencer_config ega_sequencer;
    lib_u8 sequencer_index;
    lib_u8 sequencer[X86_VIDEO_SEQUENCER_REGISTER_COUNT];
    lib_u8 ega_sequencer_configured;
    x86_video_ega_controller_config ega_controller;
    lib_u8 graphics_index;
    lib_u8 graphics[X86_VIDEO_GRAPHICS_REGISTER_COUNT];
    lib_u8 attribute_index;
    lib_u8 attribute[X86_VIDEO_ATTRIBUTE_REGISTER_COUNT];
    lib_u8 attribute_data_phase;
    lib_u8 attribute_display_enabled;
    lib_u8 ega_status_diagnostic_high;
    lib_u8 ega_controller_configured;
    lib_u8 vga_configured;
    lib_u8 vga_dac_mask;
    lib_u8 vga_dac_read_index;
    lib_u8 vga_dac_write_index;
    lib_u8 vga_dac_read_component;
    lib_u8 vga_dac_write_component;
    lib_u8 vga_dac[X86_VIDEO_PALETTE_ENTRIES][3u];
    lib_u8 ega_planar_enabled;
    lib_u8 *ega_planar_vram;
    lib_u8 ega_latches[X86_VIDEO_EGA_PLANES];
    lib_u64 captured_ega_dirty_generation;
    x86_video_text_timing text_timing;
    x86_video_text_glyph_config text_glyphs;
    lib_u32 raster_phase;
    lib_u8 crtc_initialized;
    lib_u8 cga_logical_raster_started;
    lib_u16 columns;
    lib_u16 rows;
    lib_i32 color_enabled;
    x86_video_cecg_config cecg;
    lib_u8 compaq_control_mode;
    lib_u8 compaq_feature_control;
    lib_u8 compaq_cpu_video_memory_disabled;
    lib_u8 compaq_color_io_base;
    lib_u8 compaq_clock_switch_select;
    lib_u8 compaq_odd_even_high_page;
    lib_u8 compaq_lightpen_latched;
    lib_u8 cga_lightpen_latched;
    lib_u64 dirty_generation;
    lib_i32 captured;
    x86_video_kind captured_kind;
    lib_u8 captured_mode_control;
    lib_u8 captured_color_select;
    lib_u8 text_cells[X86_VIDEO_MAX_COLUMNS *
        X86_VIDEO_MAX_ROWS * 2u];
    lib_u8 graphics_bytes[X86_VIDEO_VIDEO_BYTES];
    lib_u8 cga_vram[X86_VIDEO_VIDEO_BYTES];
    lib_u8 cga_memory_configured;
    lib_u8 characters[X86_VIDEO_MAX_COLUMNS *
        X86_VIDEO_MAX_ROWS];
    lib_u8 attributes[X86_VIDEO_MAX_COLUMNS *
        X86_VIDEO_MAX_ROWS];
    lib_u8 captured_cursor_top;
    lib_u8 captured_cursor_bottom;
    lib_u16 captured_cursor_address;
    lib_u8 captured_cursor_x;
    lib_u8 captured_cursor_y;
    lib_u16 captured_columns;
    lib_u16 captured_rows;
    lib_u8 captured_text_cell_height;
    lib_i32 captured_cursor_visible;
} x86_video_data;

struct x86_video {
    x86_video_data data;
};


void x86_video_set_allocate_zero(x86_video *adapter,
    x86_video_allocate_zero callback, void *context);
void x86_video_finalize(x86_video *adapter);
#endif
