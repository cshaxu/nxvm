/* Copyright 2012-2014 Neko. */

#ifndef VM_MACHINE_ROM_EXTERNAL_PC_AT_H
#define VM_MACHINE_ROM_EXTERNAL_PC_AT_H

#include "core/machine/firmware_interface.h"

typedef struct vm_machine_external_pc_at_rom_context {
    const type_unsigned_8 *image;
    const type_unsigned_8 *video;
    STD_SIZE_T video_bytes;
} vm_machine_external_pc_at_rom_context;

const core_machine_firmware_provider *vm_machine_external_pc_at_rom_provider(C_VOID);

#endif
