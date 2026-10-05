#include "ibmpc/machine/machine_interface.h"
#include "lib/types/types_interface.h"
#include <stdio.h>

#include "ibmpc/board-common/vadp_interface.h"
#include "../../../x86/core/bus_fixture.h"
#include "../../../x86/core/video_topology_fixture.h"
#include "ibmpc/machine/lifecycle.h"
#include "ibmpc/machine/machine_private.h"
#include "ibmpc/machine/machine_interface.h"
#include "../../support/rom/model40_session_assets.h"

lib_i32 main(void)
{
    vm_machine *session = LIB_NULL;
    lib_i32 failed = 0;

    failed |= vm_model40_fixture_create(&session) !=
        LIB_STATUS_OK || session == LIB_NULL;
    if (!failed) {
        test_core_machine_fixture_write_port(session->core_machine,
            0x03ceu, 6u);
        test_core_machine_fixture_write_port(session->core_machine,
            0x03cfu, 0x05u);
        failed |= !test_video_memory_route(session->core_machine, 0x000a0000u,
            CORE_MACHINE_MEMORY_ROUTE_PROVIDER);
        /* Model 40's low B0000h page is an unpopulated D4 decode.  It is not
         * ordinary RAM merely because the current EGA map selects A0000h. */
        failed |= !test_video_memory_route(session->core_machine, 0x000b0000u,
            CORE_MACHINE_MEMORY_ROUTE_PROVIDER);
        /* Display enable suppresses presentation, not the CPU's mapped EGA
         * aperture.  Firmware clears text VRAM before it enables output. */
        (void)test_core_machine_fixture_read_bus(session->core_machine,
            0x03dau);
        test_core_machine_fixture_write_port(session->core_machine,
            0x03c0u, 0x00u);
        failed |= !test_video_memory_route(session->core_machine, 0x000a0000u,
            CORE_MACHINE_MEMORY_ROUTE_PROVIDER);
        (void)test_core_machine_fixture_read_bus(session->core_machine,
            0x03dau);
        test_core_machine_fixture_write_port(session->core_machine,
            0x03c0u, 0x20u);
        test_core_machine_fixture_write_port(session->core_machine,
            0x03c2u, 0x02u);
        failed |= !test_video_memory_route(session->core_machine, 0x000a0000u,
            CORE_MACHINE_MEMORY_ROUTE_ORDINARY_RAM);
        test_core_machine_fixture_write_port(session->core_machine,
            0x03c2u, 0x00u);
        failed |= !test_video_memory_route(session->core_machine, 0x000a0000u,
            CORE_MACHINE_MEMORY_ROUTE_PROVIDER);
        test_core_machine_fixture_write_port(session->core_machine,
            0x03c2u, 0x02u);
        test_core_machine_fixture_write_port(session->core_machine,
            0x03ceu, 6u);
        test_core_machine_fixture_write_port(session->core_machine,
            0x03cfu, 0x09u);
        failed |= !test_video_memory_route(session->core_machine, 0x000a0000u,
            CORE_MACHINE_MEMORY_ROUTE_ORDINARY_RAM);
        failed |= !test_video_memory_route(session->core_machine, 0x000b0000u,
            CORE_MACHINE_MEMORY_ROUTE_PROVIDER);
    }
    if (!failed) {
        vm_machine_reset(session);
        test_core_machine_fixture_write_port(session->core_machine,
            0x03ceu, 6u);
        test_core_machine_fixture_write_port(session->core_machine,
            0x03cfu, 0x05u);
        failed |= !test_video_memory_route(session->core_machine, 0x000a0000u,
            CORE_MACHINE_MEMORY_ROUTE_PROVIDER);
    }
    vm_machine_destroy(session);
    if (!failed) {
        printf("M5:T386:S11:MODEL40-CPU-VIDEO-GATE:OK\n");
        return 0;
    }
    fprintf(stderr, "M5:T386:S11:MODEL40-CPU-VIDEO-GATE:FAIL\n");
    return 1;
}
