#include "lib/types/types_interface.h"
#include <stdio.h>

#include "app-nxvm/devices/machine.h"
#include "app-nxvm/devices/machine_interface.h"
#include "app-nxvm/machine/machine_private.h"
#include "app-nxvm/machine/machine_interface.h"
#include "support/rom/model40_session_assets.h"

static lib_i32 vm_model40_d4_read(core_machine *machine,
    lib_u32 physical, lib_u8 expected)
{
    lib_u8 observed = 0u;

    return core_machine_memory_read(machine, physical, &observed,
        sizeof(observed)) == LIB_STATUS_OK && observed == expected;
}

lib_i32 main(void)
{
    static lib_u8 even[VM_PROFILE_MODEL40_ROM_CHIP_BYTES];
    static lib_u8 odd[VM_PROFILE_MODEL40_ROM_CHIP_BYTES];
    vm_machine_config invalid_config = {
        .profile_kind = VM_MACHINE_PROFILE_COMPAQ_DESKPRO_386_MODEL_40
    };
    vm_machine_assets missing_assets = {0};
    vm_machine *session = LIB_NULL;
    core_machine_run_result result;
    lib_u8 write = 0u;
    lib_u32 port_b = 0u;
    lib_i32 failed = 0;

    even[0u] = 0x11u;
    odd[0u] = 0x22u;
    even[0x3ff8u] = 0xf4u;

    failed |= vm_machine_create_from_assets(&invalid_config, &missing_assets, &session) !=
        LIB_STATUS_INVALID_ARGUMENT || session != LIB_NULL;
    if (!failed) failed |= vm_model40_fixture_create_bytes(even, odd, &session) !=
        LIB_STATUS_OK || session == LIB_NULL ||
        core_machine_bus_read(session->core_machine, CORE_MACHINE_PC_AT_PORT_B,
            &port_b) != LIB_STATUS_OK || (port_b & 0x10u) == 0u ||
        (core_machine_pit_advance(&session->core_machine->shared_pit, 19u),
         core_machine_bus_read(session->core_machine, CORE_MACHINE_PC_AT_PORT_B,
            &port_b) != LIB_STATUS_OK) || (port_b & 0x10u) != 0u ||
        !vm_model40_d4_read(session->core_machine,
            VM_PROFILE_MODEL40_ROM_LOW_PHYSICAL_START, 0x11u) ||
        !vm_model40_d4_read(session->core_machine,
            VM_PROFILE_MODEL40_ROM_LOW_PHYSICAL_START + 1u, 0x22u) ||
        !vm_model40_d4_read(session->core_machine, 0x000ffff0u, 0xf4u) ||
        core_machine_memory_write(session->core_machine,
            VM_PROFILE_MODEL40_ROM_LOW_PHYSICAL_START, &write,
            sizeof(write)) != LIB_STATUS_OK ||
        /* Immutable firmware accepts the bus write but retains its sole ROM
         * byte; the companion mapping test covers the same property. */
        !vm_model40_d4_read(session->core_machine, 0x000f0000u, 0x11u) ||
        core_machine_set_a20(session->core_machine, LIB_TRUE) != LIB_STATUS_OK ||
        core_machine_reset(session->core_machine) != LIB_STATUS_OK ||
        core_machine_bus_read(session->core_machine, CORE_MACHINE_PC_AT_PORT_B,
            &port_b) != LIB_STATUS_OK || (port_b & 0x10u) == 0u;
    if (!failed) failed |= core_machine_run(session->core_machine,
        (core_machine_run_budget) {1u, 0u}, &result) != LIB_STATUS_OK ||
        result.executed != 1u ||
        result.reason != CORE_MACHINE_STOP_WAITING_FOR_INTERRUPT;
    if (!failed) printf("M5:T386:S25:D4-SOLE-ROM-MAP:OK\n");
    if (!failed) printf("M5:T386:S25:D4-COMPATIBILITY-RESET:OK\n");
    if (!failed) printf("M5:T386:S25:AT-REFRESH-CLOCK:OK\n");
    vm_machine_destroy(session);
    return failed ? 1 : 0;
}
