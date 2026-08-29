#include "type.h"

#include "core/machine/machine.h"
#include "core/machine/port.h"
#include "core/machine/vadp.h"
#include "vm/composition/session/lifecycle.h"
#include "vm/composition/session/session_private.h"
#include "vm/composition/session/session_interface.h"

static C_INT vm_model_339_cga_topology(C_VOID)
{
    const vm_session_config config = {
        .profile_kind = VM_SESSION_PROFILE_IBM_5170_MODEL_339
    };
    core_machine_display_snapshot snapshot;
    vm_session *session = STD_NULL;
    C_INT failed = vm_session_create(&config, &session) != TYPE_STATUS_OK ||
        session == STD_NULL;

    if (!failed) failed |= (core_machine_port_has_read(
        &session->core_machine->executor_port, CORE_MACHINE_VADP_PORT_CRTC_INDEX) << 1) |
        (core_machine_port_has_read(&session->core_machine->executor_port,
            CORE_MACHINE_VADP_PORT_MODE) << 2) |
        (core_machine_port_has_read(&session->core_machine->executor_port,
            CORE_MACHINE_VADP_PORT_COLOR) << 3) |
        (!core_machine_port_has_write(&session->core_machine->executor_port,
            CORE_MACHINE_VADP_PORT_MODE) << 4) |
        (!core_machine_port_has_read(&session->core_machine->executor_port,
            CORE_MACHINE_VADP_PORT_STATUS) << 5) |
        (core_machine_port_has_write(&session->core_machine->executor_port,
            CORE_MACHINE_VADP_PORT_ATTRIBUTE) << 6) |
        (core_machine_port_has_read(&session->core_machine->executor_port,
            CORE_MACHINE_VADP_PORT_SEQUENCER_INDEX) << 7) |
        (core_machine_port_has_write(&session->core_machine->executor_port,
            CORE_MACHINE_VADP_PORT_GRAPHICS_DATA) << 8) |
        (session->core_machine->shared_vadp.data.ega_sequencer_configured << 9) |
        ((!core_machine_vadp_capture_text_snapshot(&session->core_machine->shared_vadp,
            &session->core_machine->executor_memory, &snapshot) ||
            snapshot.kind != CORE_MACHINE_DISPLAY_KIND_TEXT) << 10);
    if (!failed) {
        static const type_unsigned_8 expected_crtc[] = {
            0x71u, 0x50u, 0x5au, 0x0au, 0x1fu, 0x06u, 0x19u, 0x1cu,
            0x02u, 0x07u, 0x06u, 0x07u, 0x00u, 0x00u, 0x01u, 0x90u
        };
        STD_SIZE_T index;

        for (index = 0u; index < sizeof(expected_crtc); ++index) {
            failed |= session->core_machine->shared_vadp.data.crtc[index] !=
                expected_crtc[index];
        }
        core_machine_port_write(&session->core_machine->executor_port,
            CORE_MACHINE_VADP_PORT_CRTC_INDEX, 0x13u);
        core_machine_port_write(&session->core_machine->executor_port,
            CORE_MACHINE_VADP_PORT_CRTC_DATA, 0x28u);
        failed |= session->core_machine->shared_vadp.data.crtc[0x13u] != 0u ||
            core_machine_port_read(&session->core_machine->executor_port,
                CORE_MACHINE_VADP_PORT_CRTC_DATA) != 0u;
    }
    vm_session_destroy(session);
    return failed;
}

static C_INT vm_default_ega_topology(C_VOID)
{
    vm_session *session = STD_NULL;
    C_INT failed = vm_session_create(STD_NULL, &session) != TYPE_STATUS_OK ||
        session == STD_NULL;

    if (!failed) failed |= (!core_machine_port_has_write(
        &session->core_machine->executor_port, CORE_MACHINE_VADP_PORT_ATTRIBUTE) << 1) |
        (!core_machine_port_has_read(&session->core_machine->executor_port,
            CORE_MACHINE_VADP_PORT_MODE) << 2) |
        (!core_machine_port_has_read(&session->core_machine->executor_port,
            CORE_MACHINE_VADP_PORT_COLOR) << 3) |
        (!core_machine_port_has_read(&session->core_machine->executor_port,
            CORE_MACHINE_VADP_PORT_SEQUENCER_INDEX) << 4) |
        (!session->core_machine->shared_vadp.data.ega_sequencer_configured << 5) |
        (!session->core_machine->shared_vadp.data.ega_controller_configured << 6);
    vm_session_destroy(session);
    return failed;
}

C_INT main(C_VOID)
{
    if (vm_model_339_cga_topology() || vm_default_ega_topology()) return 1;
    STD_PRINTF("M5:T366:S6:MODEL339-CGA-TOPOLOGY:OK\n");
    STD_PRINTF("M5:T375:S15:MODEL339-REV3-CGA-DEFAULTS:OK\n");
    return 0;
}
