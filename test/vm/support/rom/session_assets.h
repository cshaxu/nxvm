#ifndef TEST_VM_SUPPORT_ROM_SESSION_ASSETS_H
#define TEST_VM_SUPPORT_ROM_SESSION_ASSETS_H

#include "type.h"

#include "vm/composition/session/session_interface.h"

static inline C_VOID vm_test_default_pc_at_assets(vm_session_assets *assets,
    type_unsigned_8 rom[VM_SESSION_PC_AT_ROM_BYTES])
{
    if (assets == STD_NULL || rom == STD_NULL) return;
    STD_MEMSET(rom, 0, VM_SESSION_PC_AT_ROM_BYTES);
    rom[VM_SESSION_PC_AT_ROM_BYTES - 16u] = 0xf4u;
    *assets = (vm_session_assets) { .bios = { { rom, VM_SESSION_PC_AT_ROM_BYTES } } };
}

static inline type_status vm_test_default_pc_at_session_create(
    const vm_session_config *requested, vm_session **out_session)
{
    type_unsigned_8 rom[VM_SESSION_PC_AT_ROM_BYTES];
    vm_session_assets assets;
    vm_session_config config = requested == STD_NULL ? (vm_session_config) {0} :
        *requested;

    config.profile_kind = VM_SESSION_PROFILE_DEFAULT_PC_AT;
    config.bios_count = 1u;
    config.bios_path[0u] = STD_NULL;
    config.bios_path[1u] = STD_NULL;
    config.cmos_seed = STD_NULL;
    config.video_path = STD_NULL;
    vm_test_default_pc_at_assets(&assets, rom);
    return vm_session_create_from_assets(&config, &assets, out_session);
}

static inline C_VOID vm_test_ibm_5170_assets(vm_session_assets *assets,
    type_unsigned_8 even[VM_SESSION_PC_AT_ROM_CHIP_BYTES],
    type_unsigned_8 odd[VM_SESSION_PC_AT_ROM_CHIP_BYTES])
{
    if (assets == STD_NULL || even == STD_NULL || odd == STD_NULL) return;
    STD_MEMSET(even, 0xff, VM_SESSION_PC_AT_ROM_CHIP_BYTES);
    STD_MEMSET(odd, 0xff, VM_SESSION_PC_AT_ROM_CHIP_BYTES);
    even[VM_SESSION_PC_AT_ROM_CHIP_BYTES - 8u] = 0xf4u;
    *assets = (vm_session_assets) { .bios = {
        { even, VM_SESSION_PC_AT_ROM_CHIP_BYTES },
        { odd, VM_SESSION_PC_AT_ROM_CHIP_BYTES }
    } };
}

static inline type_status vm_test_ibm_5170_session_create(
    const vm_session_config *requested, vm_session **out_session)
{
    type_unsigned_8 even[VM_SESSION_PC_AT_ROM_CHIP_BYTES];
    type_unsigned_8 odd[VM_SESSION_PC_AT_ROM_CHIP_BYTES];
    vm_session_assets assets;
    vm_session_config config = requested == STD_NULL ? (vm_session_config) {0} :
        *requested;

    config.profile_kind = VM_SESSION_PROFILE_IBM_5170_MODEL_339;
    config.bios_count = 2u;
    config.bios_path[0u] = STD_NULL;
    config.bios_path[1u] = STD_NULL;
    config.cmos_seed = STD_NULL;
    config.video_path = STD_NULL;
    vm_test_ibm_5170_assets(&assets, even, odd);
    return vm_session_create_from_assets(&config, &assets, out_session);
}

#endif
