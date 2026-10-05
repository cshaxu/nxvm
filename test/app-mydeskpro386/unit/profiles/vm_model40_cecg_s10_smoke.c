#include "ibmpc/machine/machine_interface.h"
#include "lib/types/types_interface.h"
#include "ibmpc/board-common/machine_board_interface.h"
#include <stdio.h>

#include "ibmpc/board-common/vadp_interface.h"
#include "../../../x86/core/bus_fixture.h"
#include "ibmpc/machine/lifecycle.h"
#include "ibmpc/machine/machine_private.h"
#include "ibmpc/machine/machine_interface.h"
#include "../../support/rom/model40_session_assets.h"

lib_i32 main(void)
{
    vm_machine *session = LIB_NULL;
    x86_video_snapshot snapshot;
    static const lib_u8 text[] = { 'O', 0x07u, 'K', 0x07u };
    lib_i32 failed = 0;

    failed |= vm_model40_fixture_create(&session) !=
        LIB_STATUS_OK || session == LIB_NULL;
    if (!failed) {
        failed |= core_machine_memory_write(session->core_machine,
            0x000b8000u, text, sizeof(text)) != LIB_STATUS_OK ||
            core_machine_capture_display_snapshot(session->board, &snapshot) !=
                LIB_STATUS_OK || snapshot.kind != X86_VIDEO_KIND_TEXT ||
            snapshot.characters[0] != 'O' || snapshot.characters[1] != 'K';
    }
    if (!failed) {
        test_core_machine_fixture_write_port(session->core_machine,
            0x03dau, 0x03u);
        failed |= test_core_machine_fixture_read_bus(session->core_machine,
            0x07c6u) != 0x03u;
    }
    if (!failed) {
        vm_machine_reset(session);
        failed |= test_core_machine_fixture_read_bus(session->core_machine,
            0x07c6u) != 0x00u;
    }
    vm_machine_destroy(session);
    if (!failed) {
        printf("M5:T386:S10:MODEL40-FEATURE-ENVIRONMENT:OK\n");
        return 0;
    }
    fprintf(stderr, "M5:T386:S10:MODEL40-FEATURE-ENVIRONMENT:FAIL\n");
    return 1;
}
