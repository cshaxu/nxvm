#include "lib/types/types_interface.h"
/* Copyright 2012-2014 Neko. */

#ifndef X86_NXVM_BOARD_PC_AT_ROM_INTERFACE_H
#define X86_NXVM_BOARD_PC_AT_ROM_INTERFACE_H

#include "x86-core/machine/firmware_interface.h"
#include "x86-nxvm/board-base/rom_validation_interface.h"

#define X86_NXVM_BOARD_EXTERNAL_PC_AT_ROM_BYTES (64u * 1024u)
#define X86_NXVM_BOARD_EXTERNAL_PC_AT_ROM_CHIP_BYTES (32u * 1024u)
#define X86_NXVM_BOARD_EXTERNAL_PC_AT_VIDEO_ROM_MAX_BYTES X86_NXVM_BOARD_BYOB_OPTION_ROM_MAX_BYTES

typedef struct x86_nxvm_board_external_pc_at_rom_context {
    const lib_u8 *image;
    const lib_u8 *video;
    lib_size video_bytes;
} x86_nxvm_board_external_pc_at_rom_context;

const x86_core_machine_firmware_provider *x86_nxvm_board_external_pc_at_rom_provider(void);

#endif
