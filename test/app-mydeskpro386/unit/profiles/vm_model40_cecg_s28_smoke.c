#include "ibmpc/machine/machine_interface.h"
#include "lib/types/types_interface.h"
#include "ibmpc/board-common/machine_board_interface.h"
#include <stdio.h>

#include "ibmpc/board-common/display_interface.h"
#include "ibmpc/board-common/vadp_interface.h"
#include "../../../x86/core/bus_fixture.h"
#include "ibmpc/machine/lifecycle.h"
#include "ibmpc/machine/machine_private.h"
#include "ibmpc/machine/machine_interface.h"
#include "../../support/rom/model40_session_assets.h"

static lib_i32 t386_s28_session_write(vm_machine *session, lib_u8 value)
{
    return core_machine_memory_write(session->core_machine,
        CORE_MACHINE_VADP_EGA_APERTURE_BASE, &value,
        sizeof(value)) == LIB_STATUS_OK;
}

static void t386_s28_select_ega_320(vm_machine *session)
{
    test_core_machine_fixture_write_port(session->core_machine,
        0x03d4u, 0x01u);
    test_core_machine_fixture_write_port(session->core_machine,
        0x03d5u, 0x27u);
    test_core_machine_fixture_write_port(session->core_machine,
        0x03d4u, 0x07u);
    test_core_machine_fixture_write_port(session->core_machine,
        0x03d5u, 0x00u);
    test_core_machine_fixture_write_port(session->core_machine,
        0x03d4u, 0x12u);
    test_core_machine_fixture_write_port(session->core_machine,
        0x03d5u, 0xc7u);
    test_core_machine_fixture_write_port(session->core_machine,
        0x03d4u, 0x13u);
    test_core_machine_fixture_write_port(session->core_machine,
        0x03d5u, 0x14u);
}

lib_i32 main(void)
{
    vm_machine *session = LIB_NULL;
    x86_video_snapshot snapshot;
    x86_video_snapshot_observation observation;
    lib_i32 failed = 0;

    failed |= vm_model40_fixture_create(&session) !=
        LIB_STATUS_OK || session == LIB_NULL;
    if (!failed) {
        t386_s28_select_ega_320(session);
        test_core_machine_fixture_write_port(session->core_machine,
            0x03ceu, 6u);
        test_core_machine_fixture_write_port(session->core_machine,
            0x03cfu, 0x07u);
        failed |= !t386_s28_session_write(session, 0x80u) ||
            !core_machine_display_capture_snapshot_from(session->display_provider,
            &snapshot) || snapshot.pixels[0] != 15u;
        test_core_machine_fixture_write_port(session->core_machine,
            0x03c2u, 0x20u);
        failed |= !t386_s28_session_write(session, 0x00u) ||
            !core_machine_display_capture_snapshot_from(session->display_provider,
            &snapshot) || snapshot.pixels[0] != 0u ||
            core_machine_observe_display_snapshot(session->board,
                LIB_FALSE, 0u, &observation) != LIB_STATUS_OK ||
            !observation.generation_reliable;
        if (!failed) {
            /* The selected high page remains VADP-owned, so a Core write must
             * publish a fresh copied-frame generation. */
            failed |= core_machine_memory_write(session->core_machine,
                CORE_MACHINE_VADP_EGA_APERTURE_BASE, &(lib_u8){0x5au},
                sizeof(lib_u8)) != LIB_STATUS_OK ||
                core_machine_observe_display_snapshot(session->board,
                    LIB_TRUE, observation.generation, &observation) != LIB_STATUS_OK ||
                !observation.generation_reliable || !observation.capture_required;
        }
    }
    if (!failed) {
        vm_machine_reset(session);
        t386_s28_select_ega_320(session);
        test_core_machine_fixture_write_port(session->core_machine,
            0x03ceu, 6u);
        test_core_machine_fixture_write_port(session->core_machine,
            0x03cfu, 0x07u);
        failed |= !core_machine_display_capture_snapshot_from(session->display_provider,
            &snapshot) || snapshot.pixels[0] != 0u;
    }
    vm_machine_destroy(session);
    if (failed) {
        fprintf(stderr, "M5:T386:S28:MODEL40-CECG-ODD-EVEN:FAIL\n");
        return 1;
    }
    printf("M5:T386:S28:MODEL40-CECG-ODD-EVEN:OK\n");
    return 0;
}
