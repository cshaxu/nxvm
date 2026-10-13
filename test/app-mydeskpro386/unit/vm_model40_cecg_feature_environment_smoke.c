#include "core/machine/machine_interface.h"
#include "lib/types/types_interface.h"
#include "core/board-base/machine_board_interface.h"
#include "lib/types/file.h"

#include "core/board-base/vadp_interface.h"
#include "../../core/board-base/composition/bus_fixture.h"
#include "core/machine/lifecycle.h"
#include "core/machine/machine_private.h"
#include "core/machine/machine_interface.h"
#include "../support/model40_session_assets.h"

lib_i32 main(void)
{
    vm_machine *session = LIB_NULL;
    x86_video_snapshot snapshot;
    static const lib_u8 text[] = { 'O', 0x07u, 'K', 0x07u };
    lib_i32 failed = 0;

    failed |= vm_model40_fixture_create(&session) !=
        LIB_STATUS_OK || session == LIB_NULL;
    if (!failed) {
        /* This Feature-register receiver needs a real two-character text row. */
        const lib_u8 geometry[][2] = {
            {1u, 1u}, {9u, 7u}, {0x12u, 7u}, {0x13u, 1u},
            {0x17u, 0xa3u}, {0x18u, 0xffu}
        };

        for (lib_size index = 0u; index < sizeof(geometry) / sizeof(geometry[0]); ++index) {
            test_core_machine_fixture_write_port(session->core_machine, 0x03d4u,
                geometry[index][0]);
            test_core_machine_fixture_write_port(session->core_machine, 0x03d5u,
                geometry[index][1]);
        }
        test_core_machine_fixture_write_port(session->core_machine, 0x03ceu, 6u);
        test_core_machine_fixture_write_port(session->core_machine, 0x03cfu, 0x0eu);
        failed |= core_machine_memory_write(session->core_machine,
            0x000b8000u, text, sizeof(text)) != LIB_STATUS_OK ||
            core_machine_capture_display_snapshot(session->board, &snapshot) !=
                LIB_STATUS_OK || snapshot.kind != X86_VIDEO_KIND_TEXT ||
            snapshot.characters[0] != 'O' || snapshot.characters[1] != 'K';
    }
    if (!failed) {
        failed |= test_core_machine_fixture_read_bus(session->core_machine,
            0x07c6u) != 0x0du;
        for (lib_u8 feature = 0u; feature < 4u; ++feature) {
            test_core_machine_fixture_write_port(session->core_machine,
                0x03dau, feature);
            failed |= test_core_machine_fixture_read_bus(session->core_machine,
                0x07c6u) != 0x0du;
        }
        test_core_machine_fixture_write_port(session->core_machine,
            0x07c6u, 0xffu);
        failed |= test_core_machine_fixture_read_bus(session->core_machine,
            0x07c6u) != 0x0du;
    }
    if (!failed) {
        vm_machine_reset(session);
        failed |= test_core_machine_fixture_read_bus(session->core_machine,
            0x07c6u) != 0x0du;
    }
    vm_machine_destroy(session);
    if (!failed) {
        lib_c_printf("MODEL40-FEATURE-ENVIRONMENT:OK\n");
        return 0;
    }
    lib_c_fprintf(lib_c_stderr, "MODEL40-FEATURE-ENVIRONMENT:FAIL\n");
    return 1;
}
