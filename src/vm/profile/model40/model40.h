#ifndef VM_PROFILE_MODEL40_H
#define VM_PROFILE_MODEL40_H

#include "type.h"
#include "core/machine/firmware_interface.h"

#define VM_PROFILE_MODEL40_ROM_BYTES (128u * 1024u)
#define VM_PROFILE_MODEL40_ROM_PHYSICAL_START 0x000e0000u

typedef struct vm_profile_model40_external_rom {
    const type_unsigned_8 *bytes;
    STD_SIZE_T byte_count;
} vm_profile_model40_external_rom;

C_INT vm_profile_model40_external_rom_is_valid(
    const vm_profile_model40_external_rom *rom);
const core_machine_firmware_provider *vm_profile_model40_firmware_provider(C_VOID);

#endif