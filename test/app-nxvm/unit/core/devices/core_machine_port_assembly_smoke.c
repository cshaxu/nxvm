#include "lib/types/types_interface.h"
#include <stdio.h>

#include "app-nxvm/devices/machine.h"
#include "app-nxvm/devices/media_interface.h"

typedef struct port_assembly_probe_state {
    lib_u32 value;
} port_assembly_probe_state;

static lib_status port_assembly_read(void *owner, lib_u16 port,
    lib_u32 *out_value)
{
    port_assembly_probe_state *state = (port_assembly_probe_state *)owner;

    (void)port;
    if (state == LIB_NULL || out_value == LIB_NULL) return LIB_STATUS_INVALID_ARGUMENT;
    *out_value = state->value;
    return LIB_STATUS_OK;
}

static lib_status port_assembly_write(void *owner, lib_u16 port,
    lib_u32 value)
{
    port_assembly_probe_state *state = (port_assembly_probe_state *)owner;

    (void)port;
    if (state == LIB_NULL) return LIB_STATUS_INVALID_ARGUMENT;
    state->value = value;
    return LIB_STATUS_OK;
}

static lib_i32 port_assembly_fresh_default_create(void)
{
    core_machine_config config = {0};
    core_machine *machine = LIB_NULL;
    lib_i32 failed = core_machine_create(&config, &machine) != LIB_STATUS_OK;

    core_machine_destroy(machine);
    return failed;
}

static lib_i32 port_assembly_range_transaction(void)
{
    const core_machine_config config = { .memory_bytes = CORE_MACHINE_MINIMUM_MEMORY_BYTES };
    const core_machine_port_provider provider = { port_assembly_read, port_assembly_write };
    core_machine_port_test_allocation allocation = { 3u, 0u };
    port_assembly_probe_state state = {0u};
    core_machine *machine = LIB_NULL;
    lib_u32 value = 0u;
    lib_i32 failed = core_machine_create(&config, &machine) != LIB_STATUS_OK;

    if (!failed) {
        core_machine_port_set_test_allocation(&machine->executor_port, &allocation);
        failed |= core_machine_install_port_provider(machine, 0x00e0u, 0x00e1u,
            &provider, &state) != LIB_STATUS_NO_MEMORY;
        failed |= core_machine_port_has_read(&machine->executor_port, 0x00e0u) ||
            core_machine_port_has_write(&machine->executor_port, 0x00e0u) ||
            core_machine_port_has_read(&machine->executor_port, 0x00e1u) ||
            core_machine_port_has_write(&machine->executor_port, 0x00e1u);
        allocation.fail_at = 0u;
        allocation.attempts = 0u;
        failed |= core_machine_install_port_provider(machine, 0x00e0u, 0x00e1u,
            &provider, &state) != LIB_STATUS_OK;
        failed |= core_machine_install_port_provider(machine, 0x00e0u, 0x00e1u,
            &provider, &state) != LIB_STATUS_INVALID_STATE;
        failed |= core_machine_freeze_execution_providers(machine) != LIB_STATUS_OK ||
            core_machine_reset(machine) != LIB_STATUS_OK ||
            core_machine_bus_write(machine, 0x00e0u, 0x5au) != LIB_STATUS_OK ||
            core_machine_bus_read(machine, 0x00e0u, &value) != LIB_STATUS_OK ||
            value != 0x5au;
    }
    core_machine_destroy(machine);
    return failed || port_assembly_fresh_default_create();
}

static lib_i32 port_assembly_create_failure(void)
{
    const core_machine_config config = { .memory_bytes = CORE_MACHINE_MINIMUM_MEMORY_BYTES };
    core_machine_port_test_allocation allocation = { 2u, 0u };
    core_machine *machine = (core_machine *)(lib_uptr)1u;
    lib_status status = core_machine_create_with_test_port_allocation(&config,
        &machine, &allocation);

    if (status != LIB_STATUS_NO_MEMORY || machine != LIB_NULL ||
        allocation.attempts != 2u) {
        core_machine_destroy(machine);
        return 1;
    }
    return port_assembly_fresh_default_create();
}

static lib_i32 port_assembly_fdc_transaction(void)
{
    const core_machine_config config = { .memory_bytes = CORE_MACHINE_MINIMUM_MEMORY_BYTES };
    const core_machine_dma_wiring wiring = { .fdc_channel = 2u,
        .controller_count = CORE_MACHINE_DMA_CONTROLLER_COUNT,
        .cascade_channel = CORE_MACHINE_DMA_CASCADE_CHANNEL };
    core_machine_media_registry *media = LIB_NULL;
    core_machine_dma_request_binding request = {0};
    core_machine_fdc_topology topology = {
        .media_registry = LIB_NULL,
        .drives = {{1u, CORE_MACHINE_MEDIA_ID_INVALID, CORE_MACHINE_MEDIA_ID_INVALID,
            CORE_MACHINE_MEDIA_ID_INVALID}},
        .config = {.dor_port = 0x03f2u, .status_port = 0x03f4u,
            .data_port = 0x03f5u, .direction_port = 0x03f7u,
            .control_port = 0x03f7u, .irq = 6u, .dma_channel = 2u}
    };
    core_machine_port_test_allocation allocation = { 2u, 0u };
    core_machine *machine = LIB_NULL;
    core_machine_fdc fdc_zero = {0};
    core_machine_fdc_topology topology_zero = {0};
    lib_i32 failed = 0;

    failed |= core_machine_media_registry_create(&media) != LIB_STATUS_OK ||
        core_machine_create(&config, &machine) != LIB_STATUS_OK ||
        core_machine_configure_dma(machine, &wiring, &request) != LIB_STATUS_OK;
    topology.media_registry = media;
    topology.dma_request = request;
    if (!failed) {
        core_machine_port_set_test_allocation(&machine->executor_port, &allocation);
        failed |= core_machine_configure_fdc(machine, &topology) != LIB_STATUS_NO_MEMORY ||
            machine->fdc_configured ||
            lib_memory_compare(&machine->fdc, &fdc_zero, sizeof(fdc_zero)) != 0 ||
            lib_memory_compare(&machine->fdc_topology, &topology_zero,
                sizeof(topology_zero)) != 0 ||
            core_machine_port_has_read(&machine->executor_port, 0x03f4u) ||
            core_machine_port_has_read(&machine->executor_port, 0x03f5u) ||
            core_machine_port_has_write(&machine->executor_port, 0x03f2u) ||
            core_machine_port_has_write(&machine->executor_port, 0x03f5u);
        allocation.fail_at = 0u;
        allocation.attempts = 0u;
        failed |= core_machine_configure_fdc(machine, &topology) != LIB_STATUS_OK ||
            !machine->fdc_configured;
    }
    core_machine_destroy(machine);
    core_machine_media_registry_destroy(media);
    return failed || port_assembly_fresh_default_create();
}

static lib_i32 port_assembly_rtc_transaction(lib_size fail_at)
{
    const core_machine_config machine_config = {
        .memory_bytes = CORE_MACHINE_MINIMUM_MEMORY_BYTES
    };
    const core_machine_rtc_cmos_config rtc_config = {
        .index_port = 0x0070u, .data_port = 0x0071u, .nmi_mask_bit = 0x80u,
        .irq = 8u, .ticks_per_second = 1000u, .default_count = 1u,
        .defaults = {{CORE_MACHINE_RTC_EQUIPMENT, 0x2fu}}
    };
    core_machine_port_test_allocation allocation = { fail_at, 0u };
    core_machine_rtc rtc_zero = {0};
    core_machine_rtc_cmos_config config_zero = {0};
    core_machine *machine = LIB_NULL;
    lib_i32 failed = core_machine_create(&machine_config, &machine) != LIB_STATUS_OK;

    if (!failed) {
        core_machine_port_set_test_allocation(&machine->executor_port, &allocation);
        failed |= core_machine_configure_rtc_cmos(machine, &rtc_config) !=
                LIB_STATUS_NO_MEMORY || machine->rtc_cmos_configured ||
            lib_memory_compare(&machine->shared_rtc, &rtc_zero, sizeof(rtc_zero)) != 0 ||
            lib_memory_compare(&machine->rtc_cmos_config, &config_zero,
                sizeof(config_zero)) != 0 ||
            core_machine_port_has_write(&machine->executor_port, 0x0070u) ||
            core_machine_port_has_read(&machine->executor_port, 0x0071u) ||
            core_machine_port_has_write(&machine->executor_port, 0x0071u);
        allocation.fail_at = 0u;
        allocation.attempts = 0u;
        failed |= core_machine_configure_rtc_cmos(machine, &rtc_config) !=
            LIB_STATUS_OK || !machine->rtc_cmos_configured;
    }
    core_machine_destroy(machine);
    return failed || port_assembly_fresh_default_create();
}

static lib_i32 port_assembly_hdc_transaction(void)
{
    const core_machine_config machine_config = {
        .memory_bytes = CORE_MACHINE_MINIMUM_MEMORY_BYTES
    };
    core_machine_media_registry *media = LIB_NULL;
    core_machine_hdc_topology topology = {
        .media_registry = LIB_NULL, .media_id = 1u,
        .config = {.protocol = CORE_MACHINE_HDC_PROTOCOL_ATA_PIO, .irq = 14u,
            .bus.task_file = {
                .data_port = 0x01f0u, .error_features_port = 0x01f1u,
                .sector_count_port = 0x01f2u, .sector_number_port = 0x01f3u,
                .cylinder_low_port = 0x01f4u, .cylinder_high_port = 0x01f5u,
                .drive_head_port = 0x01f6u, .status_command_port = 0x01f7u,
                .alternate_status_device_control_port = 0x03f6u,
                .lba28_supported = LIB_TRUE}}
    };
    core_machine_port_test_allocation allocation = {17u, 0u};
    core_machine_hdc hdc_zero = {0};
    core_machine_hdc_topology topology_zero = {0};
    core_machine *machine = LIB_NULL;
    lib_i32 failed = 0;

    failed |= core_machine_media_registry_create(&media) != LIB_STATUS_OK ||
        core_machine_create(&machine_config, &machine) != LIB_STATUS_OK;
    topology.media_registry = media;
    if (!failed) {
        core_machine_port_set_test_allocation(&machine->executor_port, &allocation);
        failed |= core_machine_configure_hdc(machine, &topology) != LIB_STATUS_NO_MEMORY ||
            machine->hdc_configured ||
            lib_memory_compare(&machine->hdc, &hdc_zero, sizeof(hdc_zero)) != 0 ||
            lib_memory_compare(&machine->hdc_topology, &topology_zero,
                sizeof(topology_zero)) != 0 ||
            core_machine_port_has_read(&machine->executor_port, 0x01f0u) ||
            core_machine_port_has_write(&machine->executor_port, 0x01f0u) ||
            core_machine_port_has_read(&machine->executor_port, 0x03f6u);
        allocation.fail_at = 0u;
        allocation.attempts = 0u;
        failed |= core_machine_configure_hdc(machine, &topology) != LIB_STATUS_OK ||
            !machine->hdc_configured;
    }
    core_machine_destroy(machine);
    core_machine_media_registry_destroy(media);
    return failed || port_assembly_fresh_default_create();
}

lib_i32 main(void)
{
    lib_i32 failed = port_assembly_range_transaction() ||
        port_assembly_create_failure() || port_assembly_fdc_transaction();

    if (failed) return 1;
    puts("M5:T313:S3:PORT-ASSEMBLY:OK");

    failed = port_assembly_rtc_transaction(1u) || port_assembly_rtc_transaction(2u) ||
        port_assembly_hdc_transaction();

    if (failed) return 1;
    puts("M5:T313:S4:CONTROLLER-ROLLBACK:OK");
    return 0;
}
