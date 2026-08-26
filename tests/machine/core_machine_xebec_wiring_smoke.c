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
                .dma_channel = 3u,
                .drive_type = CORE_MACHINE_XEBEC_DRIVE_TYPE_2,
                .expected_media_geometry = {
                    CORE_MACHINE_XEBEC_TYPE_2_LOGICAL_SECTOR_COUNT,
                    CORE_MACHINE_XEBEC_TYPE_2_BYTES_PER_SECTOR,
                    CORE_MACHINE_XEBEC_TYPE_2_CYLINDERS,
                    CORE_MACHINE_XEBEC_TYPE_2_HEADS,
                    CORE_MACHINE_XEBEC_TYPE_2_SECTORS_PER_TRACK}
            }
        }
    };
    core_machine_media_registry *registry = STD_NULL;
    core_machine_dma_request_binding fdc_binding = {0};
    core_machine *machine = STD_NULL;
    core_machine_hdc_topology topology = hdc;
    const type_unsigned_8 dcb[] = {0x00u, 0x20u, 0x01u, 0x23u, 0u, 0u};
    const type_unsigned_8 initialize_dcb[] = {0x0cu, 0u, 0u, 0u, 0u, 0u};
    const type_unsigned_8 response[] = {0x22u};
    const type_unsigned_8 sense_dcb[] = {0x03u, 0x20u, 0u, 0u, 0u, 0u};
    const type_unsigned_8 sense[] = {0x04u, 0x20u, 0x01u, 0x23u};
    const type_unsigned_8 invalid_dcb[] = {0x02u, 0u, 0u, 0u, 0u, 0u};
    STD_SIZE_T index;
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
        } else {
            core_machine_port_write(&machine->executor_port, 0x0320u, dcb[0]);
            if (machine->hdc.xebec.dcb_count != 0u) failed |= 0x20;
            core_machine_port_write(&machine->executor_port, 0x0322u, 0u);
            for (index = 0u; index < sizeof(dcb); ++index)
                core_machine_port_write(&machine->executor_port, 0x0320u, dcb[index]);
            for (index = 0u; index < sizeof(response); ++index) {
                if (core_machine_port_read(&machine->executor_port, 0x0320u) != response[index]) {
                    failed |= 0x80;
                    break;
                }
            }
            if (!failed) {
                core_machine_port_write(&machine->executor_port, 0x0322u, 0u);
                for (index = 0u; index < sizeof(sense_dcb); ++index)
                    core_machine_port_write(&machine->executor_port, 0x0320u, sense_dcb[index]);
                for (index = 0u; index < sizeof(sense); ++index) {
                    if (core_machine_port_read(&machine->executor_port, 0x0320u) != sense[index]) {
                        failed |= 0x800;
                        break;
                    }
                }
            }
            if (!failed) {
                core_machine_port_write(&machine->executor_port, 0x0322u, 0u);
                for (index = 0u; index < sizeof(invalid_dcb); ++index)
                    core_machine_port_write(&machine->executor_port, 0x0320u, invalid_dcb[index]);
                if (core_machine_port_read(&machine->executor_port, 0x0320u) != 0x02u)
                    failed |= 0x1000;
            }
            if (!failed) {
                core_machine_port_write(&machine->executor_port, 0x0323u, 0x5au);
                if (machine->hdc.xebec.mask_pattern != 0x5au) failed |= 0x100;
            }
            if (!failed) {
                core_machine_port_write(&machine->executor_port, 0x0322u, 0u);
                for (index = 0u; index < sizeof(initialize_dcb); ++index)
                    core_machine_port_write(&machine->executor_port, 0x0320u, initialize_dcb[index]);
                if (machine->hdc.xebec.phase != CORE_MACHINE_XEBEC_PHASE_INITIALIZE) failed |= 0x200;
                for (index = 0u; index < sizeof(machine->hdc.xebec.initialize); ++index)
                    core_machine_port_write(&machine->executor_port, 0x0320u, 0u);
                if (machine->hdc.xebec.phase != CORE_MACHINE_XEBEC_PHASE_RESPONSE) failed |= 0x400;
            }
            if (!failed) {
                core_machine_port_write(&machine->executor_port, 0x0321u, 0u);
                if (machine->hdc.xebec.dcb_count != 0u ||
                    machine->hdc.xebec.mask_pattern != 0u ||
                    machine->hdc.xebec.phase != CORE_MACHINE_XEBEC_PHASE_IDLE) failed |= 0x40;
            }
        }
    }
    core_machine_destroy(machine);
    core_machine_media_registry_destroy(registry);
    if (failed) {
        STD_FPRINTF(STD_STDERR, "M5:T484:S15:XEBEC-STACK:FAIL bits=%x\n", failed);
        return 1;
    }
    puts("M5:T484:S15:XEBEC-STACK:OK");
    puts("M5:T484:S15:XEBEC-NO-ATA-ALIAS:OK");
    return 0;
}
