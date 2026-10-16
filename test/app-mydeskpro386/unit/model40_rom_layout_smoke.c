#include "../support/profile.h"
#include "core/machine/machine_interface.h"
#include "lib/types/types_interface.h"
#include "lib/types/file.h"

#include "x86-core/machine/machine_interface.h"
#include "core/machine/machine_private.h"
#include "../support/model40_session_assets.h"
#include "core/machine/machine_interface.h"

static lib_i32 vm_model40_rom_read(x86_core_machine *machine,
    lib_u32 physical, lib_u8 expected)
{
    lib_u8 observed = 0u;

    return x86_core_machine_memory_read(machine, physical, &observed,
        sizeof(observed)) == LIB_STATUS_OK && observed == expected;
}

lib_i32 main(void)
{
    static lib_u8 even[MYDESKPRO386_ROM_CHIP_BYTES];
    static lib_u8 odd[MYDESKPRO386_ROM_CHIP_BYTES];
    vm_machine_config invalid_config = {0};
    vm_machine_assets missing_assets = {0};
    vm_machine *session = LIB_NULL;
    x86_core_machine_run_result result;
    lib_u8 write = 0u;
    lib_i32 failed = 0;

    even[0u] = 0x11u;
    odd[0u] = 0x22u;
    even[0x3ff8u] = 0xf4u;

    failed |= vm_test_machine_create_from_assets(VM_MACHINE_PROFILE_COMPAQ_DESKPRO_386_MODEL_40,
        &invalid_config, &missing_assets, &session) !=
        LIB_STATUS_INVALID_ARGUMENT ||
        session != LIB_NULL;
    if (!failed) failed |= vm_model40_fixture_create_bytes(even, odd, &session) !=
        LIB_STATUS_OK || session == LIB_NULL ||
        !vm_model40_rom_read(session->x86_core_machine, 0x000f0000u, 0x11u) ||
        !vm_model40_rom_read(session->x86_core_machine, 0x000f0001u, 0x22u) ||
        !vm_model40_rom_read(session->x86_core_machine, 0x000f8000u, 0x11u) ||
        !vm_model40_rom_read(session->x86_core_machine, 0x000ffff0u, 0xf4u) ||
        !vm_model40_rom_read(session->x86_core_machine, 0xffff0000u, 0x11u) ||
        !vm_model40_rom_read(session->x86_core_machine, 0xffff8000u, 0x11u) ||
        !vm_model40_rom_read(session->x86_core_machine, 0xfffffff0u, 0xf4u) ||
        x86_core_machine_memory_write(session->x86_core_machine, 0x000f0000u, &write,
            sizeof(write)) != LIB_STATUS_OK ||
        x86_core_machine_memory_write(session->x86_core_machine, 0xffff0000u, &write,
            sizeof(write)) != LIB_STATUS_OK ||
        !vm_model40_rom_read(session->x86_core_machine, 0x000f0000u, 0x11u) ||
        !vm_model40_rom_read(session->x86_core_machine, 0xffff0000u, 0x11u);
    if (!failed) failed |= x86_core_machine_run(session->x86_core_machine,
        (x86_core_machine_run_budget) {1u, 0u}, &result) != LIB_STATUS_OK ||
        result.executed != 1u ||
        result.reason != X86_CORE_MACHINE_STOP_WAITING_FOR_INTERRUPT;
    if (!failed) lib_c_printf("MODEL40-ROM-LAYOUT:OK\n");
    if (!failed) lib_c_printf("MODEL40-ROM-RESET:OK\n");
    vm_machine_destroy(session);
    return failed ? 1 : 0;
}
