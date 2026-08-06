/* Product-neutral display-mode notification and text snapshot contracts. */
#ifndef CORE_MACHINE_DISPLAY_INTERFACE_H
#define CORE_MACHINE_DISPLAY_INTERFACE_H

#include "type.h"


#define CORE_MACHINE_DISPLAY_MAX_COLUMNS 80u
#define CORE_MACHINE_DISPLAY_MAX_ROWS 25u
#define CORE_MACHINE_DISPLAY_GRAPHICS_WIDTH 320u
#define CORE_MACHINE_DISPLAY_GRAPHICS_HEIGHT 200u
#define CORE_MACHINE_DISPLAY_MAX_PIXELS \
    (CORE_MACHINE_DISPLAY_GRAPHICS_WIDTH * CORE_MACHINE_DISPLAY_GRAPHICS_HEIGHT)
#define CORE_MACHINE_DISPLAY_PALETTE_ENTRIES 16u

typedef enum core_machine_display_kind {
    CORE_MACHINE_DISPLAY_KIND_TEXT,
    CORE_MACHINE_DISPLAY_KIND_CGA_320X200X4,
    CORE_MACHINE_DISPLAY_KIND_EGA_320X200X16
} core_machine_display_kind;

typedef C_VOID (*core_machine_display_provider)(C_VOID *context);

typedef struct core_machine_display_snapshot {
    core_machine_display_kind kind;
    uint16_t columns;
    uint16_t rows;
    uint8_t cursor_top;
    uint8_t cursor_bottom;
    uint8_t cursor_x;
    uint8_t cursor_y;
    C_INT cursor_visible;
    C_INT buffer_changed;
    C_INT cursor_changed;
    uint8_t characters[CORE_MACHINE_DISPLAY_MAX_COLUMNS * CORE_MACHINE_DISPLAY_MAX_ROWS];
    uint8_t attributes[CORE_MACHINE_DISPLAY_MAX_COLUMNS * CORE_MACHINE_DISPLAY_MAX_ROWS];
    uint16_t pixel_width;
    uint16_t pixel_height;
    uint8_t pixels[CORE_MACHINE_DISPLAY_MAX_PIXELS];
    uint32_t palette_rgb[CORE_MACHINE_DISPLAY_PALETTE_ENTRIES];
} core_machine_display_snapshot;

typedef C_INT (*core_machine_display_snapshot_provider)(C_VOID *context,
    core_machine_display_snapshot *out_snapshot);

typedef struct core_machine_display_provider_slot {
    C_VOID *mode_context;
    core_machine_display_provider mode_provider;
    C_VOID *snapshot_context;
    core_machine_display_snapshot_provider snapshot_provider;
    C_INT frozen;
} core_machine_display_provider_slot;

C_VOID core_machine_display_provider_slot_initialize(
    core_machine_display_provider_slot *slot);
C_VOID core_machine_display_provider_slot_bind(
    core_machine_display_provider_slot *slot, C_VOID *mode_context,
    core_machine_display_provider mode_provider, C_VOID *snapshot_context,
    core_machine_display_snapshot_provider snapshot_provider);
C_VOID core_machine_display_provider_slot_freeze(
    core_machine_display_provider_slot *slot);
C_VOID core_machine_display_provider_slot_finalize(
    core_machine_display_provider_slot *slot);
C_VOID core_machine_display_notify_mode_changed_to(
    const core_machine_display_provider_slot *slot);
C_INT core_machine_display_capture_snapshot_from(
    const core_machine_display_provider_slot *slot,
    core_machine_display_snapshot *out_snapshot);

#endif
