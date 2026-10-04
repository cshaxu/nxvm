#include "lib/types/types_interface.h"
#include <stdio.h>

#include "../../../../../../x86/core/composition_fixture.h"
#include "../../../../../../x86/ibmpc-common/composition_fixture.h"
#include "app-nxvm/machine/machine_private.h"
#include "app-nxvm/machine/machine_interface.h"
#include "app-nxvm/profiles/default_profile/pc_at_profile_private.h"
#include "../../../machine/support/rom/session_assets.h"

int main(void)
{
    const vm_machine_config config = {
        .profile_kind = VM_MACHINE_PROFILE_IBM_5170_MODEL_339
    };
    const vm_profile_default_pc_at_descriptor *profile =
        vm_profile_ibm_5170_model_339_descriptor_get();
    const vm_profile_default_pc_at_route *route;
    vm_machine *session = LIB_NULL;
    lib_i32 failed = 1;
    if (profile == LIB_NULL ||
        profile->firmware_slot !=
            VM_PROFILE_DEFAULT_PC_AT_FIRMWARE_SLOT_IBM_5170_REV3_ABSTRACT ||
        profile->diskette_drive_a_field_upgrade || profile->cmos.floppy_type != 0x20u ||
        vm_test_ibm_5170_session_create(&config, &session) != LIB_STATUS_OK ||
        session == LIB_NULL) goto done;

    route = vm_profile_default_pc_at_route_find(profile,
        VM_PROFILE_DEFAULT_PC_AT_ROUTE_FDC_IRQ6_DMA2);
    if (route == LIB_NULL || route->irq != 6u ||
        route->dma_channel != 2u ||
        !test_core_port_has_write(session->core_machine, 0x03f2u) ||
        !test_core_port_has_read(session->core_machine, 0x03f4u) ||
        !test_core_port_has_write(session->core_machine, 0x03f5u)) goto done;
    const test_board_composition_observation composition =
        test_board_capture_composition(session->board);
    failed = composition.fdc.irq != route->irq ||
        composition.fdc.dma_channel != route->dma_channel;
done:
    vm_machine_destroy(session);
    if (failed) return 1;
    printf("M5:T366:S7:MODEL339-FIRMWARE-FDC-TOPOLOGY:OK\n");
    return 0;
}
