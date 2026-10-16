/* Copyright 2012-2014 Neko. */
#include "lib/types/types_interface.h"


#include "x86-core/machine/machine_interface.h"
#include "x86-nxvm/board-base/pc_at_rom_interface.h"
#include "x86-nxvm/board-base/rom_validation_interface.h"
#include "x86-nxvm/board-base/rom_mapping_interface.h"

static lib_status x86_nxvm_board_external_pc_at_rom_configure(void *opaque,
    x86_core_machine_firmware_context *firmware)
{
    const x86_nxvm_board_external_pc_at_rom_context *context = opaque;

    x86_nxvm_board_rom_region regions[2u];
    lib_size count = 1u;

    if (context == LIB_NULL || context->image == LIB_NULL ||
        (context->video == LIB_NULL && context->video_bytes != 0u) ||
        (context->video != LIB_NULL && !x86_nxvm_board_byob_option_rom_is_valid(
            context->video, context->video_bytes,
            X86_NXVM_BOARD_EXTERNAL_PC_AT_VIDEO_ROM_MAX_BYTES))) {
        return LIB_STATUS_INVALID_ARGUMENT;
    }
    regions[0] = (x86_nxvm_board_rom_region) {0x000f0000u,
        context->image, X86_NXVM_BOARD_EXTERNAL_PC_AT_ROM_BYTES};
    if (context->video != LIB_NULL)
        regions[count++] = (x86_nxvm_board_rom_region) {0x000c0000u,
            context->video, context->video_bytes};
    return x86_nxvm_board_rom_register(firmware, regions, count, LIB_NULL, 0u);
}

static lib_status x86_nxvm_board_external_pc_at_rom_reset(void *opaque,
    x86_core_machine_firmware_context *firmware)
{
    (void)opaque;
    (void)firmware;
    return LIB_STATUS_OK;
}

static const x86_core_machine_firmware_provider x86_nxvm_board_external_pc_at_rom = {
    x86_nxvm_board_external_pc_at_rom_configure, x86_nxvm_board_external_pc_at_rom_reset,
    LIB_NULL
};

const x86_core_machine_firmware_provider *x86_nxvm_board_external_pc_at_rom_provider(void)
{
    return &x86_nxvm_board_external_pc_at_rom;
}
