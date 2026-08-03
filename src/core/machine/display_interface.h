/* Product-neutral display-mode notification and text snapshot contracts. */
#ifndef NTVDM64_CORE_MACHINE_DISPLAY_INTERFACE_H
#define NTVDM64_CORE_MACHINE_DISPLAY_INTERFACE_H

#include "type.h"

#include <stdint.h>

#define CORE_MACHINE_DISPLAY_MAX_COLUMNS 80u
#define CORE_MACHINE_DISPLAY_MAX_ROWS 25u

typedef C_VOID (*core_machine_display_provider)(C_VOID *context);

typedef struct core_machine_display_snapshot {
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
