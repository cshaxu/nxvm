#include "type.h"

#include "core/machine/machine.h"
#include "core/machine/media_interface.h"
#include "core/machine/port.h"

static C_VOID core_machine_controller_fdc_command(core_machine_fdc *fdc, t_port *port,
    const type_unsigned_8 *bytes, STD_SIZE_T count)
{
    STD_SIZE_T index;

    for (index = 0u; index < count; ++index) {
        core_machine_port_write(port, 0x03f5u, bytes[index]);
    }
    core_machine_fdc_advance(fdc);
}

static C_INT core_machine_controller_fdc_result(core_machine_fdc *fdc, t_port *port,
    type_unsigned_8 *result, STD_SIZE_T count)
{
    STD_SIZE_T index;

    core_machine_fdc_advance(fdc);
    for (index = 0u; index < count; ++index) {
        result[index] = (type_unsigned_8)core_machine_port_read(port, 0x03f5u);
    }
    return (core_machine_port_read(port, 0x03f4u) &
        (VFDC_MSR_CB | VFDC_MSR_DIO)) == 0u;
}

static C_INT core_machine_controller_hdc_program_chs(core_machine *machine,
    const core_machine_hdc_config *config)
{
    return core_machine_bus_write(machine, config->bus.task_file.sector_count_port, 1u) ==
            TYPE_STATUS_OK &&
        core_machine_bus_write(machine, config->bus.task_file.sector_number_port, 1u) ==
            TYPE_STATUS_OK &&
        core_machine_bus_write(machine, config->bus.task_file.cylinder_low_port, 0u) ==
            TYPE_STATUS_OK &&
        core_machine_bus_write(machine, config->bus.task_file.cylinder_high_port, 0u) ==
            TYPE_STATUS_OK &&
        core_machine_bus_write(machine, config->bus.task_file.drive_head_port, 0u) ==
            TYPE_STATUS_OK;
}

C_INT main(C_VOID)
{
    static const type_unsigned_8 specify_non_dma[] = {0x03u, 0xdfu, 0x03u};
    static const type_unsigned_8 read_absent[] = {
        0xe6u, 0x00u, 0x00u, 0x00u, 0x01u, 0x02u, 0x01u, 0x1bu, 0xffu
    };
    static const type_unsigned_8 write_absent[] = {
        0xc5u, 0x00u, 0x00u, 0x00u, 0x01u, 0x02u, 0x01u, 0x1bu, 0xffu
    };
    const core_machine_config config = {
        .memory_bytes = CORE_MACHINE_MINIMUM_MEMORY_BYTES,
        .cpu_profile = CORE_MACHINE_CPU_PROFILE_8086,
        .fpu_profile = CORE_MACHINE_FPU_PROFILE_NONE,
        .ticks_per_instruction = 1u
    };
    const core_machine_dma_wiring dma_wiring = { .fdc_channel = 2u,
        .controller_count = CORE_MACHINE_DMA_CONTROLLER_COUNT,
        .cascade_channel = CORE_MACHINE_DMA_CASCADE_CHANNEL };
    const core_machine_fdc_config fdc_config = {
        .dor_port = 0x03f2u, .status_port = 0x03f4u, .data_port = 0x03f5u,
        .direction_port = 0x03f7u, .control_port = 0x03f7u,
        .irq = 6u, .dma_channel = 2u
    };
    const core_machine_hdc_config hdc_config = {
        .protocol = CORE_MACHINE_HDC_PROTOCOL_ATA_PIO,
        .irq = 14u, .bus.task_file = {
            .data_port = 0x01f0u, .error_features_port = 0x01f1u,
            .sector_count_port = 0x01f2u, .sector_number_port = 0x01f3u,
            .cylinder_low_port = 0x01f4u, .cylinder_high_port = 0x01f5u,
            .drive_head_port = 0x01f6u, .status_command_port = 0x01f7u,
            .alternate_status_device_control_port = 0x03f6u,
            .lba28_supported = TYPE_TRUE}
    };
    core_machine_media_registry *media = STD_NULL;
    core_machine_dma_request_binding dma_request = {0};
    core_machine_fdc_topology fdc_topology = {
        .media_registry = STD_NULL,
        .drives = {{1u, CORE_MACHINE_MEDIA_ID_INVALID,
            CORE_MACHINE_MEDIA_ID_INVALID, CORE_MACHINE_MEDIA_ID_INVALID}},
        .config = {
            .dor_port = 0x03f2u, .status_port = 0x03f4u, .data_port = 0x03f5u,
            .direction_port = 0x03f7u, .control_port = 0x03f7u,
            .irq = 6u, .dma_channel = 2u
        }
    };
    core_machine_hdc_topology hdc_topology = {
        .media_registry = STD_NULL,
        .media_id = 2u,
        .config = {
            .protocol = CORE_MACHINE_HDC_PROTOCOL_ATA_PIO, .irq = 14u,
            .bus.task_file = {
                .data_port = 0x01f0u, .error_features_port = 0x01f1u,
                .sector_count_port = 0x01f2u, .sector_number_port = 0x01f3u,
                .cylinder_low_port = 0x01f4u, .cylinder_high_port = 0x01f5u,
                .drive_head_port = 0x01f6u, .status_command_port = 0x01f7u,
                .alternate_status_device_control_port = 0x03f6u,
                .lba28_supported = TYPE_TRUE}
        }
    };
    core_machine *machine = STD_NULL;
    t_port *port;
    type_unsigned_8 result[7] = {0};
    type_unsigned_32 status = 0u;
    type_unsigned_32 error = 0u;
    type_status fdc_before_dma = TYPE_STATUS_OK;
    type_status dma_status = TYPE_STATUS_OK;
    type_status fdc_status = TYPE_STATUS_OK;
    type_status hdc_status = TYPE_STATUS_OK;
    C_INT failed = 0;

    if (core_machine_media_registry_create(&media) != TYPE_STATUS_OK ||
        core_machine_create(&config, &machine) != TYPE_STATUS_OK) failed |= 0x01;
    if (!failed) {
        fdc_topology.media_registry = media;
        hdc_topology.media_registry = media;
        fdc_topology.dma_request = dma_request;
        fdc_before_dma = core_machine_configure_fdc(machine, &fdc_topology);
        dma_status = core_machine_configure_dma(machine, &dma_wiring, &dma_request);
        if (fdc_before_dma != TYPE_STATUS_INVALID_STATE ||
            dma_status != TYPE_STATUS_OK) {
            failed |= 0x02;
        }
        fdc_topology.dma_request = dma_request;
        fdc_status = core_machine_configure_fdc(machine, &fdc_topology);
        hdc_status = core_machine_configure_hdc(machine, &hdc_topology);
        if (fdc_status != TYPE_STATUS_OK ||
            core_machine_configure_fdc(machine, &fdc_topology) !=
                TYPE_STATUS_INVALID_STATE ||
            hdc_status != TYPE_STATUS_OK ||
            core_machine_configure_hdc(machine, &hdc_topology) !=
                TYPE_STATUS_INVALID_STATE ||
            core_machine_media_registry_freeze(media) != TYPE_STATUS_OK ||
            core_machine_freeze_execution_providers(machine) != TYPE_STATUS_OK ||
            core_machine_reset(machine) != TYPE_STATUS_OK) {
            failed |= 0x04;
        } else {
            port = &machine->executor_port;
            failed |= machine->fdc.connect.dma_request.core_token !=
                    dma_request.core_token ||
                machine->fdc.connect.irq_source.irq != fdc_config.irq ||
                machine->hdc.connect.irq_source.irq != hdc_config.irq ||
                machine->hdc.connect.media_id != hdc_topology.media_id;

            core_machine_port_write(port, fdc_config.dor_port, 0x1cu);
            core_machine_controller_fdc_command(&machine->fdc, port, specify_non_dma,
                sizeof(specify_non_dma));
            core_machine_controller_fdc_command(&machine->fdc, port, read_absent,
                sizeof(read_absent));
            failed |= !core_machine_controller_fdc_result(&machine->fdc, port, result,
                sizeof(result)) || result[0] != core_machine_fdc_ST0_ABNORMAL ||
                result[1] != 0x04u;

            core_machine_controller_fdc_command(&machine->fdc, port, write_absent,
                sizeof(write_absent));
            core_machine_port_write(port, fdc_config.data_port, 0x5au);
            failed |= !core_machine_controller_fdc_result(&machine->fdc, port, result,
                sizeof(result)) || result[0] != core_machine_fdc_ST0_ABNORMAL ||
                result[1] != 0x04u;

            if (!core_machine_controller_hdc_program_chs(machine, &hdc_config) ||
                core_machine_bus_write(machine, hdc_config.bus.task_file.status_command_port,
                    0x20u) != TYPE_STATUS_OK ||
                core_machine_bus_read(machine, hdc_config.bus.task_file.status_command_port,
                    &status) != TYPE_STATUS_OK ||
                status != CORE_MACHINE_HDC_STATUS_BSY) {
                failed |= 0x08;
            } else {
                core_machine_hdc_advance(&machine->hdc);
                if (core_machine_bus_read(machine, hdc_config.bus.task_file.status_command_port,
                        &status) != TYPE_STATUS_OK ||
                core_machine_bus_read(machine, hdc_config.bus.task_file.error_features_port,
                    &error) != TYPE_STATUS_OK ||
                status != (CORE_MACHINE_HDC_STATUS_DRDY | CORE_MACHINE_HDC_STATUS_ERR) ||
                error != CORE_MACHINE_HDC_ERROR_ABORT) {
                    failed |= 0x08;
                }
            }
            if (core_machine_reset(machine) != TYPE_STATUS_OK ||
                machine->fdc.data.phase != core_machine_fdc_PHASE_COMMAND ||
                machine->hdc.data.status != (CORE_MACHINE_HDC_STATUS_DRDY |
                    CORE_MACHINE_HDC_STATUS_DSC)) {
                failed |= 0x10;
            }
        }
    }
    core_machine_destroy(machine);
    core_machine_media_registry_destroy(media);
    if (failed) {
        STD_FPRINTF(STD_STDERR,
            "M5:T296:S4:CONTROLLER-AUTHORITY:FAIL bits=%x status=%02x error=%02x fdc0=%d dma=%d fdc=%d hdc=%d\n",
            failed, status, error, fdc_before_dma, dma_status, fdc_status, hdc_status);
        return 1;
    }
    puts("M5:T296:S4:CONTROLLER-AUTHORITY:OK");
    return 0;
}
