#ifndef TEST_APP_MY5170_SESSION_ASSETS_H
#define TEST_APP_MY5170_SESSION_ASSETS_H

#include "app-my5170/machine_construction_interface.h"
#include "x86-nxvm/board-base/pc_at_rom_interface.h"
#include "x86-nxvm/machine/machine_interface.h"

static inline void vm_test_ibm_5170_assets(x86_nxvm_machine_assets *assets,
    lib_u8 even[X86_NXVM_BOARD_EXTERNAL_PC_AT_ROM_CHIP_BYTES],
    lib_u8 odd[X86_NXVM_BOARD_EXTERNAL_PC_AT_ROM_CHIP_BYTES])
{
    if (assets == LIB_NULL || even == LIB_NULL || odd == LIB_NULL) return;
    lib_memory_set(even, 0xff, X86_NXVM_BOARD_EXTERNAL_PC_AT_ROM_CHIP_BYTES);
    lib_memory_set(odd, 0xff, X86_NXVM_BOARD_EXTERNAL_PC_AT_ROM_CHIP_BYTES);
    even[X86_NXVM_BOARD_EXTERNAL_PC_AT_ROM_CHIP_BYTES - 8u] = 0xf4u;
    *assets = (x86_nxvm_machine_assets) { .bios = {
        { even, X86_NXVM_BOARD_EXTERNAL_PC_AT_ROM_CHIP_BYTES },
        { odd, X86_NXVM_BOARD_EXTERNAL_PC_AT_ROM_CHIP_BYTES }
    } };
}

static inline lib_status vm_test_ibm_5170_session_create(
    const x86_nxvm_machine_config *requested, x86_nxvm_machine **out_session)
{
    lib_u8 even[X86_NXVM_BOARD_EXTERNAL_PC_AT_ROM_CHIP_BYTES];
    lib_u8 odd[X86_NXVM_BOARD_EXTERNAL_PC_AT_ROM_CHIP_BYTES];
    x86_nxvm_machine_assets assets;
    x86_nxvm_machine_construction construction;
    x86_nxvm_machine_config config = requested == LIB_NULL ? (x86_nxvm_machine_config) {0} :
        *requested;
    lib_status status;

    if (out_session == LIB_NULL) return LIB_STATUS_INVALID_ARGUMENT;
    *out_session = LIB_NULL;
    config.bios_count = 2u;
    vm_test_ibm_5170_assets(&assets, even, odd);
    status = vm_profile_machine_plan_create_5170(&config, &assets, &construction);
    if (status != LIB_STATUS_OK) return status;
    return x86_nxvm_machine_create(&config, &construction, out_session);
}

#endif
