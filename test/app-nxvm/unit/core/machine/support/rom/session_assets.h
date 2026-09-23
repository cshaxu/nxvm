#ifndef TEST_VM_SUPPORT_ROM_SESSION_ASSETS_H
#define TEST_VM_SUPPORT_ROM_SESSION_ASSETS_H
#include "lib/types/types_interface.h"

#include "type.h"

#include "app-nxvm/machine/machine_interface.h"

static inline C_VOID vm_test_default_pc_at_assets(vm_machine_assets *assets,
    lib_u8 rom[VM_PROFILE_EXTERNAL_PC_AT_ROM_BYTES])
{
    if (assets == LIB_NULL || rom == LIB_NULL) return;
    lib_memory_set(rom, 0, VM_PROFILE_EXTERNAL_PC_AT_ROM_BYTES);
    rom[VM_PROFILE_EXTERNAL_PC_AT_ROM_BYTES - 16u] = 0xf4u;
    *assets = (vm_machine_assets) { .bios = { { rom, VM_PROFILE_EXTERNAL_PC_AT_ROM_BYTES } } };
}

static inline type_status vm_test_default_pc_at_session_create(
    const vm_machine_config *requested, vm_machine **out_session)
{
    lib_u8 rom[VM_PROFILE_EXTERNAL_PC_AT_ROM_BYTES];
    vm_machine_assets assets;
    vm_machine_config config = requested == LIB_NULL ? (vm_machine_config) {0} :
        *requested;

    config.profile_kind = VM_MACHINE_PROFILE_DEFAULT_PC_AT;
    config.bios_count = 1u;
    config.bios_path[0u] = LIB_NULL;
    config.bios_path[1u] = LIB_NULL;
    config.cmos_seed = LIB_NULL;
    config.video_path = LIB_NULL;
    vm_test_default_pc_at_assets(&assets, rom);
    return vm_machine_create_from_assets(&config, &assets, out_session);
}

static inline C_VOID vm_test_ibm_5170_assets(vm_machine_assets *assets,
    lib_u8 even[VM_PROFILE_EXTERNAL_PC_AT_ROM_CHIP_BYTES],
    lib_u8 odd[VM_PROFILE_EXTERNAL_PC_AT_ROM_CHIP_BYTES])
{
    if (assets == LIB_NULL || even == LIB_NULL || odd == LIB_NULL) return;
    lib_memory_set(even, 0xff, VM_PROFILE_EXTERNAL_PC_AT_ROM_CHIP_BYTES);
    lib_memory_set(odd, 0xff, VM_PROFILE_EXTERNAL_PC_AT_ROM_CHIP_BYTES);
    even[VM_PROFILE_EXTERNAL_PC_AT_ROM_CHIP_BYTES - 8u] = 0xf4u;
    *assets = (vm_machine_assets) { .bios = {
        { even, VM_PROFILE_EXTERNAL_PC_AT_ROM_CHIP_BYTES },
        { odd, VM_PROFILE_EXTERNAL_PC_AT_ROM_CHIP_BYTES }
    } };
}

static inline type_status vm_test_ibm_5170_session_create(
    const vm_machine_config *requested, vm_machine **out_session)
{
    lib_u8 even[VM_PROFILE_EXTERNAL_PC_AT_ROM_CHIP_BYTES];
    lib_u8 odd[VM_PROFILE_EXTERNAL_PC_AT_ROM_CHIP_BYTES];
    vm_machine_assets assets;
    vm_machine_config config = requested == LIB_NULL ? (vm_machine_config) {0} :
        *requested;

    config.profile_kind = VM_MACHINE_PROFILE_IBM_5170_MODEL_339;
    config.bios_count = 2u;
    config.bios_path[0u] = LIB_NULL;
    config.bios_path[1u] = LIB_NULL;
    config.cmos_seed = LIB_NULL;
    config.video_path = LIB_NULL;
    vm_test_ibm_5170_assets(&assets, even, odd);
    return vm_machine_create_from_assets(&config, &assets, out_session);
}

#endif
