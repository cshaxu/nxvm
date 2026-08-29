#include "type.h"

#include "core/machine/machine.h"
#include "core/machine/port.h"
#include "vm/composition/session/session_private.h"
#include "vm/composition/session/session_interface.h"
#include "vm/profile/default_profile/pc_at_profile.h"

int main(C_VOID)
{
    const vm_session_config config = {
        .profile_kind = VM_SESSION_PROFILE_IBM_5170_MODEL_339
    };
    const vm_profile_default_pc_at_descriptor *profile =
        vm_profile_ibm_5170_model_339_descriptor_get();
    const vm_profile_default_pc_at_route *route;
    vm_session *session = STD_NULL;
    C_INT failed = profile == STD_NULL ||
        profile->firmware_slot !=
            VM_PROFILE_DEFAULT_PC_AT_FIRMWARE_SLOT_IBM_5170_REV3_ABSTRACT ||
        profile->diskette_drive_a_field_upgrade || profile->cmos.floppy_type != 0x20u ||
        vm_session_create(&config, &session) != TYPE_STATUS_OK || session == STD_NULL;

    route = vm_profile_default_pc_at_route_find(profile,
        VM_PROFILE_DEFAULT_PC_AT_ROUTE_FDC_IRQ6_DMA2);
    if (!failed) failed |= route == STD_NULL || route->irq != 6u ||
        route->dma_channel != 2u ||
        !core_machine_port_has_write(&session->core_machine->executor_port, 0x03f2u) ||
        !core_machine_port_has_read(&session->core_machine->executor_port, 0x03f4u) ||
        !core_machine_port_has_write(&session->core_machine->executor_port, 0x03f5u) ||
        session->core_machine->fdc_topology.config.irq != route->irq ||
        session->core_machine->fdc_topology.config.dma_channel != route->dma_channel;
    vm_session_destroy(session);
    if (failed) return 1;
    STD_PRINTF("M5:T366:S7:MODEL339-FIRMWARE-FDC-TOPOLOGY:OK\n");
    return 0;
}
