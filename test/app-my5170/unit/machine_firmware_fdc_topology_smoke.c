#include "x86-nxvm/machine/machine_interface.h"
#include "lib/types/file.h"
#include "lib/types/types_interface.h"

#include "../../x86-nxvm/board-base/composition/composition_fixture.h"
#include "../../x86-nxvm/board-base/support/composition_fixture.h"
#include "x86-nxvm/machine/machine_private.h"
#include "x86-nxvm/machine/machine_interface.h"
#include "app-my5170/machine_config_interface.h"
#include "../support/session_assets.h"

lib_i32 main(void)
{
    const x86_nxvm_machine_config config = {0};
    const x86_nxvm_board_pc_at_descriptor *profile =
        vm_profile_ibm_5170_model_339_descriptor_get();
    const x86_nxvm_board_at_route *route;
    x86_nxvm_machine *session = LIB_NULL;
    lib_i32 failed = 1;
    if (profile == LIB_NULL ||
        profile->firmware_slot !=
            X86_NXVM_BOARD_PC_AT_FIRMWARE_SLOT_IBM_5170_REV3_ABSTRACT ||
        profile->diskette_drive_a_field_upgrade || profile->cmos.floppy_type != 0x20u ||
        vm_test_ibm_5170_session_create(&config, &session) != LIB_STATUS_OK ||
        session == LIB_NULL) goto done;

    route = x86_nxvm_board_at_route_find(profile->routes, profile->route_count,
        X86_NXVM_BOARD_AT_ROUTE_FDC_IRQ6_DMA2);
    if (route == LIB_NULL || route->irq != 6u ||
        route->dma_channel != 2u ||
        !test_x86_nxvm_port_accepts_write(session->x86_core_machine, 0x03f2u) ||
        !test_x86_nxvm_port_accepts_read(session->x86_core_machine, 0x03f4u) ||
        !test_x86_nxvm_port_accepts_write(session->x86_core_machine, 0x03f5u)) goto done;
    const test_board_composition_observation composition =
        test_board_capture_composition(session->board);
    failed = composition.fdc.irq != route->irq ||
        composition.fdc.dma_channel != route->dma_channel;
done:
    x86_nxvm_machine_destroy(session);
    if (failed) return 1;
    lib_c_printf("MODEL339-FIRMWARE-FDC-TOPOLOGY:OK\n");
    return 0;
}
