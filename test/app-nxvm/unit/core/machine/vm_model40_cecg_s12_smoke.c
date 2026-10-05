#include "ibmpc/machine/machine_interface.h"
#include "lib/types/types_interface.h"
#include <stdio.h>

#include "ibmpc/board-common/vadp_interface.h"
#include "../../../../x86/core/bus_fixture.h"
#include "ibmpc/machine/lifecycle.h"
#include "ibmpc/machine/machine_private.h"
#include "ibmpc/machine/machine_interface.h"
#include "support/rom/model40_session_assets.h"

lib_i32 main(void)
{
    vm_machine *session = LIB_NULL;
    core_machine *core;
    lib_i32 failed = 0;

    failed |= vm_model40_fixture_create(&session) !=
        LIB_STATUS_OK || session == LIB_NULL;
    if (!failed) {
        core = session->core_machine;
        test_core_machine_fixture_write_port(core, 0x03d4u, 0x0eu);
        test_core_machine_fixture_write_port(core, 0x03d5u, 0x12u);
        test_core_machine_fixture_write_port(core, 0x03c2u,
            0x00u);
        test_core_machine_fixture_write_port(core, 0x03d4u, 0x0eu);
        test_core_machine_fixture_write_port(core, 0x03d5u, 0x56u);
        test_core_machine_fixture_write_port(core, 0x03b4u, 0x0eu);
        test_core_machine_fixture_write_port(core, 0x03b5u, 0x34u);
        test_core_machine_fixture_write_port(core, 0x03bau, 0x03u);
        failed |= test_core_machine_fixture_read_bus(core, 0x03d5u) != 0u ||
            test_core_machine_fixture_read_bus(core, 0x03b5u) != 0x34u ||
            test_core_machine_fixture_read_bus(core, 0x07c6u) != 0x03u;
    }
    if (!failed) {
        vm_machine_reset(session);
        core = session->core_machine;
        test_core_machine_fixture_write_port(core, 0x03d4u, 0x0eu);
        test_core_machine_fixture_write_port(core, 0x03d5u, 0x25u);
        failed |= test_core_machine_fixture_read_bus(core, 0x03b5u) != 0u ||
            test_core_machine_fixture_read_bus(core, 0x03d5u) != 0x25u ||
            test_core_machine_fixture_read_bus(core, 0x07c6u) != 0x00u;
    }
    vm_machine_destroy(session);
    if (!failed) {
        printf("M5:T386:S12:MODEL40-IO-BASE:OK\n");
        return 0;
    }
    fprintf(stderr, "M5:T386:S12:MODEL40-IO-BASE:FAIL\n");
    return 1;
}
