#include "type.h"

#include "core/machine/machine.h"
#include "core/machine/machine_interface.h"
#include "vm/composition/session/session_private.h"
#include "../support/vm_model40_byob_fixture.h"

C_INT main(C_VOID)
{
    static type_unsigned_8 even[VM_PROFILE_MODEL40_ROM_CHIP_BYTES];
    static type_unsigned_8 odd[VM_PROFILE_MODEL40_ROM_CHIP_BYTES];
    core_machine_run_result result;
    type_unsigned_8 high_rom_byte = 0u;
    vm_session *session = STD_NULL;
    C_INT failed = 0;

    odd[0u] = 0xa5u;

    failed |= vm_model40_fixture_create_bytes("t386-s23-even.bin", even,
        "4fe7b59af6de3b665b67788cc2f99892ab827efae3a467342b3bb4e3bc8e5bfe",
        "t386-s23-odd.bin", odd,
        "3c67be978c30df98fa1dc37c870af553b70885e9565e510b9bcdb05556c7b38e",
        &session) != TYPE_STATUS_OK ||
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
            core_machine_memory_read(session->core_machine,
                VM_PROFILE_MODEL40_ROM_HIGH_RESET_ALIAS_START + 1u, &high_rom_byte,
                sizeof(high_rom_byte)) != TYPE_STATUS_OK || high_rom_byte != 0xa5u ||
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
    vm_model40_fixture_remove("t386-s23-even.bin", "t386-s23-odd.bin");
    if (failed) return 1;
    STD_PRINTF("M5:T386:S23:D4-SKEY-A20:OK\n");
    STD_PRINTF("M5:T386:S23:CORE-VM-RESET-OWNER:OK\n");
    STD_PRINTF("M5:T390:S31:MODEL40-A20-POLICY:OK\n");
    return 0;
}
