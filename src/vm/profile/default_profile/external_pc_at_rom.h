/* Copyright 2012-2014 Neko. */

#ifndef VM_PROFILE_EXTERNAL_PC_AT_ROM_H
#define VM_PROFILE_EXTERNAL_PC_AT_ROM_H

#include "core/machine/firmware_interface.h"
#include "vm/profile/byob/blob.h"

#define VM_PROFILE_EXTERNAL_PC_AT_ROM_BYTES (64u * 1024u)
#define VM_PROFILE_EXTERNAL_PC_AT_ROM_CHIP_BYTES (32u * 1024u)
#define VM_PROFILE_EXTERNAL_PC_AT_VIDEO_ROM_MAX_BYTES VM_PROFILE_BYOB_OPTION_ROM_MAX_BYTES

typedef struct vm_profile_external_pc_at_rom_context {
    const type_unsigned_8 *image;
    const type_unsigned_8 *video;
    STD_SIZE_T video_bytes;
} vm_profile_external_pc_at_rom_context;

const core_machine_firmware_provider *vm_profile_external_pc_at_rom_provider(C_VOID);

#endif
