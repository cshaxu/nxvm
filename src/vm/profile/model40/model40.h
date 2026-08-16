#ifndef VM_PROFILE_MODEL40_H
#define VM_PROFILE_MODEL40_H

#include "type.h"
#include "core/machine/firmware_interface.h"

#define VM_PROFILE_MODEL40_ROM_CHIP_BYTES (16u * 1024u)
#define VM_PROFILE_MODEL40_ROM_LOGICAL_BYTES (2u * VM_PROFILE_MODEL40_ROM_CHIP_BYTES)
#define VM_PROFILE_MODEL40_ROM_WINDOW_BYTES (2u * VM_PROFILE_MODEL40_ROM_LOGICAL_BYTES)
#define VM_PROFILE_MODEL40_ROM_LOW_PHYSICAL_START 0x000f0000u
#define VM_PROFILE_MODEL40_ROM_HIGH_PHYSICAL_START 0xffef0000u

typedef struct vm_profile_model40_external_rom {
    const type_unsigned_8 *even_bytes;
    const type_unsigned_8 *odd_bytes;
    STD_SIZE_T chip_byte_count;
} vm_profile_model40_external_rom;

C_INT vm_profile_model40_external_rom_is_valid(
    const vm_profile_model40_external_rom *rom);
const core_machine_firmware_provider *vm_profile_model40_firmware_provider(C_VOID);

#endif
