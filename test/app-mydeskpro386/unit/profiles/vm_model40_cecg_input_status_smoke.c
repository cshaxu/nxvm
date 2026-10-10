#include "core/machine/machine_interface.h"
#include "lib/types/types_interface.h"
#include <stdio.h>

#include "core/board-base/vadp_interface.h"
#include "../../../core/board-base/composition/bus_fixture.h"
#include "core/machine/lifecycle.h"
#include "core/machine/machine_private.h"
#include "core/machine/machine_interface.h"
#include "../../support/model40_session_assets.h"

lib_i32 main(void)
{
    vm_machine *session = LIB_NULL;
    core_machine *core;
    lib_i32 failed = 0;

    failed |= vm_model40_fixture_create(&session) !=
        LIB_STATUS_OK || session == LIB_NULL;
    if (!failed) {
        core = session->core_machine;
        failed |= test_core_machine_fixture_read_bus(core,
            0x03c2u) != 0xe0u;
        test_core_machine_fixture_write_port(core, 0x03c2u,
            0x00u);
        failed |= test_core_machine_fixture_read_bus(core,
            0x03c2u) != 0xf0u;
    }
    if (!failed) {
        vm_machine_reset(session);
        core = session->core_machine;
        failed |= test_core_machine_fixture_read_bus(core,
            0x03c2u) != 0xe0u;
    }
    vm_machine_destroy(session);
    if (!failed) {
        printf("MODEL40-INPUT-STATUS-0:OK\n");
        return 0;
    }
    fprintf(stderr, "MODEL40-INPUT-STATUS-0:FAIL\n");
    return 1;
}
