#include "x86-nxvm/machine/machine_interface.h"
#include "lib/types/types_interface.h"
#include "lib/types/file.h"

#include "x86-nxvm/board-base/vadp_interface.h"
#include "../../x86-nxvm/board-base/composition/bus_fixture.h"
#include "../../x86-nxvm/support/video_topology_fixture.h"
#include "x86-nxvm/machine/lifecycle.h"
#include "x86-nxvm/machine/machine_private.h"
#include "x86-nxvm/machine/machine_interface.h"
#include "../support/model40_session_assets.h"

lib_i32 main(void)
{
    x86_nxvm_machine *session = LIB_NULL;
    lib_i32 failed = 0;

    failed |= vm_model40_fixture_create(&session) !=
        LIB_STATUS_OK || session == LIB_NULL;
    if (!failed) {
        x86_video_snapshot snapshot;

        failed |= x86_nxvm_board_capture_display_snapshot(session->board, &snapshot) != LIB_STATUS_OK ||
            snapshot.palette_rgb[1] != 0x0000aau;
        test_x86_nxvm_machine_fixture_write_port(session->x86_core_machine, 0x03c6u, 0x51u);
        failed |= x86_nxvm_board_capture_display_snapshot(session->board, &snapshot) != LIB_STATUS_OK ||
            snapshot.palette_rgb[1] != 0u || snapshot.cursor_visible || !snapshot.buffer_changed;
        test_x86_nxvm_machine_fixture_write_port(session->x86_core_machine, 0x03c6u, 0x61u);
        failed |= x86_nxvm_board_capture_display_snapshot(session->board, &snapshot) != LIB_STATUS_OK ||
            snapshot.palette_rgb[1] != 0x0000aau || !snapshot.buffer_changed;
        test_x86_nxvm_machine_fixture_write_port(session->x86_core_machine, 0x03c6u, 0x41u);
        failed |= test_x86_nxvm_machine_fixture_read_bus(session->x86_core_machine,
            0x03c6u) != 0x41u ||
            test_video_missing_write_port(session->x86_core_machine, 0x03dcu);
        test_x86_nxvm_machine_fixture_write_port(session->x86_core_machine,
            0x03c6u, 0x7fu);
        test_x86_nxvm_machine_fixture_write_port(session->x86_core_machine,
            0x03dcu, 0u);
        failed |= test_x86_nxvm_machine_fixture_read_bus(session->x86_core_machine,
            0x03c6u) != 0x7fu ||
            (test_x86_nxvm_machine_fixture_read_bus(session->x86_core_machine,
            0x03dau) & 0x06u) != 0x06u;
        test_x86_nxvm_machine_fixture_write_port(session->x86_core_machine,
            0x03c6u, 0x5au);
        failed |= test_x86_nxvm_machine_fixture_read_bus(session->x86_core_machine,
            0x03c6u) != 0x5au;
        test_x86_nxvm_machine_fixture_write_port(session->x86_core_machine,
            0x03c6u, 0xa5u);
        failed |= test_x86_nxvm_machine_fixture_read_bus(session->x86_core_machine,
            0x03c6u) != 0xa5u;
    }
    if (!failed) {
        x86_nxvm_machine_reset(session);
        failed |= test_x86_nxvm_machine_fixture_read_bus(session->x86_core_machine,
            0x03c6u) != 0x41u ||
            (test_x86_nxvm_machine_fixture_read_bus(session->x86_core_machine,
            0x03dau) & 0x06u) != 0x04u;
    }
    x86_nxvm_machine_destroy(session);
    if (!failed) {
        lib_c_printf("MODEL40-CECG:OK\n");
        return 0;
    }
    lib_c_fprintf(lib_c_stderr, "MODEL40-CECG:FAIL\n");
    return 1;
}
