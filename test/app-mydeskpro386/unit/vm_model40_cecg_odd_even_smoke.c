#include "x86-nxvm/machine/machine_interface.h"
#include "lib/types/types_interface.h"
#include "x86-nxvm/board-base/machine_board_interface.h"
#include "lib/types/file.h"

#include "x86-nxvm/board-base/display_interface.h"
#include "x86-nxvm/board-base/vadp_interface.h"
#include "../../x86-nxvm/board-base/composition/bus_fixture.h"
#include "x86-nxvm/machine/lifecycle.h"
#include "x86-nxvm/machine/machine_private.h"
#include "x86-nxvm/machine/machine_interface.h"
#include "../support/model40_session_assets.h"

static lib_i32 vm_model40_cecg_session_write(x86_nxvm_machine *session, lib_u8 value)
{
    return x86_core_machine_memory_write(session->x86_core_machine,
        X86_NXVM_BOARD_VADP_EGA_APERTURE_BASE, &value,
        sizeof(value)) == LIB_STATUS_OK;
}

static void vm_model40_cecg_select_ega_320(x86_nxvm_machine *session)
{
    (void)test_x86_nxvm_machine_fixture_read_bus(session->x86_core_machine, 0x03dau);
    test_x86_nxvm_machine_fixture_write_port(session->x86_core_machine, 0x03c0u, 0x30u);
    test_x86_nxvm_machine_fixture_write_port(session->x86_core_machine, 0x03c0u, 1u);
    test_x86_nxvm_machine_fixture_write_port(session->x86_core_machine,
        0x03d4u, 0x01u);
    test_x86_nxvm_machine_fixture_write_port(session->x86_core_machine,
        0x03d5u, 0x27u);
    test_x86_nxvm_machine_fixture_write_port(session->x86_core_machine,
        0x03d4u, 0x07u);
    test_x86_nxvm_machine_fixture_write_port(session->x86_core_machine,
        0x03d5u, 0x10u);
    test_x86_nxvm_machine_fixture_write_port(session->x86_core_machine,
        0x03d4u, 0x12u);
    test_x86_nxvm_machine_fixture_write_port(session->x86_core_machine,
        0x03d5u, 0xc7u);
    test_x86_nxvm_machine_fixture_write_port(session->x86_core_machine,
        0x03d4u, 0x13u);
    test_x86_nxvm_machine_fixture_write_port(session->x86_core_machine,
        0x03d5u, 0x14u);
    test_x86_nxvm_machine_fixture_write_port(session->x86_core_machine,
        0x03d4u, 0x09u);
    test_x86_nxvm_machine_fixture_write_port(session->x86_core_machine,
        0x03d5u, 0u);
    test_x86_nxvm_machine_fixture_write_port(session->x86_core_machine,
        0x03d4u, 0x17u);
    test_x86_nxvm_machine_fixture_write_port(session->x86_core_machine,
        0x03d5u, 0xe3u);
    test_x86_nxvm_machine_fixture_write_port(session->x86_core_machine,
        0x03d4u, 0x18u);
    test_x86_nxvm_machine_fixture_write_port(session->x86_core_machine,
        0x03d5u, 0xffu);
}

lib_i32 main(void)
{
    x86_nxvm_machine *session = LIB_NULL;
    x86_video_snapshot snapshot;
    x86_video_snapshot_observation observation;
    lib_i32 failed = 0;

    failed |= vm_model40_fixture_create(&session) !=
        LIB_STATUS_OK || session == LIB_NULL;
    if (!failed) {
        vm_model40_cecg_select_ega_320(session);
        test_x86_nxvm_machine_fixture_write_port(session->x86_core_machine,
            0x03ceu, 6u);
        test_x86_nxvm_machine_fixture_write_port(session->x86_core_machine,
            0x03cfu, 0x07u);
        failed |= !vm_model40_cecg_session_write(session, 0x80u) ||
            x86_nxvm_board_display_capture_snapshot_from(session->display_provider,
            &snapshot) != LIB_STATUS_OK || snapshot.pixels[0] != 5u;
        failed |= x86_core_machine_memory_write(session->x86_core_machine,
            X86_NXVM_BOARD_VADP_EGA_APERTURE_BASE + 2u, &(lib_u8){0x80u}, 1u) != LIB_STATUS_OK;
        test_x86_nxvm_machine_fixture_write_port(session->x86_core_machine,
            0x03c2u, 0x21u);
        failed |= !vm_model40_cecg_session_write(session, 0x00u) ||
            x86_nxvm_board_display_capture_snapshot_from(session->display_provider,
            &snapshot) != LIB_STATUS_OK || snapshot.pixels[0] != 5u ||
            x86_nxvm_board_observe_display_snapshot(session->board,
                LIB_FALSE, 0u, &observation) != LIB_STATUS_OK ||
            !observation.generation_reliable;
        test_x86_nxvm_machine_fixture_write_port(session->x86_core_machine, 0x03d4u, 0x17u);
        test_x86_nxvm_machine_fixture_write_port(session->x86_core_machine, 0x03d5u, 0xe3u);
        test_x86_nxvm_machine_fixture_write_port(session->x86_core_machine, 0x03d4u, 0x0du);
        test_x86_nxvm_machine_fixture_write_port(session->x86_core_machine, 0x03d5u, 1u);
        failed |= x86_nxvm_board_display_capture_snapshot_from(session->display_provider,
            &snapshot) != LIB_STATUS_OK || snapshot.pixels[0] != 0u;
        if (!failed) {
            /* The selected high page remains VADP-owned, so a Core write must
             * publish a fresh copied-frame generation. */
            failed |= x86_core_machine_memory_write(session->x86_core_machine,
                X86_NXVM_BOARD_VADP_EGA_APERTURE_BASE, &(lib_u8){0x5au},
                sizeof(lib_u8)) != LIB_STATUS_OK ||
                x86_nxvm_board_observe_display_snapshot(session->board,
                    LIB_TRUE, observation.generation, &observation) != LIB_STATUS_OK ||
                !observation.generation_reliable || !observation.capture_required;
        }
    }
    if (!failed) {
        x86_nxvm_machine_reset(session);
        vm_model40_cecg_select_ega_320(session);
        test_x86_nxvm_machine_fixture_write_port(session->x86_core_machine,
            0x03ceu, 6u);
        test_x86_nxvm_machine_fixture_write_port(session->x86_core_machine,
            0x03cfu, 0x07u);
        failed |= x86_nxvm_board_display_capture_snapshot_from(session->display_provider,
            &snapshot) != LIB_STATUS_OK || snapshot.pixels[0] != 0u;
    }
    x86_nxvm_machine_destroy(session);
    if (failed) {
        lib_c_fprintf(lib_c_stderr, "MODEL40-CECG-ODD-EVEN:FAIL\n");
        return 1;
    }
    lib_c_printf("MODEL40-CECG-ODD-EVEN:OK\n");
    return 0;
}
