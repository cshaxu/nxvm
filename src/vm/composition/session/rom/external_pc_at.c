/* Copyright 2012-2014 Neko. */

#include "type.h"

#include "core/machine/machine_interface.h"
#include "vm/composition/session/rom/external_pc_at.h"
#include "vm/composition/session/session_private.h"
#include "vm/profile/byob/blob.h"

static type_status vm_session_external_pc_at_rom_configure(C_VOID *opaque,
    core_machine_firmware_context *firmware)
{
    const vm_session_external_pc_at_rom_context *context = opaque;

    type_status status;

    if (context == STD_NULL || context->image == STD_NULL ||
        (context->video == STD_NULL && context->video_bytes != 0u) ||
        (context->video != STD_NULL && !vm_profile_byob_option_rom_is_valid(
            context->video, context->video_bytes,
            VM_SESSION_PC_AT_VIDEO_ROM_MAX_BYTES))) {
        return TYPE_STATUS_INVALID_ARGUMENT;
    }
    status = core_machine_firmware_register_immutable_rom(firmware, 0x000f0000u,
        context->image, VM_SESSION_PC_AT_ROM_BYTES);
    if (status != TYPE_STATUS_OK || context->video == STD_NULL) return status;
    return core_machine_firmware_register_immutable_rom(firmware, 0x000c0000u,
        context->video, context->video_bytes);
}

static type_status vm_session_external_pc_at_rom_reset(C_VOID *opaque,
    core_machine_firmware_context *firmware)
{
    (C_VOID)opaque;
    (C_VOID)firmware;
    return TYPE_STATUS_OK;
}

static const core_machine_firmware_provider vm_session_external_pc_at_rom = {
    vm_session_external_pc_at_rom_configure, vm_session_external_pc_at_rom_reset,
    STD_NULL, STD_NULL
};

const core_machine_firmware_provider *vm_session_external_pc_at_rom_provider(C_VOID)
{
    return &vm_session_external_pc_at_rom;
}
