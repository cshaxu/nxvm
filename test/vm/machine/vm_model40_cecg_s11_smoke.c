#include "type.h"

#include "core/machine/machine.h"
#include "core/machine/memory.h"
#include "core/machine/port.h"
#include "core/machine/vadp.h"
#include "vm/composition/session/lifecycle.h"
#include "vm/composition/session/session_private.h"
#include "vm/composition/session/session_interface.h"
#include "../support/vm_model40_byob_fixture.h"

static C_INT t386_s11_session_route(const vm_session *session,
    type_unsigned_32 physical, core_machine_memory_route expected)
{
    core_machine_memory_route route;

    return core_machine_memory_query_physical(&session->core_machine->executor_memory,
        physical, 1u, CORE_MACHINE_MEMORY_ACCESS_READ, &route) == TYPE_STATUS_OK &&
        route == expected;
}

C_INT main(C_VOID)
{
    vm_session *session = STD_NULL;
    C_INT failed = 0;

    failed |= vm_model40_fixture_create("t386-s11-even.bin", "t386-s11-odd.bin", &session) !=
        TYPE_STATUS_OK || session == STD_NULL;
    if (!failed) {
        core_machine_port_write(&session->core_machine->executor_port,
            CORE_MACHINE_VADP_PORT_GRAPHICS_INDEX, 6u);
        core_machine_port_write(&session->core_machine->executor_port,
            CORE_MACHINE_VADP_PORT_GRAPHICS_DATA, 0x05u);
        failed |= !t386_s11_session_route(session, 0x000a0000u,
            CORE_MACHINE_MEMORY_ROUTE_PROVIDER);
        /* Model 40's low B0000h page is an unpopulated D4 decode.  It is not
         * ordinary RAM merely because the current EGA map selects A0000h. */
        failed |= !t386_s11_session_route(session, 0x000b0000u,
            CORE_MACHINE_MEMORY_ROUTE_PROVIDER);
        /* Display enable suppresses presentation, not the CPU's mapped EGA
         * aperture.  Firmware clears text VRAM before it enables output. */
        (C_VOID)core_machine_port_read(&session->core_machine->executor_port,
            CORE_MACHINE_VADP_PORT_STATUS);
        core_machine_port_write(&session->core_machine->executor_port,
            CORE_MACHINE_VADP_PORT_ATTRIBUTE, 0x00u);
        failed |= !t386_s11_session_route(session, 0x000a0000u,
            CORE_MACHINE_MEMORY_ROUTE_PROVIDER);
        (C_VOID)core_machine_port_read(&session->core_machine->executor_port,
            CORE_MACHINE_VADP_PORT_STATUS);
        core_machine_port_write(&session->core_machine->executor_port,
            CORE_MACHINE_VADP_PORT_ATTRIBUTE, 0x20u);
        core_machine_port_write(&session->core_machine->executor_port,
            CORE_MACHINE_VADP_PORT_COMPAQ_MISCELLANEOUS_OUTPUT, 0x02u);
        failed |= !t386_s11_session_route(session, 0x000a0000u,
            CORE_MACHINE_MEMORY_ROUTE_ORDINARY_RAM);
        core_machine_port_write(&session->core_machine->executor_port,
            CORE_MACHINE_VADP_PORT_COMPAQ_MISCELLANEOUS_OUTPUT, 0x00u);
        failed |= !t386_s11_session_route(session, 0x000a0000u,
            CORE_MACHINE_MEMORY_ROUTE_PROVIDER);
        core_machine_port_write(&session->core_machine->executor_port,
            CORE_MACHINE_VADP_PORT_COMPAQ_MISCELLANEOUS_OUTPUT, 0x02u);
        core_machine_port_write(&session->core_machine->executor_port,
            CORE_MACHINE_VADP_PORT_GRAPHICS_INDEX, 6u);
        core_machine_port_write(&session->core_machine->executor_port,
            CORE_MACHINE_VADP_PORT_GRAPHICS_DATA, 0x09u);
        failed |= !t386_s11_session_route(session, 0x000a0000u,
            CORE_MACHINE_MEMORY_ROUTE_ORDINARY_RAM);
        failed |= !t386_s11_session_route(session, 0x000b0000u,
            CORE_MACHINE_MEMORY_ROUTE_PROVIDER);
    }
    if (!failed) {
        vm_session_reset(session);
        core_machine_port_write(&session->core_machine->executor_port,
            CORE_MACHINE_VADP_PORT_GRAPHICS_INDEX, 6u);
        core_machine_port_write(&session->core_machine->executor_port,
            CORE_MACHINE_VADP_PORT_GRAPHICS_DATA, 0x05u);
        failed |= !t386_s11_session_route(session, 0x000a0000u,
            CORE_MACHINE_MEMORY_ROUTE_PROVIDER);
    }
    vm_session_destroy(session);
    vm_model40_fixture_remove("t386-s11-even.bin", "t386-s11-odd.bin");
    if (!failed) {
        STD_PRINTF("M5:T386:S11:MODEL40-CPU-VIDEO-GATE:OK\n");
        return 0;
    }
    STD_FPRINTF(STD_STDERR, "M5:T386:S11:MODEL40-CPU-VIDEO-GATE:FAIL\n");
    return 1;
}
