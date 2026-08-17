#include "type.h"

#include "core/machine/display_interface.h"
#include "core/machine/machine.h"
#include "core/machine/memory.h"
#include "core/machine/port.h"
#include "core/machine/vadp.h"
#include "vm/composition/session/lifecycle.h"
#include "vm/composition/session/session.h"
#include "vm/composition/session/session_interface.h"

static C_INT t386_s28_session_write(vm_session *session, type_unsigned_8 value)
{
    return core_machine_memory_write_physical(&session->core_machine->executor_memory,
        CORE_MACHINE_VADP_EGA_APERTURE_BASE, (type_virtual_address)&value,
        sizeof(value)) == TYPE_STATUS_OK;
}

C_INT main(C_VOID)
{
    static type_unsigned_8 even[VM_PROFILE_MODEL40_ROM_CHIP_BYTES];
    static type_unsigned_8 odd[VM_PROFILE_MODEL40_ROM_CHIP_BYTES];
    vm_profile_model40_external_rom external_rom = { even, odd, sizeof(even) };
    vm_session *session = STD_NULL;
    core_machine_display_snapshot snapshot;
    C_INT failed = 0;

    failed |= vm_session_create_model40_private(&external_rom, &session) !=
        TYPE_STATUS_OK || session == STD_NULL;
    if (!failed) {
        core_machine_port_write(&session->core_machine->executor_port,
            CORE_MACHINE_VADP_PORT_GRAPHICS_INDEX, 6u);
        core_machine_port_write(&session->core_machine->executor_port,
            CORE_MACHINE_VADP_PORT_GRAPHICS_DATA, 0x07u);
        failed |= !t386_s28_session_write(session, 0x80u) ||
            !core_machine_display_capture_snapshot_from(&session->display_provider,
            &snapshot) || snapshot.pixels[0] != 15u;
        core_machine_port_write(&session->core_machine->executor_port,
            CORE_MACHINE_VADP_PORT_COMPAQ_MISCELLANEOUS_OUTPUT, 0x20u);
        failed |= !t386_s28_session_write(session, 0x00u) ||
            !core_machine_display_capture_snapshot_from(&session->display_provider,
            &snapshot) || snapshot.pixels[0] != 0u;
    }
    if (!failed) {
        vm_session_reset(session);
        core_machine_port_write(&session->core_machine->executor_port,
            CORE_MACHINE_VADP_PORT_GRAPHICS_INDEX, 6u);
        core_machine_port_write(&session->core_machine->executor_port,
            CORE_MACHINE_VADP_PORT_GRAPHICS_DATA, 0x07u);
        failed |= !core_machine_display_capture_snapshot_from(&session->display_provider,
            &snapshot) || snapshot.pixels[0] != 0u;
    }
    vm_session_destroy(session);
    if (failed) {
        STD_FPRINTF(STD_STDERR, "M5:T386:S28:MODEL40-CECG-ODD-EVEN:FAIL\n");
        return 1;
    }
    STD_PRINTF("M5:T386:S28:MODEL40-CECG-ODD-EVEN:OK\n");
    return 0;
}