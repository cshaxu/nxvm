#include "core/machine/machine_interface.h"
#include "lib/types/types_interface.h"
#include "core/board-base/machine_board_interface.h"
#include <stdio.h>

#include "../../../core/board-base/composition/bus_fixture.h"
#include "../../../core/board-base/composition/composition_fixture.h"
#include "../../../core/board-base/support/composition_fixture.h"
#include "core/machine/lifecycle.h"
#include "core/machine/machine_private.h"
#include "core/machine/machine_interface.h"
#include "../../../core/machine/qualification/model40_session_assets.h"

lib_i32 main(void)
{
    vm_machine *session = LIB_NULL;
    lib_i32 failed = 0;
    const core_machine_dma_channel_provider duplicate_provider = {0};
    core_machine_dma_request_binding duplicate_request = {0};

    if (vm_model40_fixture_create(&session) != LIB_STATUS_OK ||
        session == LIB_NULL || !test_board_capture_composition(session->board).dma_configured ||
        !test_core_dma_timing_matches(session->core_machine, 1u, LIB_TRUE, LIB_TRUE) ||
        test_board_capture_composition(session->board).dma_wiring.fdc_channel != 2u ||
        test_board_capture_composition(session->board).dma_wiring.controller_count !=
            CORE_MACHINE_DMA_CONTROLLER_COUNT ||
        test_board_capture_composition(session->board).dma_wiring.cascade_channel !=
            CORE_MACHINE_DMA_CASCADE_CHANNEL ||
        test_board_dma_duplicate_bind(session->board, 2u,
            &duplicate_provider, &duplicate_request) !=
            LIB_STATUS_INVALID_STATE ||
        !test_core_port_has_write(session->core_machine,
            0x00d6u) || !test_core_port_has_write(
            session->core_machine, 0x00d4u)) {
        failed = 1;
        goto done;
    }

    test_core_machine_fixture_write_port(session->core_machine, 0x00d6u,
        0xc0u);
    test_core_machine_fixture_write_port(session->core_machine, 0x00d4u,
        0u);
    test_core_machine_fixture_write_port(session->core_machine, 0x000bu,
        0x86u);
    test_core_machine_fixture_write_port(session->core_machine, 0x000eu,
        0u);
    test_core_machine_fixture_write_port(session->core_machine, 0x0009u,
        0x06u);
    if (!test_board_dma_has_pending_request(session->board)) {
        failed = 1;
        goto done;
    }

    vm_machine_reset(session);
    if (test_board_dma_has_pending_request(session->board)) {
        failed = 1;
        goto done;
    }

done:
    vm_machine_destroy(session);
    if (failed) return 1;
    printf("DUAL-DMA-TOPOLOGY:OK\n");
    printf("DMA-WORD-CASCADE:OK\n");
    printf("DMA-RESET-BINDING:OK\n");
    printf("D4-DMA-GRANT-WAIT:OK\n");
    printf("D4-DMA-BUSRDY:OK\n");
    return 0;
}
