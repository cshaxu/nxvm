#include "../../../support/hdc.h"
#include "app-nxvm/devices/machine_board_interface.h"
#include "support/fdc_fixture.h"
#include "lib/types/types_interface.h"
#include <stdio.h>

#include "app-nxvm/devices/machine.h"
#include "app-nxvm/devices/machine_board_state.h"
#include "app-nxvm/devices/media_interface.h"
#include "app-nxvm/devices/port.h"

typedef struct board_phase_probe {
    core_machine *machine;
    lib_u32 calls[4];
    lib_u32 reset_phase;
    lib_bool failed;
} board_phase_probe;

static void board_phase_reset_devices(void *owner)
{
    board_phase_probe *probe = owner;
    core_machine *machine = probe->machine;
    ++probe->calls[0];
    probe->failed |= probe->reset_phase != 0u ||
        machine->elapsed_ticks != 17u;
    probe->reset_phase = 1u;
}

static void board_phase_reset_clocks(void *owner)
{
    board_phase_probe *probe = owner;
    core_machine *machine = probe->machine;
    ++probe->calls[1];
    probe->failed |= probe->reset_phase != 1u ||
        machine->elapsed_ticks != 0u;
    probe->reset_phase = 2u;
}

static void board_phase_refresh_nmi(void *owner)
{
    board_phase_probe *probe = owner;
    core_machine *machine = probe->machine;
    ++probe->calls[2];
    probe->failed |= core_machine_cpu_nmi_is_masked(machine->executor_cpu_execution);
}

static void board_phase_finalize_devices(void *owner)
{
    board_phase_probe *probe = owner;
    core_machine *machine = probe->machine;
    ++probe->calls[3];
    probe->failed |= machine->executor_cpu_execution == LIB_NULL ||
        machine->firmware_provider != LIB_NULL;
}

static lib_i32 verify_board_phases(const core_machine_config *config)
{
    const core_machine_executor_config executor = {
        .memory_bytes = config->memory_bytes,
        .cpu_profile = config->cpu_profile,
        .ticks_per_instruction = config->ticks_per_instruction
    };
    core_machine *machine = LIB_NULL;
    core_machine_board_state *board = LIB_NULL;
    board_phase_probe probe = {0};
    board_phase_probe rejected = {0};
    core_machine_attachment binding = {
        .reset_devices = board_phase_reset_devices,
        .reset_clocks = board_phase_reset_clocks,
        .refresh_nmi = board_phase_refresh_nmi,
        .finalize_devices = board_phase_finalize_devices,
        .context = &probe
    };

    core_machine_board_refresh_nmi(LIB_NULL);
    if (core_machine_create(config, &machine, &board) != LIB_STATUS_OK) return 1;
    probe.failed = machine->attachment.context != board ||
        machine->attachment.context == machine || board->core != machine ||
        machine->attachment.reset_devices !=
            core_machine_board_reset_devices ||
        machine->attachment.reset_clocks != core_machine_board_reset_clocks ||
        machine->attachment.refresh_nmi != core_machine_board_refresh_nmi ||
        machine->attachment.finalize_devices != core_machine_board_finalize_devices;
    core_machine_destroy(machine);
    if (core_machine_neutral_create(&executor, &machine) !=
            LIB_STATUS_OK) return 1;
    if (core_machine_freeze_execution_providers(machine) != LIB_STATUS_OK ||
        core_machine_bind_attachment(machine, &binding) != LIB_STATUS_INVALID_STATE ||
        machine->attachment.context != LIB_NULL) probe.failed = LIB_TRUE;
    core_machine_destroy(machine);
    if (core_machine_neutral_create(&executor, &machine) !=
            LIB_STATUS_OK) return 1;
    probe.machine = machine;
    if (core_machine_bind_attachment(machine, LIB_NULL) != LIB_STATUS_INVALID_ARGUMENT ||
        core_machine_bind_attachment(LIB_NULL, &binding) != LIB_STATUS_INVALID_ARGUMENT)
        probe.failed = LIB_TRUE;
    binding.context = LIB_NULL;
    if (core_machine_bind_attachment(machine, &binding) != LIB_STATUS_INVALID_ARGUMENT ||
        machine->attachment.context != LIB_NULL) probe.failed = LIB_TRUE;
    binding.context = &probe;
    binding.finalize_devices = LIB_NULL;
    if (core_machine_bind_attachment(machine, &binding) != LIB_STATUS_INVALID_ARGUMENT ||
        machine->attachment.context != LIB_NULL) probe.failed = LIB_TRUE;
    binding.finalize_devices = board_phase_finalize_devices;
    if (core_machine_bind_attachment(machine, &binding) != LIB_STATUS_OK)
        probe.failed = LIB_TRUE;
    /* Core owns the copy, not the caller's binding storage. */
    binding.context = &rejected;
    binding.reset_devices = LIB_NULL;
    if (core_machine_bind_attachment(machine, &binding) != LIB_STATUS_INVALID_STATE ||
        machine->attachment.context != &probe ||
        machine->attachment.reset_devices != board_phase_reset_devices)
        probe.failed = LIB_TRUE;
    machine->elapsed_ticks = 17u;
    if (core_machine_freeze_execution_providers(machine) != LIB_STATUS_OK ||
        core_machine_bind_attachment(machine, &binding) != LIB_STATUS_INVALID_STATE ||
        core_machine_reset(machine) != LIB_STATUS_OK ||
        core_machine_set_nmi_mask(machine, 1) != LIB_STATUS_OK ||
        probe.calls[2] != 0u ||
        core_machine_set_nmi_mask(machine, 0) != LIB_STATUS_OK) probe.failed = LIB_TRUE;
    core_machine_destroy(machine);
    return probe.failed || probe.reset_phase != 2u || probe.calls[0] != 1u ||
        probe.calls[1] != 1u || probe.calls[2] != 1u || probe.calls[3] != 1u ||
        rejected.calls[0] != 0u || rejected.calls[3] != 0u;
}

static lib_i32 verify_partial_board_cleanup(void)
{
    const core_machine_executor_config executor = {
        .memory_bytes = CORE_MACHINE_MINIMUM_MEMORY_BYTES,
        .cpu_profile = CORE_MACHINE_CPU_PROFILE_8086,
        .ticks_per_instruction = 1u
    };
    const core_machine_config invalid_board = {
        .clock_plan.pit = {1u, 0u}
    };
    core_machine *machine = LIB_NULL;
    core_machine_board_state *board = LIB_NULL;

    if (core_machine_neutral_create(&executor, &machine) !=
            LIB_STATUS_OK) return 1;
    /* Board construction failure owns and destroys the unpublished Core. */
    return core_machine_board_create(machine, &invalid_board, &board) !=
        LIB_STATUS_INVALID_ARGUMENT || board != LIB_NULL;
}

static void core_machine_controller_fdc_command(core_machine_fdc *fdc, t_port *port,
    const lib_u8 *bytes, lib_size count)
{
    lib_size index;

    for (index = 0u; index < count; ++index) {
        core_machine_port_write(port, 0x03f5u, bytes[index]);
    }
    test_fdc_advance(fdc);
}

static lib_i32 core_machine_controller_fdc_result(core_machine_fdc *fdc, t_port *port,
    lib_u8 *result, lib_size count)
{
    lib_size index;

    test_fdc_advance(fdc);
    for (index = 0u; index < count; ++index) {
        result[index] = (lib_u8)core_machine_port_read(port, 0x03f5u);
    }
    return (core_machine_port_read(port, 0x03f4u) &
        (TEST_FDC_MSR_CB | TEST_FDC_MSR_DIO)) == 0u;
}

static lib_i32 core_machine_controller_hdc_program_chs(core_machine *machine,
    const core_machine_hdc_config *config)
{
    return core_machine_bus_write(machine, config->bus.task_file.sector_count_port, 1u) ==
            LIB_STATUS_OK &&
        core_machine_bus_write(machine, config->bus.task_file.sector_number_port, 1u) ==
            LIB_STATUS_OK &&
        core_machine_bus_write(machine, config->bus.task_file.cylinder_low_port, 0u) ==
            LIB_STATUS_OK &&
        core_machine_bus_write(machine, config->bus.task_file.cylinder_high_port, 0u) ==
            LIB_STATUS_OK &&
        core_machine_bus_write(machine, config->bus.task_file.drive_head_port, 0u) ==
            LIB_STATUS_OK;
}

lib_i32 main(void)
{
    static const lib_u8 specify_non_dma[] = {0x03u, 0xdfu, 0x03u};
    static const lib_u8 read_absent[] = {
        0xe6u, 0x00u, 0x00u, 0x00u, 0x01u, 0x02u, 0x01u, 0x1bu, 0xffu
    };
    static const lib_u8 write_absent[] = {
        0xc5u, 0x00u, 0x00u, 0x00u, 0x01u, 0x02u, 0x01u, 0x1bu, 0xffu
    };
    const core_machine_config config = {
        .memory_bytes = CORE_MACHINE_MINIMUM_MEMORY_BYTES,
        .cpu_profile = CORE_MACHINE_CPU_PROFILE_8086,
        .fpu_profile = X86_FPU_PROFILE_NONE,
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
            .lba28_supported = LIB_TRUE}
    };
    core_machine_media_registry *media = LIB_NULL;
    core_machine_dma_request_binding dma_request = {0};
    core_machine_fdc_topology fdc_topology = {
        .media_registry = LIB_NULL,
        .drives = {{1u, CORE_MACHINE_MEDIA_ID_INVALID,
            CORE_MACHINE_MEDIA_ID_INVALID, CORE_MACHINE_MEDIA_ID_INVALID}},
        .config = {
            .dor_port = 0x03f2u, .status_port = 0x03f4u, .data_port = 0x03f5u,
            .direction_port = 0x03f7u, .control_port = 0x03f7u,
            .irq = 6u, .dma_channel = 2u
        }
    };
    core_machine_hdc_topology hdc_topology = {
        .media_registry = LIB_NULL,
        .media_id = 2u,
        .config = {
            .protocol = CORE_MACHINE_HDC_PROTOCOL_ATA_PIO, .irq = 14u,
            .bus.task_file = {
                .data_port = 0x01f0u, .error_features_port = 0x01f1u,
                .sector_count_port = 0x01f2u, .sector_number_port = 0x01f3u,
                .cylinder_low_port = 0x01f4u, .cylinder_high_port = 0x01f5u,
                .drive_head_port = 0x01f6u, .status_command_port = 0x01f7u,
                .alternate_status_device_control_port = 0x03f6u,
                .lba28_supported = LIB_TRUE}
        }
    };
    core_machine *machine = LIB_NULL;
    core_machine_board_state *board = LIB_NULL;
    t_port *port;
    lib_u8 result[7] = {0};
    lib_u32 status = 0u;
    lib_u32 error = 0u;
    lib_status fdc_before_dma = LIB_STATUS_OK;
    lib_status dma_status = LIB_STATUS_OK;
    lib_status fdc_status = LIB_STATUS_OK;
    lib_status hdc_status = LIB_STATUS_OK;
    lib_i32 failed = verify_board_phases(&config) || verify_partial_board_cleanup();

    if (core_machine_media_registry_create(&media) != LIB_STATUS_OK ||
        core_machine_create(&config, &machine, &board) != LIB_STATUS_OK) failed |= 0x01;
    if (!failed) {
        fdc_topology.media_registry = media;
        hdc_topology.media_registry = media;
        fdc_topology.dma_request = dma_request;
        fdc_before_dma = core_machine_configure_fdc(board, &fdc_topology);
        dma_status = core_machine_configure_dma(board, &dma_wiring, &dma_request);
        if (fdc_before_dma != LIB_STATUS_INVALID_STATE ||
            dma_status != LIB_STATUS_OK) {
            failed |= 0x02;
        }
        fdc_topology.dma_request = dma_request;
        fdc_status = core_machine_configure_fdc(board, &fdc_topology);
        hdc_status = core_machine_configure_hdc(board, &hdc_topology);
        if (fdc_status != LIB_STATUS_OK ||
            core_machine_configure_fdc(board, &fdc_topology) !=
                LIB_STATUS_INVALID_STATE ||
            hdc_status != LIB_STATUS_OK ||
            core_machine_configure_hdc(board, &hdc_topology) !=
                LIB_STATUS_INVALID_STATE ||
            core_machine_media_registry_freeze(media) != LIB_STATUS_OK ||
            core_machine_freeze_execution_providers(machine) != LIB_STATUS_OK ||
            core_machine_reset(machine) != LIB_STATUS_OK) {
            failed |= 0x04;
        } else {
            port = &machine->executor_port;
            failed |= board->fdc.connect.dma_request.core_token !=
                    dma_request.core_token ||
                board->fdc.connect.dma_request_owner != board ||
                board->fdc.connect.machine != machine ||
                board->fdc.connect.irq_source.irq != fdc_config.irq ||
                board->hdc.connect.irq_source.irq != hdc_config.irq ||
                board->hdc.connect.media_id != hdc_topology.media_id;

            core_machine_port_write(port, fdc_config.dor_port, 0x1cu);
            core_machine_controller_fdc_command(&board->fdc, port, specify_non_dma,
                sizeof(specify_non_dma));
            core_machine_controller_fdc_command(&board->fdc, port, read_absent,
                sizeof(read_absent));
            failed |= !core_machine_controller_fdc_result(&board->fdc, port, result,
                sizeof(result)) || result[0] != 0x48u || result[1] != 0u;

            core_machine_controller_fdc_command(&board->fdc, port, write_absent,
                sizeof(write_absent));
            core_machine_port_write(port, fdc_config.data_port, 0x5au);
            failed |= !core_machine_controller_fdc_result(&board->fdc, port, result,
                sizeof(result)) || result[0] != 0x48u || result[1] != 0u;

            if (!core_machine_controller_hdc_program_chs(machine, &hdc_config) ||
                core_machine_bus_write(machine, hdc_config.bus.task_file.status_command_port,
                    0x20u) != LIB_STATUS_OK ||
                core_machine_bus_read(machine, hdc_config.bus.task_file.status_command_port,
                    &status) != LIB_STATUS_OK ||
                status != X86_HDC_STATUS_BSY) {
                failed |= 0x08;
            } else {
                hdc_service(&board->hdc);
                if (core_machine_bus_read(machine, hdc_config.bus.task_file.status_command_port,
                        &status) != LIB_STATUS_OK ||
                core_machine_bus_read(machine, hdc_config.bus.task_file.error_features_port,
                    &error) != LIB_STATUS_OK ||
                status != (X86_HDC_STATUS_DRDY | X86_HDC_STATUS_ERR) ||
                error != X86_HDC_ERROR_ABORT) {
                    failed |= 0x08;
                }
            }
            if (core_machine_reset(machine) != LIB_STATUS_OK ||
                core_machine_port_read(port, fdc_config.status_port) != TEST_FDC_MSR_RQM ||
                hdc_observe(&board->hdc).status != (X86_HDC_STATUS_DRDY |
                    X86_HDC_STATUS_DSC)) {
                failed |= 0x10;
            }
        }
    }
    core_machine_destroy(machine);
    core_machine_media_registry_destroy(media);
    if (failed) {
        fprintf(stderr,
            "M5:T296:S4:CONTROLLER-AUTHORITY:FAIL bits=%x status=%02x error=%02x fdc0=%d dma=%d fdc=%d hdc=%d\n",
            failed, status, error, fdc_before_dma, dma_status, fdc_status, hdc_status);
        return 1;
    }
    puts("M5:T296:S4:CONTROLLER-AUTHORITY:OK");
    return 0;
}
