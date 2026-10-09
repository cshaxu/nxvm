#ifndef VM_PROFILE_ROM_MAPPING_INTERFACE_H
#define VM_PROFILE_ROM_MAPPING_INTERFACE_H

#include "core/x86/firmware_interface.h"

typedef struct vm_profile_rom_region {
    lib_u32 physical_start;
    const lib_u8 *image;
    lib_size bytes;
} vm_profile_rom_region;

typedef struct vm_profile_rom_alias {
    lib_u32 source_start;
    lib_u32 physical_start;
    lib_size bytes;
} vm_profile_rom_alias;

/* Configure-only: register copied regions, then aliases in declared order.
 * Stops on the first Core error; the existing construction transaction owns
 * rollback. Addresses, source bytes and alias selection belong to the App. */
lib_status vm_profile_rom_register(core_machine_firmware_context *firmware,
    const vm_profile_rom_region *regions, lib_size region_count,
    const vm_profile_rom_alias *aliases, lib_size alias_count);

#endif
