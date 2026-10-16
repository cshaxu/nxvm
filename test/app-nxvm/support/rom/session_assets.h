#ifndef TEST_APP_NXVM_ROM_SESSION_ASSETS_H
#define TEST_APP_NXVM_ROM_SESSION_ASSETS_H

#include "../profile.h"
#include "x86-nxvm/board-base/pc_at_rom_interface.h"

static inline void vm_test_default_pc_at_assets(x86_nxvm_machine_assets *assets,
    lib_u8 rom[X86_NXVM_BOARD_EXTERNAL_PC_AT_ROM_BYTES])
{
    if (assets == LIB_NULL || rom == LIB_NULL) return;
    lib_memory_set(rom, 0, X86_NXVM_BOARD_EXTERNAL_PC_AT_ROM_BYTES);
    rom[X86_NXVM_BOARD_EXTERNAL_PC_AT_ROM_BYTES - 16u] = 0xf4u;
    *assets = (x86_nxvm_machine_assets) { .bios = { { rom, X86_NXVM_BOARD_EXTERNAL_PC_AT_ROM_BYTES } } };
}

static inline lib_status vm_test_default_pc_at_session_create(
    const x86_nxvm_machine_config *requested, x86_nxvm_machine **out_session)
{
    lib_u8 rom[X86_NXVM_BOARD_EXTERNAL_PC_AT_ROM_BYTES];
    x86_nxvm_machine_assets assets;
    x86_nxvm_machine_config config = requested == LIB_NULL ? (x86_nxvm_machine_config) {0} :
        *requested;

    config.bios_count = 1u;
    vm_test_default_pc_at_assets(&assets, rom);
    return vm_test_machine_create_from_assets(VM_MACHINE_PROFILE_DEFAULT_PC_AT,
        &config, &assets, out_session);
}

#endif
