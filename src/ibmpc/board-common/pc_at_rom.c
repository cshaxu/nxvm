/* Copyright 2012-2014 Neko. */
#include "lib/types/types_interface.h"


#include "x86/core/machine_interface.h"
#include "ibmpc/board-common/pc_at_rom_interface.h"
#include "ibmpc/board-common/rom_validation_interface.h"
#include "ibmpc/board-common/rom_mapping_interface.h"

static lib_status vm_profile_external_pc_at_rom_configure(void *opaque,
    core_machine_firmware_context *firmware)
{
    const vm_profile_external_pc_at_rom_context *context = opaque;

    vm_profile_rom_region regions[2u];
    lib_size count = 1u;

    if (context == LIB_NULL || context->image == LIB_NULL ||
        (context->video == LIB_NULL && context->video_bytes != 0u) ||
        (context->video != LIB_NULL && !vm_profile_byob_option_rom_is_valid(
            context->video, context->video_bytes,
            VM_PROFILE_EXTERNAL_PC_AT_VIDEO_ROM_MAX_BYTES))) {
        return LIB_STATUS_INVALID_ARGUMENT;
    }
    regions[0] = (vm_profile_rom_region) {0x000f0000u,
        context->image, VM_PROFILE_EXTERNAL_PC_AT_ROM_BYTES};
    if (context->video != LIB_NULL)
        regions[count++] = (vm_profile_rom_region) {0x000c0000u,
            context->video, context->video_bytes};
    return vm_profile_rom_register(firmware, regions, count, LIB_NULL, 0u);
}

static lib_status vm_profile_external_pc_at_rom_reset(void *opaque,
    core_machine_firmware_context *firmware)
{
    (void)opaque;
    (void)firmware;
    return LIB_STATUS_OK;
}

static const core_machine_firmware_provider vm_profile_external_pc_at_rom = {
    vm_profile_external_pc_at_rom_configure, vm_profile_external_pc_at_rom_reset,
    LIB_NULL
};

const core_machine_firmware_provider *vm_profile_external_pc_at_rom_provider(void)
{
    return &vm_profile_external_pc_at_rom;
}
