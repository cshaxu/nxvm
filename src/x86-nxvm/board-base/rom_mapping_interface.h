#ifndef X86_NXVM_BOARD_ROM_MAPPING_INTERFACE_H
#define X86_NXVM_BOARD_ROM_MAPPING_INTERFACE_H

#include "x86-core/machine/firmware_interface.h"

typedef struct x86_nxvm_board_rom_region {
    lib_u32 physical_start;
    const lib_u8 *image;
    lib_size bytes;
} x86_nxvm_board_rom_region;

typedef struct x86_nxvm_board_rom_alias {
    lib_u32 source_start;
    lib_u32 physical_start;
    lib_size bytes;
} x86_nxvm_board_rom_alias;

/* Configure-only: register copied regions, then aliases in declared order.
 * Stops on the first Core error; the existing construction transaction owns
 * rollback. Addresses, source bytes and alias selection belong to the App. */
lib_status x86_nxvm_board_rom_register(x86_core_machine_firmware_context *firmware,
    const x86_nxvm_board_rom_region *regions, lib_size region_count,
    const x86_nxvm_board_rom_alias *aliases, lib_size alias_count);

#endif
