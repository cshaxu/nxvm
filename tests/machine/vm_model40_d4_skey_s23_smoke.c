#include "type.h"

#include "core/machine/machine.h"
#include "core/machine/machine_interface.h"
#include "vm/composition/session/session.h"

C_INT main(C_VOID)
{
    static type_unsigned_8 even[VM_PROFILE_MODEL40_ROM_CHIP_BYTES];
    static type_unsigned_8 odd[VM_PROFILE_MODEL40_ROM_CHIP_BYTES];
    vm_profile_model40_external_rom rom = { even, odd, sizeof(even) };
    core_machine_run_result result;
    vm_session *session = STD_NULL;
    C_INT failed = 0;

    failed |= vm_session_create_model40_private(&rom, &session) != TYPE_STATUS_OK ||
        session == STD_NULL;
    if (!failed) {
        failed |= core_machine_bus_write(session->core_machine, 0x0064u, 0xd1u) !=
                TYPE_STATUS_OK ||
            core_machine_bus_write(session->core_machine, 0x0060u, 0x03u) !=
                TYPE_STATUS_OK || !session->core_machine->executor_memory.data.flagA20 ||
            core_machine_bus_write(session->core_machine, 0x0064u, 0xd1u) !=
                TYPE_STATUS_OK ||
            core_machine_bus_write(session->core_machine, 0x0060u, 0x01u) !=
                TYPE_STATUS_OK || session->core_machine->executor_memory.data.flagA20 ||
            core_machine_bus_write(session->core_machine, 0x0064u, 0xd1u) !=
                TYPE_STATUS_OK ||
            core_machine_bus_write(session->core_machine, 0x0060u, 0u) !=
                TYPE_STATUS_OK ||
            core_machine_run(session->core_machine, (core_machine_run_budget){1u, 0u},
                &result) != TYPE_STATUS_OK ||
            result.reason != CORE_MACHINE_STOP_RESET_REQUESTED ||
            session->core_machine->executor_memory.data.flagA20;
    }
    vm_session_destroy(session);
    if (failed) return 1;
    STD_PRINTF("M5:T386:S23:D4-SKEY-A20:OK\n");
    STD_PRINTF("M5:T386:S23:CORE-VM-RESET-OWNER:OK\n");
    return 0;
}