#include "type.h"

#include "core/machine/machine.h"
#include "core/machine/media_interface.h"
#include "core/machine/port.h"

C_INT main(C_VOID)
{
    const core_machine_config config = {
        .memory_bytes = CORE_MACHINE_MINIMUM_MEMORY_BYTES,
        .cpu_profile = CORE_MACHINE_CPU_PROFILE_8088,
        .fpu_profile = CORE_MACHINE_FPU_PROFILE_NONE,
        .ticks_per_instruction = 1u
    };
    const core_machine_dma_wiring dma = {
        .fdc_channel = CORE_MACHINE_DMA_FDC_CHANNEL_UNBOUND,
        .controller_count = 1u,
        .cascade_channel = 0u
    };
    const core_machine_hdc_topology hdc = {
        .media_registry = STD_NULL,
        .media_id = 1u,
        .slave_media_id = CORE_MACHINE_MEDIA_ID_INVALID,
        .config = {
            .protocol = CORE_MACHINE_HDC_PROTOCOL_XEBEC_XT,
            .irq = 5u,
            .bus.xebec = {
                .data_port = 0x0320u,
                .hardware_status_reset_port = 0x0321u,
                .jumpers_select_port = 0x0322u,
                .dma_irq_mask_port = 0x0323u,
                .dma_channel = 3u
            }
        }
    };
    core_machine_media_registry *registry = STD_NULL;
    core_machine_dma_request_binding fdc_binding = {0};
    core_machine *machine = STD_NULL;
    core_machine_hdc_topology topology = hdc;
    C_INT failed = 0;

    if (core_machine_create(&config, &machine) != TYPE_STATUS_OK ||
        core_machine_media_registry_create(&registry) != TYPE_STATUS_OK) failed |= 0x01;
    if (!failed) {
        topology.media_registry = registry;
        if (core_machine_configure_dma(machine, &dma, &fdc_binding) != TYPE_STATUS_OK ||
            core_machine_configure_hdc(machine, &topology) != TYPE_STATUS_OK) {
            failed |= 0x02;
        } else if (!core_machine_port_has_read(&machine->executor_port, 0x0320u) ||
            !core_machine_port_has_write(&machine->executor_port, 0x0320u) ||
            !core_machine_port_has_read(&machine->executor_port, 0x0321u) ||
            !core_machine_port_has_write(&machine->executor_port, 0x0321u) ||
            !core_machine_port_has_read(&machine->executor_port, 0x0322u) ||
            !core_machine_port_has_write(&machine->executor_port, 0x0322u) ||
            core_machine_port_has_read(&machine->executor_port, 0x0323u) ||
            !core_machine_port_has_write(&machine->executor_port, 0x0323u) ||
            core_machine_port_has_read(&machine->executor_port, 0x01f0u) ||
            core_machine_port_has_write(&machine->executor_port, 0x01f7u)) {
            failed |= 0x04;
        } else if (
            machine->hdc.connect.config.protocol != CORE_MACHINE_HDC_PROTOCOL_XEBEC_XT ||
            machine->hdc.connect.irq_source.irq != 5u) {
            failed |= 0x08;
        } else if (
            machine->hdc_dma_request.core_token == 0u ||
            machine->hdc_dma_request.channel != 3u) {
            failed |= 0x10;
        } else if (core_machine_port_execute_read(&machine->executor_port, 0x0320u) !=
                TYPE_STATUS_UNSUPPORTED) {
            failed |= 0x20;
        }
    }
    core_machine_destroy(machine);
    core_machine_media_registry_destroy(registry);
    if (failed) {
        STD_FPRINTF(STD_STDERR, "M5:T484:S13:XEBEC-WIRING:FAIL bits=%x\n", failed);
        return 1;
    }
    puts("M5:T484:S13:XEBEC-TAGGED-PLAN:OK");
    puts("M5:T484:S13:XEBEC-DMA3-IRQ5:OK");
    puts("M5:T484:S13:HDC-NO-ATA-ALIAS:OK");
    return 0;
}
