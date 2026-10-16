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
        test_x86_nxvm_machine_fixture_write_port(session->x86_core_machine,
            0x03ceu, 6u);
        test_x86_nxvm_machine_fixture_write_port(session->x86_core_machine,
            0x03cfu, 0x05u);
        failed |= !test_video_memory_route(session->x86_core_machine, 0x000a0000u,
            X86_CORE_MACHINE_MEMORY_ROUTE_PROVIDER);
        /* Model 40's low B0000h page is an unpopulated D4 decode.  It is not
         * ordinary RAM merely because the current EGA map selects A0000h. */
        failed |= !test_video_memory_route(session->x86_core_machine, 0x000b0000u,
            X86_CORE_MACHINE_MEMORY_ROUTE_PROVIDER);
        /* Display enable suppresses presentation, not the CPU's mapped EGA
         * aperture.  Firmware clears text VRAM before it enables output. */
        (void)test_x86_nxvm_machine_fixture_read_bus(session->x86_core_machine,
            0x03dau);
        test_x86_nxvm_machine_fixture_write_port(session->x86_core_machine,
            0x03c0u, 0x00u);
        failed |= !test_video_memory_route(session->x86_core_machine, 0x000a0000u,
            X86_CORE_MACHINE_MEMORY_ROUTE_PROVIDER);
        (void)test_x86_nxvm_machine_fixture_read_bus(session->x86_core_machine,
            0x03dau);
        test_x86_nxvm_machine_fixture_write_port(session->x86_core_machine,
            0x03c0u, 0x20u);
        test_x86_nxvm_machine_fixture_write_port(session->x86_core_machine,
            0x03c2u, 0x02u);
        failed |= !test_video_memory_route(session->x86_core_machine, 0x000a0000u,
            X86_CORE_MACHINE_MEMORY_ROUTE_PROVIDER);
        test_x86_nxvm_machine_fixture_write_port(session->x86_core_machine,
            0x03c2u, 0x00u);
        failed |= !test_video_memory_route(session->x86_core_machine, 0x000a0000u,
            X86_CORE_MACHINE_MEMORY_ROUTE_ORDINARY_RAM);
        test_x86_nxvm_machine_fixture_write_port(session->x86_core_machine,
            0x03c2u, 0x02u);
        test_x86_nxvm_machine_fixture_write_port(session->x86_core_machine,
            0x03ceu, 6u);
        test_x86_nxvm_machine_fixture_write_port(session->x86_core_machine,
            0x03cfu, 0x09u);
        failed |= !test_video_memory_route(session->x86_core_machine, 0x000a0000u,
            X86_CORE_MACHINE_MEMORY_ROUTE_ORDINARY_RAM);
        failed |= !test_video_memory_route(session->x86_core_machine, 0x000b0000u,
            X86_CORE_MACHINE_MEMORY_ROUTE_PROVIDER);
    }
    if (!failed) {
        x86_nxvm_machine_reset(session);
        test_x86_nxvm_machine_fixture_write_port(session->x86_core_machine,
            0x03ceu, 6u);
        test_x86_nxvm_machine_fixture_write_port(session->x86_core_machine,
            0x03cfu, 0x05u);
        failed |= !test_video_memory_route(session->x86_core_machine, 0x000a0000u,
            X86_CORE_MACHINE_MEMORY_ROUTE_PROVIDER);
    }
    x86_nxvm_machine_destroy(session);
    if (!failed) {
        lib_c_printf("MODEL40-CPU-VIDEO-GATE:OK\n");
        return 0;
    }
    lib_c_fprintf(lib_c_stderr, "MODEL40-CPU-VIDEO-GATE:FAIL\n");
    return 1;
}
