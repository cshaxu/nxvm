#include "ibmpc/machine/machine_interface.h"
#include "lib/types/types_interface.h"
#include "ibmpc/board-common/machine_board_interface.h"
#include <stdio.h>

#include "ibmpc/board-common/vadp_interface.h"
#include "../../../x86/core/bus_fixture.h"
#include "../../../x86/core/video_topology_fixture.h"
#include "../../../ibmpc/board-common/video_topology_fixture.h"
#include "ibmpc/machine/lifecycle.h"
#include "ibmpc/machine/machine_private.h"
#include "ibmpc/machine/machine_interface.h"
#include "../../../ibmpc/machine/support/rom/session_assets.h"

static lib_i32 vm_model_339_cga_topology(void)
{
    const vm_machine_config config = {0};
    x86_video_snapshot snapshot;
    lib_u8 value = 0x5au;
    vm_machine *session = LIB_NULL;
    lib_i32 failed = vm_test_ibm_5170_session_create(&config, &session) != LIB_STATUS_OK ||
        session == LIB_NULL;

    if (!failed) failed |= test_video_cga_ports(session->core_machine) |
        (test_video_aperture_mismatch(session->board, LIB_FALSE) << 9) |
        ((core_machine_capture_display_snapshot(session->board, &snapshot) !=
            LIB_STATUS_OK ||
            snapshot.kind != X86_VIDEO_KIND_TEXT) << 10) |
        (core_machine_memory_write(session->core_machine, 0x000a0000u,
            &value, sizeof(value)) != LIB_STATUS_OK) << 11 |
        (core_machine_memory_read(session->core_machine, 0x000a0000u,
            &value, sizeof(value)) != LIB_STATUS_OK || value != 0xffu) << 12;
    vm_machine_destroy(session);
    return failed;
}

static lib_i32 vm_default_ega_topology(void)
{
    vm_machine *session = LIB_NULL;
    lib_i32 failed = vm_test_default_pc_at_session_create(LIB_NULL, &session) != LIB_STATUS_OK ||
        session == LIB_NULL;

    if (!failed) failed |= test_video_ega_ports(session->core_machine) |
        (test_video_aperture_mismatch(session->board, LIB_TRUE) << 5);
    if (!failed) {
        test_core_machine_fixture_write_port(session->core_machine, 0x3ceu, 6u);
        failed |= (test_core_machine_fixture_read_bus(session->core_machine,
            0x3cfu) != 0x05u) << 6;
    }
    vm_machine_destroy(session);
    return failed;
}

lib_i32 main(void)
{
    if (vm_model_339_cga_topology() || vm_default_ega_topology()) return 1;
    printf("M5:T366:S6:MODEL339-CGA-TOPOLOGY:OK\n");
    printf("M5:T375:S15:MODEL339-REV3-CGA-DEFAULTS:OK\n");
    return 0;
}
