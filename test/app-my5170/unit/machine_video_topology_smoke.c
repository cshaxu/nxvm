#include "x86-nxvm/machine/machine_interface.h"
#include "lib/types/types_interface.h"
#include "lib/types/file.h"

#include "../../x86-nxvm/board-base/composition/bus_fixture.h"
#include "../../x86-nxvm/support/video_topology_fixture.h"
#include "../../x86-nxvm/board-base/support/video_topology_fixture.h"
#include "../support/session_assets.h"
#include "x86-nxvm/machine/machine_private.h"

lib_i32 main(void)
{
    const x86_nxvm_machine_config config = {0};
    x86_video_snapshot snapshot;
    lib_u8 value = 0x5au;
    x86_nxvm_machine *session = LIB_NULL;
    lib_i32 failed = vm_test_ibm_5170_session_create(&config, &session) != LIB_STATUS_OK ||
        session == LIB_NULL;

    if (!failed) failed |= test_video_cga_ports(session->x86_core_machine);
    if (!failed) failed |= x86_nxvm_machine_reset(session) != LIB_STATUS_OK;
    if (!failed) failed |= (test_video_aperture_mismatch(session->board, LIB_FALSE) << 9) |
        ((x86_nxvm_board_capture_display_snapshot(session->board, &snapshot) !=
            LIB_STATUS_OK || snapshot.kind != X86_VIDEO_KIND_TEXT) << 10) |
        (x86_core_machine_memory_write(session->x86_core_machine, 0x000a0000u,
            &value, sizeof(value)) != LIB_STATUS_OK) << 11 |
        (x86_core_machine_memory_read(session->x86_core_machine, 0x000a0000u,
            &value, sizeof(value)) != LIB_STATUS_OK || value != 0xffu) << 12;
    x86_nxvm_machine_destroy(session);
    if (failed) return 1;
    lib_c_printf("MY5170:CGA-TOPOLOGY:OK\n");
    return 0;
}
