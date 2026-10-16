#include "x86-nxvm/machine/machine_interface.h"
#include "lib/types/types_interface.h"
#include "lib/types/file.h"

#include "x86-nxvm/board-base/vadp_interface.h"
#include "../../x86-nxvm/board-base/composition/bus_fixture.h"
#include "x86-nxvm/machine/lifecycle.h"
#include "x86-nxvm/machine/machine_private.h"
#include "x86-nxvm/machine/machine_interface.h"
#include "../support/model40_session_assets.h"

lib_i32 main(void)
{
    x86_nxvm_machine *session = LIB_NULL;
    x86_core_machine *core;
    lib_i32 failed = 0;

    failed |= vm_model40_fixture_create(&session) !=
        LIB_STATUS_OK || session == LIB_NULL;
    if (!failed) {
        core = session->x86_core_machine;
        test_x86_nxvm_machine_fixture_write_port(core, 0x03d4u, 0x0eu);
        test_x86_nxvm_machine_fixture_write_port(core, 0x03d5u, 0x12u);
        test_x86_nxvm_machine_fixture_write_port(core, 0x03c2u,
            0x00u);
        test_x86_nxvm_machine_fixture_write_port(core, 0x03d4u, 0x0eu);
        test_x86_nxvm_machine_fixture_write_port(core, 0x03d5u, 0x56u);
        test_x86_nxvm_machine_fixture_write_port(core, 0x03b4u, 0x0eu);
        test_x86_nxvm_machine_fixture_write_port(core, 0x03b5u, 0x34u);
        test_x86_nxvm_machine_fixture_write_port(core, 0x03bau, 0x03u);
        /* Inactive routes preserve the last Core bus write, not a chip zero. */
        failed |= test_x86_nxvm_machine_fixture_read_bus(core, 0x03d5u) != 3u ||
            test_x86_nxvm_machine_fixture_read_bus(core, 0x03b5u) != 0x34u ||
            test_x86_nxvm_machine_fixture_read_bus(core, 0x07c6u) != 0x0du;
    }
    if (!failed) {
        x86_nxvm_machine_reset(session);
        core = session->x86_core_machine;
        test_x86_nxvm_machine_fixture_write_port(core, 0x03d4u, 0x0eu);
        test_x86_nxvm_machine_fixture_write_port(core, 0x03d5u, 0x25u);
        failed |= test_x86_nxvm_machine_fixture_read_bus(core, 0x03b5u) != 0x25u ||
            test_x86_nxvm_machine_fixture_read_bus(core, 0x03d5u) != 0x25u ||
            test_x86_nxvm_machine_fixture_read_bus(core, 0x07c6u) != 0x0du;
    }
    x86_nxvm_machine_destroy(session);
    if (!failed) {
        lib_c_printf("MODEL40-IO-BASE:OK\n");
        return 0;
    }
    lib_c_fprintf(lib_c_stderr, "MODEL40-IO-BASE:FAIL\n");
    return 1;
}
