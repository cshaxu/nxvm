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
        failed |= test_x86_nxvm_machine_fixture_read_bus(core,
            0x03c2u) != 0xe0u;
        test_x86_nxvm_machine_fixture_write_port(core, 0x03c2u,
            0x00u);
        failed |= test_x86_nxvm_machine_fixture_read_bus(core,
            0x03c2u) != 0xf0u;
    }
    if (!failed) {
        x86_nxvm_machine_reset(session);
        core = session->x86_core_machine;
        failed |= test_x86_nxvm_machine_fixture_read_bus(core,
            0x03c2u) != 0xe0u;
    }
    x86_nxvm_machine_destroy(session);
    if (!failed) {
        lib_c_printf("MODEL40-INPUT-STATUS-0:OK\n");
        return 0;
    }
    lib_c_fprintf(lib_c_stderr, "MODEL40-INPUT-STATUS-0:FAIL\n");
    return 1;
}
