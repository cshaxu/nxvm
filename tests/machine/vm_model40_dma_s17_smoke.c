#include "type.h"

#include "core/machine/dma.h"
#include "core/machine/machine.h"
#include "core/machine/port.h"
#include "vm/composition/session/lifecycle.h"
#include "vm/composition/session/session_private.h"
#include "vm/composition/session/session_interface.h"
#include "../support/vm_model40_byob_fixture.h"

C_INT main(C_VOID)
{
    vm_session *session = STD_NULL;
    C_INT failed = 0;

    if (vm_model40_fixture_create("t386-s17-even.bin", "t386-s17-odd.bin", &session) != TYPE_STATUS_OK ||
        session == STD_NULL || !session->core_machine->dma_configured ||
        session->core_machine->transaction_contract.dma_cycle_wait_quanta != 1u ||
        !session->core_machine->transaction_contract.dma_cycle_bus_ready_gate_enabled ||
        !session->core_machine->dma_cycle_bus_ready ||
        session->core_machine->dma_wiring.fdc_channel != 2u ||
        session->core_machine->dma_wiring.controller_count !=
            CORE_MACHINE_DMA_CONTROLLER_COUNT ||
        session->core_machine->dma_wiring.cascade_channel !=
            CORE_MACHINE_DMA_CASCADE_CHANNEL ||
        session->core_machine->shared_dma_primary.connect.peer !=
            &session->core_machine->shared_dma_secondary ||
        session->core_machine->shared_dma_secondary.connect.peer !=
            &session->core_machine->shared_dma_primary ||
        session->core_machine->shared_dma_primary.connect.device_owner[2u] !=
            &session->core_machine->fdc ||
        !core_machine_port_has_write(&session->core_machine->executor_port,
            0x00d6u) || !core_machine_port_has_write(
            &session->core_machine->executor_port, 0x00d4u)) {
        failed = 1;
        goto done;
    }

    core_machine_port_write(&session->core_machine->executor_port, 0x00d6u,
        0xc0u);
    core_machine_port_write(&session->core_machine->executor_port, 0x00d4u,
        0u);
    core_machine_port_write(&session->core_machine->executor_port, 0x000bu,
        0x86u);
    core_machine_port_write(&session->core_machine->executor_port, 0x000eu,
        0u);
    core_machine_port_write(&session->core_machine->executor_port, 0x0009u,
        0x06u);
    if (session->core_machine->shared_dma_secondary.data.mode[0u] != 0xc0u ||
        !core_machine_dma_has_pending_request(
            &session->core_machine->shared_dma_primary,
            &session->core_machine->shared_dma_secondary)) {
        failed = 1;
        goto done;
    }

    vm_session_reset(session);
    if (session->core_machine->shared_dma_primary.data.request != 0u ||
        session->core_machine->shared_dma_secondary.data.request != 0u ||
        session->core_machine->shared_dma_primary.data.mask != VDMA_MASK_VALID ||
        session->core_machine->shared_dma_secondary.data.mask != VDMA_MASK_VALID ||
        session->core_machine->shared_dma_secondary.data.mode[0u] != 0u) {
        failed = 1;
        goto done;
    }

done:
    vm_session_destroy(session);
    vm_model40_fixture_remove("t386-s17-even.bin", "t386-s17-odd.bin");
    if (failed) return 1;
    STD_PRINTF("M5:T386:S17:DUAL-DMA-TOPOLOGY:OK\n");
    STD_PRINTF("M5:T386:S17:DMA-WORD-CASCADE:OK\n");
    STD_PRINTF("M5:T386:S17:DMA-RESET-BINDING:OK\n");
    STD_PRINTF("M5:T419:S2:D4-DMA-GRANT-WAIT:OK\n");
    STD_PRINTF("M5:T419:S3:D4-DMA-BUSRDY:OK\n");
    return 0;
}
