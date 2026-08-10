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
#define CORE_MACHINE_VADP_CRTC_REGISTER_COUNT 20u
#define CORE_MACHINE_VADP_EGA_APERTURE_BASE 0x000a0000u
#define CORE_MACHINE_VADP_EGA_APERTURE_BYTES 0x00010000u
#define CORE_MACHINE_VADP_SEQUENCER_REGISTER_COUNT 5u
#define CORE_MACHINE_VADP_GRAPHICS_REGISTER_COUNT \
    CORE_MACHINE_DISPLAY_EGA_GRAPHICS_REGISTER_COUNT
#define CORE_MACHINE_VADP_ATTRIBUTE_REGISTER_COUNT \
    CORE_MACHINE_DISPLAY_EGA_ATTRIBUTE_REGISTER_COUNT
#define CORE_MACHINE_VADP_EGA_PLANES 4u
#define CORE_MACHINE_VADP_EGA_PLANE_BYTES CORE_MACHINE_VADP_EGA_APERTURE_BYTES
#define CORE_MACHINE_VADP_PORT_ATTRIBUTE 0x03c0u
#define CORE_MACHINE_VADP_PORT_ATTRIBUTE_DATA_READ 0x03c1u
#define CORE_MACHINE_VADP_PORT_SEQUENCER_INDEX 0x03c4u
#define CORE_MACHINE_VADP_PORT_SEQUENCER_DATA 0x03c5u
#define CORE_MACHINE_VADP_PORT_GRAPHICS_INDEX 0x03ceu
#define CORE_MACHINE_VADP_PORT_GRAPHICS_DATA 0x03cfu
#define CORE_MACHINE_VADP_PORT_CRTC_INDEX 0x03d4u
#define CORE_MACHINE_VADP_PORT_CRTC_DATA 0x03d5u
#define CORE_MACHINE_VADP_PORT_MODE 0x03d8u
#define CORE_MACHINE_VADP_PORT_COLOR 0x03d9u
#define CORE_MACHINE_VADP_PORT_STATUS 0x03dau

typedef struct t_port t_port;
typedef struct t_ram t_ram;

typedef struct t_vadp_data {
    uint8_t crtc_index;
    uint8_t crtc[CORE_MACHINE_VADP_CRTC_REGISTER_COUNT];
    uint8_t mode_control;
    uint8_t color_select;
    core_machine_vadp_ega_sequencer_config ega_sequencer;
    uint8_t sequencer_index;
    uint8_t sequencer[CORE_MACHINE_VADP_SEQUENCER_REGISTER_COUNT];
    type_bool ega_sequencer_configured;
    core_machine_vadp_ega_controller_config ega_controller;
    uint8_t graphics_index;
    uint8_t graphics[CORE_MACHINE_VADP_GRAPHICS_REGISTER_COUNT];
    uint8_t attribute_index;
    uint8_t attribute[CORE_MACHINE_VADP_ATTRIBUTE_REGISTER_COUNT];
    type_bool attribute_data_phase;
    type_bool attribute_display_enabled;
    type_bool ega_controller_configured;
    type_bool ega_planar_enabled;
    type_bool ega_planar_armed;
    type_virtual_address ega_planar_vram;
    uint8_t ega_latches[CORE_MACHINE_VADP_EGA_PLANES];
    uint64_t captured_ega_dirty_generation;
    core_machine_vadp_text_timing text_timing;
    uint32_t raster_phase;
    uint16_t columns;
    uint16_t rows;
    C_INT color_enabled;
    uint64_t dirty_generation;
    C_INT captured;
    core_machine_display_kind captured_kind;
    uint8_t captured_mode_control;
    uint8_t captured_color_select;
    uint8_t text_cells[CORE_MACHINE_DISPLAY_MAX_COLUMNS *
        CORE_MACHINE_DISPLAY_MAX_ROWS * 2u];
    uint8_t graphics_bytes[CORE_MACHINE_VADP_VIDEO_BYTES];
    uint8_t characters[CORE_MACHINE_DISPLAY_MAX_COLUMNS *
        CORE_MACHINE_DISPLAY_MAX_ROWS];
    uint8_t attributes[CORE_MACHINE_DISPLAY_MAX_COLUMNS *
        CORE_MACHINE_DISPLAY_MAX_ROWS];
    uint8_t captured_cursor_top;
    uint8_t captured_cursor_bottom;
    uint16_t captured_cursor_address;
    uint8_t captured_cursor_x;
    uint8_t captured_cursor_y;
    C_INT captured_cursor_visible;
} t_vadp_data;

typedef struct t_vadp {
    t_vadp_data data;
} t_vadp;

C_VOID core_machine_vadp_initialize(t_vadp *adapter, t_port *port);
C_VOID core_machine_vadp_reset(t_vadp *adapter);
C_VOID core_machine_vadp_advance(t_vadp *adapter, t_ram *memory,
    uint64_t elapsed_ticks);
C_VOID core_machine_vadp_finalize(t_vadp *adapter);

type_status core_machine_vadp_configure_text(t_vadp *adapter, uint8_t mode,
    uint16_t columns, uint16_t rows, C_INT color_enabled);
type_status core_machine_vadp_configure_text_timing(t_vadp *adapter,
    const core_machine_vadp_text_timing *timing);
type_status core_machine_vadp_configure_ega_sequencer(t_vadp *adapter,
    t_ram *memory, const core_machine_vadp_ega_sequencer_config *config);
type_status core_machine_vadp_configure_ega_controllers(t_vadp *adapter,
    const core_machine_vadp_ega_controller_config *config);
C_INT core_machine_vadp_ega_aperture_contains(const t_vadp *adapter,
    uint32_t physical, STD_SIZE_T bytes);
C_INT core_machine_vadp_capture_text_snapshot(t_vadp *adapter, t_ram *memory,
    core_machine_display_snapshot *out_snapshot);
C_INT core_machine_vadp_capture_snapshot(t_vadp *adapter, t_ram *memory,
    core_machine_display_snapshot *out_snapshot);

#ifdef __cplusplus
}/*_EOCD_*/
#endif

#endif
