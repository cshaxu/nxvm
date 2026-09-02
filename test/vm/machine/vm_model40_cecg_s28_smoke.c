#include "type.h"

#include "core/machine/display_interface.h"
#include "core/machine/machine.h"
#include "core/machine/memory.h"
#include "core/machine/port.h"
#include "core/machine/vadp.h"
#include "vm/composition/session/lifecycle.h"
#include "vm/composition/session/session_private.h"
#include "vm/composition/session/session_interface.h"
#include "../support/vm_model40_byob_fixture.h"

static C_INT t386_s28_session_write(vm_session *session, type_unsigned_8 value)
{
    return core_machine_memory_write_physical(&session->core_machine->executor_memory,
        CORE_MACHINE_VADP_EGA_APERTURE_BASE, (type_virtual_address)&value,
        sizeof(value)) == TYPE_STATUS_OK;
}

static C_VOID t386_s28_select_ega_320(vm_session *session)
{
    core_machine_port_write(&session->core_machine->executor_port,
        CORE_MACHINE_VADP_PORT_CRTC_INDEX, 0x01u);
    core_machine_port_write(&session->core_machine->executor_port,
        CORE_MACHINE_VADP_PORT_CRTC_DATA, 0x27u);
    core_machine_port_write(&session->core_machine->executor_port,
        CORE_MACHINE_VADP_PORT_CRTC_INDEX, 0x07u);
    core_machine_port_write(&session->core_machine->executor_port,
        CORE_MACHINE_VADP_PORT_CRTC_DATA, 0x00u);
    core_machine_port_write(&session->core_machine->executor_port,
        CORE_MACHINE_VADP_PORT_CRTC_INDEX, 0x12u);
    core_machine_port_write(&session->core_machine->executor_port,
        CORE_MACHINE_VADP_PORT_CRTC_DATA, 0xc7u);
    core_machine_port_write(&session->core_machine->executor_port,
        CORE_MACHINE_VADP_PORT_CRTC_INDEX, 0x13u);
    core_machine_port_write(&session->core_machine->executor_port,
        CORE_MACHINE_VADP_PORT_CRTC_DATA, 0x14u);
}

C_INT main(C_VOID)
{
    vm_session *session = STD_NULL;
    core_machine_display_snapshot snapshot;
    core_machine_display_snapshot_observation observation;
    C_INT failed = 0;

    failed |= vm_model40_fixture_create("t386-s28-even.bin", "t386-s28-odd.bin", &session) !=
        TYPE_STATUS_OK || session == STD_NULL;
    if (!failed) {
        t386_s28_select_ega_320(session);
        core_machine_port_write(&session->core_machine->executor_port,
            CORE_MACHINE_VADP_PORT_GRAPHICS_INDEX, 6u);
        core_machine_port_write(&session->core_machine->executor_port,
            CORE_MACHINE_VADP_PORT_GRAPHICS_DATA, 0x07u);
        failed |= !t386_s28_session_write(session, 0x80u) ||
            !core_machine_display_capture_snapshot_from(session->display_provider,
            &snapshot) || snapshot.pixels[0] != 15u;
        core_machine_port_write(&session->core_machine->executor_port,
            CORE_MACHINE_VADP_PORT_COMPAQ_MISCELLANEOUS_OUTPUT, 0x20u);
        failed |= !t386_s28_session_write(session, 0x00u) ||
            !core_machine_display_capture_snapshot_from(session->display_provider,
            &snapshot) || snapshot.pixels[0] != 0u ||
            core_machine_observe_display_snapshot(session->core_machine,
                TYPE_FALSE, 0u, &observation) != TYPE_STATUS_OK ||
            !observation.generation_reliable;
        if (!failed) {
            /* The selected high page remains VADP-owned, so a Core write must
             * publish a fresh copied-frame generation. */
            failed |= core_machine_memory_write(session->core_machine,
                CORE_MACHINE_VADP_EGA_APERTURE_BASE, &(type_unsigned_8){0x5au},
                sizeof(type_unsigned_8)) != TYPE_STATUS_OK ||
                core_machine_observe_display_snapshot(session->core_machine,
                    TYPE_TRUE, observation.generation, &observation) != TYPE_STATUS_OK ||
                !observation.generation_reliable || !observation.capture_required;
        }
    }
    if (!failed) {
        vm_session_reset(session);
        t386_s28_select_ega_320(session);
        core_machine_port_write(&session->core_machine->executor_port,
            CORE_MACHINE_VADP_PORT_GRAPHICS_INDEX, 6u);
        core_machine_port_write(&session->core_machine->executor_port,
            CORE_MACHINE_VADP_PORT_GRAPHICS_DATA, 0x07u);
        failed |= !core_machine_display_capture_snapshot_from(session->display_provider,
            &snapshot) || snapshot.pixels[0] != 0u;
    }
    vm_session_destroy(session);
    vm_model40_fixture_remove("t386-s28-even.bin", "t386-s28-odd.bin");
    if (failed) {
        STD_FPRINTF(STD_STDERR, "M5:T386:S28:MODEL40-CECG-ODD-EVEN:FAIL\n");
        return 1;
    }
    STD_PRINTF("M5:T386:S28:MODEL40-CECG-ODD-EVEN:OK\n");
    return 0;
}
