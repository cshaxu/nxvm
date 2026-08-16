#include "type.h"

#include "core/machine/machine.h"
#include "core/machine/memory.h"
#include "core/machine/port.h"
#include "core/machine/vadp.h"
#include "vm/composition/session/lifecycle.h"
#include "vm/composition/session/session.h"
#include "vm/composition/session/session_interface.h"

static C_INT t386_s11_session_route(const vm_session *session,
    core_machine_memory_route expected)
{
    core_machine_memory_route route;

    return core_machine_memory_query_physical(&session->core_machine->executor_memory,
        CORE_MACHINE_VADP_EGA_APERTURE_BASE, 1u,
        CORE_MACHINE_MEMORY_ACCESS_READ, &route) == TYPE_STATUS_OK && route == expected;
}

C_INT main(C_VOID)
{
    static type_unsigned_8 rom[VM_PROFILE_MODEL40_ROM_BYTES];
    vm_profile_model40_external_rom external_rom = { rom, sizeof(rom) };
    vm_session *session = STD_NULL;
    C_INT failed = 0;

    failed |= vm_session_create_model40_private(&external_rom, &session) !=
        TYPE_STATUS_OK || session == STD_NULL;
    if (!failed) {
        core_machine_port_write(&session->core_machine->executor_port,
            CORE_MACHINE_VADP_PORT_GRAPHICS_INDEX, 6u);
        core_machine_port_write(&session->core_machine->executor_port,
            CORE_MACHINE_VADP_PORT_GRAPHICS_DATA, 0x05u);
        failed |= !t386_s11_session_route(session, CORE_MACHINE_MEMORY_ROUTE_PROVIDER);
        core_machine_port_write(&session->core_machine->executor_port,
            CORE_MACHINE_VADP_PORT_COMPAQ_MISCELLANEOUS_OUTPUT, 0x02u);
        failed |= !t386_s11_session_route(session, CORE_MACHINE_MEMORY_ROUTE_ORDINARY_RAM);
    }
    if (!failed) {
        vm_session_reset(session);
        core_machine_port_write(&session->core_machine->executor_port,
            CORE_MACHINE_VADP_PORT_GRAPHICS_INDEX, 6u);
        core_machine_port_write(&session->core_machine->executor_port,
            CORE_MACHINE_VADP_PORT_GRAPHICS_DATA, 0x05u);
        failed |= !t386_s11_session_route(session, CORE_MACHINE_MEMORY_ROUTE_PROVIDER);
    }
    vm_session_destroy(session);
    if (!failed) {
        STD_PRINTF("M5:T386:S11:MODEL40-CPU-VIDEO-GATE:OK\n");
        return 0;
    }
    STD_FPRINTF(STD_STDERR, "M5:T386:S11:MODEL40-CPU-VIDEO-GATE:FAIL\n");
    return 1;
}