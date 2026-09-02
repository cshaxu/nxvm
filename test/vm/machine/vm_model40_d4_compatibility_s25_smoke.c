#include "type.h"

#include "core/machine/machine.h"
#include "core/machine/machine_interface.h"
#include "vm/composition/session/session_private.h"
#include "vm/composition/session/session_interface.h"
#include "../support/vm_model40_byob_fixture.h"

static C_INT vm_model40_d4_read(core_machine *machine,
    type_unsigned_32 physical, type_unsigned_8 expected)
{
    type_unsigned_8 observed = 0u;

    return core_machine_memory_read(machine, physical, &observed,
        sizeof(observed)) == TYPE_STATUS_OK && observed == expected;
}

C_INT main(C_VOID)
{
    static type_unsigned_8 even[VM_PROFILE_MODEL40_ROM_CHIP_BYTES];
    static type_unsigned_8 odd[VM_PROFILE_MODEL40_ROM_CHIP_BYTES];
    vm_session_config invalid_config = {
        .profile_kind = VM_SESSION_PROFILE_COMPAQ_DESKPRO_386_MODEL_40
    };
    vm_session *session = STD_NULL;
    core_machine_run_result result;
    type_unsigned_8 write = 0u;
    type_unsigned_32 port_b = 0u;
    C_INT failed = 0;

    even[0u] = 0x11u;
    odd[0u] = 0x22u;
    even[0x3ff8u] = 0xf4u;

    failed |= vm_session_create(&invalid_config, &session) !=
        TYPE_STATUS_INVALID_ARGUMENT || session != STD_NULL;
    if (!failed) failed |= vm_model40_fixture_create_bytes("t386-s25-even.bin", even,
        "f7eb6af712d2cbc0ee03468e664d089fa67bfc48591b071c3bcaf8ab830653af",
        "t386-s25-odd.bin", odd,
        "bd99701a5fbeb22a5d990331ad56a7164935b8229a6410007d4f5f08dfd93335",
        &session) !=
        TYPE_STATUS_OK || session == STD_NULL ||
        core_machine_bus_read(session->core_machine, CORE_MACHINE_PC_AT_PORT_B,
            &port_b) != TYPE_STATUS_OK || (port_b & 0x10u) == 0u ||
        (core_machine_pit_advance(&session->core_machine->shared_pit, 19u),
         core_machine_bus_read(session->core_machine, CORE_MACHINE_PC_AT_PORT_B,
            &port_b) != TYPE_STATUS_OK) || (port_b & 0x10u) != 0u ||
        !vm_model40_d4_read(session->core_machine,
            VM_PROFILE_MODEL40_ROM_LOW_PHYSICAL_START, 0x11u) ||
        !vm_model40_d4_read(session->core_machine,
            VM_PROFILE_MODEL40_ROM_LOW_PHYSICAL_START + 1u, 0x22u) ||
        !vm_model40_d4_read(session->core_machine, 0x000ffff0u, 0xf4u) ||
        core_machine_memory_write(session->core_machine,
            VM_PROFILE_MODEL40_ROM_LOW_PHYSICAL_START, &write,
            sizeof(write)) != TYPE_STATUS_OK ||
        /* Immutable firmware accepts the bus write but retains its sole ROM
         * byte; the companion mapping test covers the same property. */
        !vm_model40_d4_read(session->core_machine, 0x000f0000u, 0x11u) ||
        core_machine_set_a20(session->core_machine, TYPE_TRUE) != TYPE_STATUS_OK ||
        core_machine_reset(session->core_machine) != TYPE_STATUS_OK ||
        core_machine_bus_read(session->core_machine, CORE_MACHINE_PC_AT_PORT_B,
            &port_b) != TYPE_STATUS_OK || (port_b & 0x10u) == 0u;
    if (!failed) failed |= core_machine_run(session->core_machine,
        (core_machine_run_budget) {1u, 0u}, &result) != TYPE_STATUS_OK ||
        result.executed != 1u ||
        result.reason != CORE_MACHINE_STOP_WAITING_FOR_INTERRUPT;
    if (!failed) STD_PRINTF("M5:T386:S25:D4-SOLE-ROM-MAP:OK\n");
    if (!failed) STD_PRINTF("M5:T386:S25:D4-COMPATIBILITY-RESET:OK\n");
    if (!failed) STD_PRINTF("M5:T386:S25:AT-REFRESH-CLOCK:OK\n");
    vm_session_destroy(session);
    vm_model40_fixture_remove("t386-s25-even.bin", "t386-s25-odd.bin");
    return failed ? 1 : 0;
}
