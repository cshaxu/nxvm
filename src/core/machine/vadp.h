/* Copyright 2012-2014 Neko. */

#ifndef CORE_MACHINE_VADP_H
#define CORE_MACHINE_VADP_H

#ifdef __cplusplus
extern "C" {
#endif

#include "type.h"

#include "core/machine/display_interface.h"

#define CORE_MACHINE_DEVICE_VADP "CGA Adapter"
#define CORE_MACHINE_VADP_VIDEO_BASE 0x000b8000u
#define CORE_MACHINE_VADP_VIDEO_BYTES 0x00004000u
#define CORE_MACHINE_VADP_TEXT_BASE CORE_MACHINE_VADP_VIDEO_BASE
#define CORE_MACHINE_VADP_TEXT_BYTES CORE_MACHINE_VADP_VIDEO_BYTES
#define CORE_MACHINE_VADP_CRTC_EGA_LAST 0x18u
#define CORE_MACHINE_VADP_CRTC_REGISTER_COUNT \
    (CORE_MACHINE_VADP_CRTC_EGA_LAST + 1u)
#define CORE_MACHINE_VADP_EGA_APERTURE_BASE 0x000a0000u
#define CORE_MACHINE_VADP_EGA_APERTURE_BYTES 0x00010000u
/* Register the full EGA CPU-decode span.  Graphics Controller register 6
 * selects a smaller active window inside this span. */
#define CORE_MACHINE_VADP_EGA_CPU_DECODE_BYTES 0x00020000u
#define CORE_MACHINE_VADP_SEQUENCER_REGISTER_COUNT 5u
#define CORE_MACHINE_VADP_GRAPHICS_REGISTER_COUNT \
    CORE_MACHINE_DISPLAY_EGA_GRAPHICS_REGISTER_COUNT
#define CORE_MACHINE_VADP_ATTRIBUTE_REGISTER_COUNT \
    CORE_MACHINE_DISPLAY_EGA_ATTRIBUTE_REGISTER_COUNT
#define CORE_MACHINE_VADP_EGA_PLANES 4u
#define CORE_MACHINE_VADP_EGA_PLANE_BYTES CORE_MACHINE_VADP_EGA_APERTURE_BYTES
#define CORE_MACHINE_VADP_EGA_ODD_EVEN_PAGE_BYTES 0x00004000u
#define CORE_MACHINE_VADP_PORT_ATTRIBUTE 0x03c0u
#define CORE_MACHINE_VADP_PORT_ATTRIBUTE_DATA_READ 0x03c1u
#define CORE_MACHINE_VADP_PORT_SEQUENCER_INDEX 0x03c4u
#define CORE_MACHINE_VADP_PORT_SEQUENCER_DATA 0x03c5u
#define CORE_MACHINE_VADP_PORT_GRAPHICS_INDEX 0x03ceu
#define CORE_MACHINE_VADP_PORT_GRAPHICS_DATA 0x03cfu
#define CORE_MACHINE_VADP_PORT_MONO_CRTC_INDEX 0x03b4u
#define CORE_MACHINE_VADP_PORT_MONO_CRTC_DATA 0x03b5u
#define CORE_MACHINE_VADP_PORT_CRTC_INDEX 0x03d4u
#define CORE_MACHINE_VADP_PORT_CRTC_DATA 0x03d5u
#define CORE_MACHINE_VADP_PORT_MONO_STATUS 0x03bau
#define CORE_MACHINE_VADP_PORT_MONO_LIGHTPEN_LATCH_RESET 0x03bbu
#define CORE_MACHINE_VADP_PORT_MONO_LIGHTPEN_LATCH_SET 0x03bcu
#define CORE_MACHINE_VADP_PORT_MODE 0x03d8u
#define CORE_MACHINE_VADP_PORT_COLOR 0x03d9u
#define CORE_MACHINE_VADP_PORT_STATUS 0x03dau
#define CORE_MACHINE_VADP_PORT_EGA_MISCELLANEOUS_OUTPUT 0x03c2u
#define CORE_MACHINE_VADP_PORT_VGA_DAC_MASK 0x03c6u
#define CORE_MACHINE_VADP_PORT_VGA_DAC_READ_INDEX 0x03c7u
#define CORE_MACHINE_VADP_PORT_VGA_DAC_WRITE_INDEX 0x03c8u
#define CORE_MACHINE_VADP_PORT_VGA_DAC_DATA 0x03c9u
#define CORE_MACHINE_VADP_PORT_EGA_INPUT_STATUS_0 0x03c2u
#define CORE_MACHINE_VADP_PORT_EGA_FEATURE_CONTROL_MONO 0x03bau
#define CORE_MACHINE_VADP_PORT_EGA_FEATURE_CONTROL_COLOR 0x03dau
#define CORE_MACHINE_VADP_PORT_COMPAQ_MISCELLANEOUS_OUTPUT 0x03c2u
#define CORE_MACHINE_VADP_PORT_COMPAQ_FEATURE_CONTROL \
    CORE_MACHINE_VADP_PORT_STATUS
#define CORE_MACHINE_VADP_PORT_COMPAQ_CONTROL_MODE 0x03c6u
#define CORE_MACHINE_VADP_PORT_COMPAQ_LIGHTPEN_LATCH_RESET 0x03dbu
#define CORE_MACHINE_VADP_PORT_COMPAQ_LIGHTPEN_LATCH_SET 0x03dcu
#define CORE_MACHINE_VADP_PORT_COMPAQ_ENVIRONMENT 0x07c6u
#define CORE_MACHINE_VADP_PORT_COMPAQ_DISPLAY_TYPE 0x0bc6u
#define CORE_MACHINE_VADP_PORT_COMPAQ_INITIAL_MODE 0x0fc6u

typedef struct t_port t_port;
typedef struct t_ram t_ram;

typedef struct t_vadp_data {
    type_unsigned_8 crtc_index;
    type_unsigned_8 crtc[CORE_MACHINE_VADP_CRTC_REGISTER_COUNT];
    type_unsigned_8 mode_control;
    type_unsigned_8 color_select;
    core_machine_vadp_ega_personality ega_personality;
    type_bool ega_external_configured;
    type_unsigned_8 ega_miscellaneous_output;
    type_unsigned_8 ega_feature_control;
    core_machine_vadp_ega_sequencer_config ega_sequencer;
    type_unsigned_8 sequencer_index;
    type_unsigned_8 sequencer[CORE_MACHINE_VADP_SEQUENCER_REGISTER_COUNT];
    type_bool ega_sequencer_configured;
    core_machine_vadp_ega_controller_config ega_controller;
    type_unsigned_8 graphics_index;
    type_unsigned_8 graphics[CORE_MACHINE_VADP_GRAPHICS_REGISTER_COUNT];
    type_unsigned_8 attribute_index;
    type_unsigned_8 attribute[CORE_MACHINE_VADP_ATTRIBUTE_REGISTER_COUNT];
    type_bool attribute_data_phase;
    type_bool attribute_display_enabled;
    type_bool ega_status_diagnostic_high;
    type_bool ega_controller_configured;
    type_bool vga_configured;
    type_unsigned_8 vga_dac_mask;
    type_unsigned_8 vga_dac_read_index;
    type_unsigned_8 vga_dac_write_index;
    type_unsigned_8 vga_dac_read_component;
    type_unsigned_8 vga_dac_write_component;
    type_unsigned_8 vga_dac[CORE_MACHINE_DISPLAY_PALETTE_ENTRIES][3u];
    type_bool ega_planar_enabled;
    type_virtual_address ega_planar_vram;
    type_unsigned_8 ega_latches[CORE_MACHINE_VADP_EGA_PLANES];
    type_unsigned_64 captured_ega_dirty_generation;
    core_machine_vadp_text_timing text_timing;
    core_machine_vadp_text_glyph_config text_glyphs;
    type_unsigned_32 raster_phase;
    type_bool crtc_initialized;
    type_bool cga_logical_raster_started;
    type_unsigned_16 columns;
    type_unsigned_16 rows;
    C_INT color_enabled;
    core_machine_vadp_cecg_config cecg;
    type_unsigned_8 compaq_control_mode;
    type_unsigned_8 compaq_feature_control;
    type_bool compaq_cpu_video_memory_disabled;
    type_bool compaq_color_io_base;
    type_unsigned_8 compaq_clock_switch_select;
    type_bool compaq_odd_even_high_page;
    type_bool compaq_lightpen_latched;
    type_bool cga_lightpen_latched;
    type_unsigned_64 dirty_generation;
    C_INT captured;
    core_machine_display_kind captured_kind;
    type_unsigned_8 captured_mode_control;
    type_unsigned_8 captured_color_select;
    type_unsigned_8 text_cells[CORE_MACHINE_DISPLAY_MAX_COLUMNS *
        CORE_MACHINE_DISPLAY_MAX_ROWS * 2u];
    type_unsigned_8 graphics_bytes[CORE_MACHINE_VADP_VIDEO_BYTES];
    type_unsigned_8 cga_vram[CORE_MACHINE_VADP_VIDEO_BYTES];
    type_bool cga_memory_configured;
    type_unsigned_8 characters[CORE_MACHINE_DISPLAY_MAX_COLUMNS *
        CORE_MACHINE_DISPLAY_MAX_ROWS];
    type_unsigned_8 attributes[CORE_MACHINE_DISPLAY_MAX_COLUMNS *
        CORE_MACHINE_DISPLAY_MAX_ROWS];
    type_unsigned_8 captured_cursor_top;
    type_unsigned_8 captured_cursor_bottom;
    type_unsigned_16 captured_cursor_address;
    type_unsigned_8 captured_cursor_x;
    type_unsigned_8 captured_cursor_y;
    C_INT captured_cursor_visible;
} t_vadp_data;

typedef struct t_vadp {
    t_vadp_data data;
} t_vadp;

C_VOID core_machine_vadp_initialize(t_vadp *adapter, t_port *port);
C_VOID core_machine_vadp_configure_ega_ports(t_vadp *adapter, t_port *port);
type_status core_machine_vadp_configure_ega_personality(t_vadp *adapter,
    t_port *port, core_machine_vadp_ega_personality personality);
type_status core_machine_vadp_configure_vga(t_vadp *adapter, t_port *port);
C_INT core_machine_vadp_cecg_config_is_valid(
    const core_machine_vadp_cecg_config *config);
type_status core_machine_vadp_configure_cecg(t_vadp *adapter,
    const core_machine_vadp_cecg_config *config);
type_status core_machine_vadp_configure_cga_memory(t_vadp *adapter, t_ram *memory);
C_VOID core_machine_vadp_reset(t_vadp *adapter);
C_VOID core_machine_vadp_advance(t_vadp *adapter, t_ram *memory,
    type_unsigned_64 elapsed_ticks);
C_VOID core_machine_vadp_finalize(t_vadp *adapter);

type_status core_machine_vadp_configure_text(t_vadp *adapter, type_unsigned_8 mode,
    type_unsigned_16 columns, type_unsigned_16 rows, C_INT color_enabled);
type_status core_machine_vadp_configure_text_timing(t_vadp *adapter,
    const core_machine_vadp_text_timing *timing);
type_status core_machine_vadp_configure_text_glyphs(t_vadp *adapter,
    const core_machine_vadp_text_glyph_config *config);
type_status core_machine_vadp_configure_ega_sequencer(t_vadp *adapter,
    t_ram *memory, const core_machine_vadp_ega_sequencer_config *config);
type_status core_machine_vadp_configure_ega_controllers(t_vadp *adapter,
    const core_machine_vadp_ega_controller_config *config);
C_INT core_machine_vadp_ega_aperture_contains(const t_vadp *adapter,
    type_unsigned_32 physical, STD_SIZE_T bytes);
C_INT core_machine_vadp_capture_text_snapshot(t_vadp *adapter, t_ram *memory,
    core_machine_display_snapshot *out_snapshot);
C_VOID core_machine_vadp_observe_snapshot(const t_vadp *adapter,
    type_bool acknowledged_generation_valid,
    type_unsigned_64 acknowledged_generation,
    core_machine_display_snapshot_observation *out_observation);
C_INT core_machine_vadp_capture_snapshot(t_vadp *adapter, t_ram *memory,
    core_machine_display_snapshot *out_snapshot);

#ifdef __cplusplus
}/*_EOCD_*/
#endif

#endif
