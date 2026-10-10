#include "core/machine/machine_interface.h"
#include "lib/types/types_interface.h"
#include "lib/types/file.h"

#include "../../../core/board-base/composition/bus_fixture.h"
#include "../../../core/support/video_topology_fixture.h"
#include "../../../core/board-base/support/video_topology_fixture.h"
#include "../../support/session_assets.h"
#include "core/machine/machine_private.h"

lib_i32 main(void)
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
            LIB_STATUS_OK || snapshot.kind != X86_VIDEO_KIND_TEXT) << 10) |
        (core_machine_memory_write(session->core_machine, 0x000a0000u,
            &value, sizeof(value)) != LIB_STATUS_OK) << 11 |
        (core_machine_memory_read(session->core_machine, 0x000a0000u,
            &value, sizeof(value)) != LIB_STATUS_OK || value != 0xffu) << 12;
    vm_machine_destroy(session);
    if (failed) return 1;
    lib_c_printf("MY5170:CGA-TOPOLOGY:OK\n");
    return 0;
}
