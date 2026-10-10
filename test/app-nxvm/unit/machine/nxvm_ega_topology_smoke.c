#include "core/machine/machine_interface.h"
#include "lib/types/types_interface.h"
#include "lib/types/file.h"

#include "../../../core/board-base/composition/bus_fixture.h"
#include "../../../core/support/video_topology_fixture.h"
#include "../../../core/board-base/support/video_topology_fixture.h"
#include "../../support/rom/session_assets.h"
#include "core/machine/machine_private.h"

lib_i32 main(void)
{
    vm_machine *session = LIB_NULL;
    lib_i32 failed = vm_test_default_pc_at_session_create(LIB_NULL, &session) !=
        LIB_STATUS_OK || session == LIB_NULL;

    if (!failed) failed |= test_video_ega_ports(session->core_machine) |
        (test_video_aperture_mismatch(session->board, LIB_TRUE) << 5);
    if (!failed) {
        test_core_machine_fixture_write_port(session->core_machine, 0x3ceu, 6u);
        failed |= (test_core_machine_fixture_read_bus(session->core_machine,
            0x3cfu) != 0x05u) << 6;
    }
    vm_machine_destroy(session);
    if (failed) return 1;
    lib_c_printf("NXVM:EGA-TOPOLOGY:OK\n");
    return 0;
}
