/* Machine display-provider binding; video values belong to the device. */
#ifndef CORE_MACHINE_DISPLAY_INTERFACE_H
#define CORE_MACHINE_DISPLAY_INTERFACE_H
#include "x86/chips/video/video_values_interface.h"

typedef void (*core_machine_display_provider)(void *context);

typedef lib_i32 (*core_machine_display_snapshot_provider)(void *context,
    x86_video_snapshot *out_snapshot);

typedef struct core_machine_display_provider_slot core_machine_display_provider_slot;

lib_status core_machine_display_provider_slot_create(
    core_machine_display_provider_slot **out_slot);
void core_machine_display_provider_slot_bind(
    core_machine_display_provider_slot *slot, void *mode_context,
    core_machine_display_provider mode_provider, void *snapshot_context,
    core_machine_display_snapshot_provider snapshot_provider);
void core_machine_display_provider_slot_freeze(
    core_machine_display_provider_slot *slot);
void core_machine_display_provider_slot_destroy(
    core_machine_display_provider_slot *slot);
lib_i32 core_machine_display_capture_snapshot_from(
    const core_machine_display_provider_slot *slot,
    x86_video_snapshot *out_snapshot);

#endif
