#include "lib/types/types_interface.h"
/* Copyright 2012-2014 Neko. */

#ifndef VM_PROFILE_EXTERNAL_PC_AT_ROM_H
#define VM_PROFILE_EXTERNAL_PC_AT_ROM_H

#include "app-nxvm/devices/firmware_interface.h"
#include "app-nxvm/profiles/byob/blob.h"

#define VM_PROFILE_EXTERNAL_PC_AT_ROM_BYTES (64u * 1024u)
#define VM_PROFILE_EXTERNAL_PC_AT_ROM_CHIP_BYTES (32u * 1024u)
#define VM_PROFILE_EXTERNAL_PC_AT_VIDEO_ROM_MAX_BYTES VM_PROFILE_BYOB_OPTION_ROM_MAX_BYTES

typedef struct vm_profile_external_pc_at_rom_context {
    const lib_u8 *image;
    const lib_u8 *video;
    lib_size video_bytes;
} vm_profile_external_pc_at_rom_context;

const core_machine_firmware_provider *vm_profile_external_pc_at_rom_provider(void);

#endif
