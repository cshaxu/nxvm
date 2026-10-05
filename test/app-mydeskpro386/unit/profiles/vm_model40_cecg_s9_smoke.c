#include "ibmpc/machine/machine_interface.h"
#include "lib/types/types_interface.h"
#include <stdio.h>

#include "ibmpc/board-common/vadp_interface.h"
#include "../../../ibmpc/core/bus_fixture.h"
#include "../../../app-nxvm/unit/support/core/video_topology_fixture.h"
#include "ibmpc/machine/lifecycle.h"
#include "ibmpc/machine/machine_private.h"
#include "ibmpc/machine/machine_interface.h"
#include "../../support/rom/model40_session_assets.h"

lib_i32 main(void)
{
    vm_machine *session = LIB_NULL;
    lib_i32 failed = 0;

    failed |= vm_model40_fixture_create(&session) !=
        LIB_STATUS_OK || session == LIB_NULL;
    if (!failed) {
        failed |= test_core_machine_fixture_read_bus(session->core_machine,
            0x03c6u) != 0x40u ||
            test_video_missing_write_port(session->core_machine, 0x03dcu);
        test_core_machine_fixture_write_port(session->core_machine,
            0x03c6u, 0x7fu);
        test_core_machine_fixture_write_port(session->core_machine,
            0x03dcu, 0u);
        failed |= test_core_machine_fixture_read_bus(session->core_machine,
            0x03c6u) != 0x7fu ||
            (test_core_machine_fixture_read_bus(session->core_machine,
            0x03dau) & 0x06u) != 0x06u;
        test_core_machine_fixture_write_port(session->core_machine,
            0x03c6u, 0xa5u);
        failed |= test_core_machine_fixture_read_bus(session->core_machine,
            0x03c6u) != 0xa5u;
    }
    if (!failed) {
        vm_machine_reset(session);
        failed |= test_core_machine_fixture_read_bus(session->core_machine,
            0x03c6u) != 0x40u ||
            (test_core_machine_fixture_read_bus(session->core_machine,
            0x03dau) & 0x06u) != 0x04u;
    }
    vm_machine_destroy(session);
    if (!failed) {
        printf("M5:T386:S9:MODEL40-CECG:OK\n");
        return 0;
    }
    fprintf(stderr, "M5:T386:S9:MODEL40-CECG:FAIL\n");
    return 1;
}
