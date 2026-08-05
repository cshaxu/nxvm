/* Copyright 2012-2014 Neko. */

#ifndef CORE_MACHINE_VADP_H
#define CORE_MACHINE_VADP_H

#ifdef __cplusplus
extern "C" {
#endif

#include "type.h"

#include "core/machine/display_interface.h"

#define CORE_MACHINE_DEVICE_VADP "CGA Text Adapter"
#define CORE_MACHINE_VADP_TEXT_BASE 0x000b8000u
#define CORE_MACHINE_VADP_TEXT_BYTES 0x00004000u
#define CORE_MACHINE_VADP_CRTC_REGISTER_COUNT 18u

typedef struct t_port t_port;
typedef struct t_ram t_ram;

typedef struct t_vadp_data {
    uint8_t crtc_index;
    uint8_t crtc[CORE_MACHINE_VADP_CRTC_REGISTER_COUNT];
    uint8_t mode_control;
    uint8_t color_select;
    uint8_t status;
    uint8_t refresh_phase;
    uint16_t columns;
    uint16_t rows;
    C_INT color_enabled;
    uint64_t dirty_generation;
    C_INT captured;
    uint8_t text_cells[CORE_MACHINE_DISPLAY_MAX_COLUMNS *
        CORE_MACHINE_DISPLAY_MAX_ROWS * 2u];
    uint8_t characters[CORE_MACHINE_DISPLAY_MAX_COLUMNS *
        CORE_MACHINE_DISPLAY_MAX_ROWS];
    uint8_t attributes[CORE_MACHINE_DISPLAY_MAX_COLUMNS *
        CORE_MACHINE_DISPLAY_MAX_ROWS];
    uint8_t cursor_top;
    uint8_t cursor_bottom;
    uint16_t cursor_address;
    uint8_t captured_cursor_top;
    uint8_t captured_cursor_bottom;
    uint16_t captured_cursor_address;
} t_vadp_data;

typedef struct t_vadp {
    t_vadp_data data;
} t_vadp;

C_VOID core_machine_vadp_initialize(t_vadp *adapter, t_port *port);
C_VOID core_machine_vadp_reset(t_vadp *adapter);
C_VOID core_machine_vadp_refresh(t_vadp *adapter, t_ram *memory);
C_VOID core_machine_vadp_finalize(t_vadp *adapter);

type_status core_machine_vadp_configure_text(t_vadp *adapter, uint8_t mode,
    uint16_t columns, uint16_t rows, C_INT color_enabled);
C_VOID core_machine_vadp_set_cursor_shape(t_vadp *adapter, uint8_t top,
    uint8_t bottom);
C_VOID core_machine_vadp_set_cursor_address(t_vadp *adapter, uint16_t address);
C_VOID core_machine_vadp_set_display_start(t_vadp *adapter, uint16_t address);
C_INT core_machine_vadp_capture_text_snapshot(t_vadp *adapter, t_ram *memory,
    core_machine_display_snapshot *out_snapshot);

#ifdef __cplusplus
}/*_EOCD_*/
#endif

#endif
