#include "core/machine/machine_interface.h"
#include "lib/types/types_interface.h"
#include <stdio.h>

#include "../../../core/board-base/composition/composition_fixture.h"
#include "core/x86/memory_interface.h"
#include "core/x86/machine_interface.h"
#include "core/machine/machine_private.h"
#include "../../support/rom/model40_session_assets.h"

lib_i32 main(void)
{
    static lib_u8 even[VM_PROFILE_MODEL40_ROM_CHIP_BYTES];
    static lib_u8 odd[VM_PROFILE_MODEL40_ROM_CHIP_BYTES];
    core_machine_run_result result;
    lib_u8 high_rom_byte = 0u;
    vm_machine *session = LIB_NULL;
    lib_i32 failed = 0;

    odd[0u] = 0xa5u;

    failed |= vm_model40_fixture_create_bytes(even, odd, &session) != LIB_STATUS_OK ||
        session == LIB_NULL;
    if (!failed) {
        failed |= core_machine_bus_write(session->core_machine, 0x0064u, 0xd1u) !=
                LIB_STATUS_OK ||
            core_machine_bus_write(session->core_machine, 0x0060u, 0x03u) !=
                LIB_STATUS_OK || !test_core_a20_is_enabled(session->core_machine) ||
            core_machine_bus_write(session->core_machine, 0x0064u, 0xd1u) !=
                LIB_STATUS_OK ||
            core_machine_bus_write(session->core_machine, 0x0060u, 0x01u) !=
                LIB_STATUS_OK || test_core_a20_is_enabled(session->core_machine) ||
            core_machine_memory_read(session->core_machine,
                VM_PROFILE_MODEL40_ROM_HIGH_RESET_ALIAS_START + 1u, &high_rom_byte,
                sizeof(high_rom_byte)) != LIB_STATUS_OK || high_rom_byte != 0xa5u ||
            core_machine_bus_write(session->core_machine, 0x0064u, 0xd1u) !=
                LIB_STATUS_OK ||
            core_machine_bus_write(session->core_machine, 0x0060u, 0u) !=
                LIB_STATUS_OK ||
            core_machine_run(session->core_machine, (core_machine_run_budget){1u, 0u},
                &result) != LIB_STATUS_OK ||
            result.reason != CORE_MACHINE_STOP_RESET_REQUESTED ||
            test_core_a20_is_enabled(session->core_machine);
    }
    vm_machine_destroy(session);
    if (failed) return 1;
    printf("D4-SKEY-A20:OK\n");
    printf("CORE-VM-RESET-OWNER:OK\n");
    printf("MODEL40-A20-POLICY:OK\n");
    return 0;
}
