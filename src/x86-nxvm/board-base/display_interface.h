/* Machine display-provider binding; video values belong to the device. */
#ifndef X86_NXVM_BOARD_DISPLAY_INTERFACE_H
#define X86_NXVM_BOARD_DISPLAY_INTERFACE_H
#include "x86-nxvm/chips/video/video_values_interface.h"

typedef lib_status (*x86_nxvm_board_display_snapshot_provider)(void *context,
    x86_video_snapshot *out_snapshot);

typedef struct x86_nxvm_board_display_provider_slot x86_nxvm_board_display_provider_slot;

lib_status x86_nxvm_board_display_provider_slot_create(
    x86_nxvm_board_display_provider_slot **out_slot);
void x86_nxvm_board_display_provider_slot_bind(
    x86_nxvm_board_display_provider_slot *slot, void *snapshot_context,
    x86_nxvm_board_display_snapshot_provider snapshot_provider);
void x86_nxvm_board_display_provider_slot_freeze(
    x86_nxvm_board_display_provider_slot *slot);
void x86_nxvm_board_display_provider_slot_destroy(
    x86_nxvm_board_display_provider_slot *slot);
lib_status x86_nxvm_board_display_capture_snapshot_from(
    const x86_nxvm_board_display_provider_slot *slot,
    x86_video_snapshot *out_snapshot);

#endif
