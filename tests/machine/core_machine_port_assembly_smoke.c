#include "type.h"

#include "core/machine/machine.h"
#include "core/machine/media_interface.h"

typedef struct port_assembly_probe_state {
    type_unsigned_32 value;
} port_assembly_probe_state;

static type_status port_assembly_read(C_VOID *owner, type_unsigned_16 port,
    type_unsigned_32 *out_value)
{
    port_assembly_probe_state *state = (port_assembly_probe_state *)owner;

    (C_VOID)port;
    if (state == STD_NULL || out_value == STD_NULL) return TYPE_STATUS_INVALID_ARGUMENT;
    *out_value = state->value;
    return TYPE_STATUS_OK;
}

static type_status port_assembly_write(C_VOID *owner, type_unsigned_16 port,
    type_unsigned_32 value)
{
    port_assembly_probe_state *state = (port_assembly_probe_state *)owner;

    (C_VOID)port;
    if (state == STD_NULL) return TYPE_STATUS_INVALID_ARGUMENT;
    state->value = value;
    return TYPE_STATUS_OK;
}

static C_INT port_assembly_fresh_default_create(C_VOID)
{
    core_machine_config config = {0};
    core_machine *machine = STD_NULL;
    C_INT failed = core_machine_create(&config, &machine) != TYPE_STATUS_OK;

    core_machine_destroy(machine);
    return failed;
}

static C_INT port_assembly_range_transaction(C_VOID)
{
    const core_machine_config config = { .memory_bytes = CORE_MACHINE_MINIMUM_MEMORY_BYTES };
    const core_machine_port_provider provider = { port_assembly_read, port_assembly_write };
    core_machine_port_test_allocation allocation = { 3u, 0u };
    port_assembly_probe_state state = {0u};
    core_machine *machine = STD_NULL;
    type_unsigned_32 value = 0u;
    C_INT failed = core_machine_create(&config, &machine) != TYPE_STATUS_OK;

    if (!failed) {
        core_machine_port_set_test_allocation(&machine->executor_port, &allocation);
        failed |= core_machine_install_port_provider(machine, 0x00e0u, 0x00e1u,
            &provider, &state) != TYPE_STATUS_NO_MEMORY;
        failed |= core_machine_port_has_read(&machine->executor_port, 0x00e0u) ||
            core_machine_port_has_write(&machine->executor_port, 0x00e0u) ||
            core_machine_port_has_read(&machine->executor_port, 0x00e1u) ||
            core_machine_port_has_write(&machine->executor_port, 0x00e1u);
        allocation.fail_at = 0u;
        allocation.attempts = 0u;
        failed |= core_machine_install_port_provider(machine, 0x00e0u, 0x00e1u,
            &provider, &state) != TYPE_STATUS_OK;
        failed |= core_machine_install_port_provider(machine, 0x00e0u, 0x00e1u,
            &provider, &state) != TYPE_STATUS_INVALID_STATE;
        failed |= core_machine_freeze_execution_providers(machine) != TYPE_STATUS_OK ||
            core_machine_reset(machine) != TYPE_STATUS_OK ||
            core_machine_bus_write(machine, 0x00e0u, 0x5au) != TYPE_STATUS_OK ||
            core_machine_bus_read(machine, 0x00e0u, &value) != TYPE_STATUS_OK ||
            value != 0x5au;
    }
    core_machine_destroy(machine);
    return failed || port_assembly_fresh_default_create();
}

static C_INT port_assembly_create_failure(C_VOID)
{
    const core_machine_config config = { .memory_bytes = CORE_MACHINE_MINIMUM_MEMORY_BYTES };
    core_machine_port_test_allocation allocation = { 2u, 0u };
    core_machine *machine = (core_machine *)(type_virtual_address)1u;
    type_status status = core_machine_create_with_test_port_allocation(&config,
        &machine, &allocation);

    if (status != TYPE_STATUS_NO_MEMORY || machine != STD_NULL ||
        allocation.attempts != 2u) {
        core_machine_destroy(machine);
        return 1;
    }
    return port_assembly_fresh_default_create();
}

static C_INT port_assembly_fdc_transaction(C_VOID)
{
    const core_machine_config config = { .memory_bytes = CORE_MACHINE_MINIMUM_MEMORY_BYTES };
    const core_machine_dma_wiring wiring = { .fdc_channel = 2u,
        .controller_count = CORE_MACHINE_DMA_CONTROLLER_COUNT,
        .cascade_channel = CORE_MACHINE_DMA_CASCADE_CHANNEL };
    core_machine_media_registry *media = STD_NULL;
    core_machine_dma_request_binding request = {0};
    core_machine_fdc_topology topology = {
        .media_registry = STD_NULL,
        .drives = {{1u, CORE_MACHINE_MEDIA_ID_INVALID, CORE_MACHINE_MEDIA_ID_INVALID,
            CORE_MACHINE_MEDIA_ID_INVALID}},
        .config = {.dor_port = 0x03f2u, .status_port = 0x03f4u,
            .data_port = 0x03f5u, .direction_port = 0x03f7u,
            .control_port = 0x03f7u, .irq = 6u, .dma_channel = 2u}
    };
    core_machine_port_test_allocation allocation = { 2u, 0u };
    core_machine *machine = STD_NULL;
    core_machine_fdc fdc_zero = {0};
    core_machine_fdc_topology topology_zero = {0};
    C_INT failed = 0;

    failed |= core_machine_media_registry_create(&media) != TYPE_STATUS_OK ||
        core_machine_create(&config, &machine) != TYPE_STATUS_OK ||
        core_machine_configure_dma(machine, &wiring, &request) != TYPE_STATUS_OK;
    topology.media_registry = media;
    topology.dma_request = request;
    if (!failed) {
        core_machine_port_set_test_allocation(&machine->executor_port, &allocation);
        failed |= core_machine_configure_fdc(machine, &topology) != TYPE_STATUS_NO_MEMORY ||
            machine->fdc_configured ||
            STD_MEMCMP(&machine->fdc, &fdc_zero, sizeof(fdc_zero)) != 0 ||
            STD_MEMCMP(&machine->fdc_topology, &topology_zero,
                sizeof(topology_zero)) != 0 ||
            core_machine_port_has_read(&machine->executor_port, 0x03f4u) ||
            core_machine_port_has_read(&machine->executor_port, 0x03f5u) ||
            core_machine_port_has_write(&machine->executor_port, 0x03f2u) ||
            core_machine_port_has_write(&machine->executor_port, 0x03f5u);
        allocation.fail_at = 0u;
        allocation.attempts = 0u;
        failed |= core_machine_configure_fdc(machine, &topology) != TYPE_STATUS_OK ||
            !machine->fdc_configured;
    }
    core_machine_destroy(machine);
    core_machine_media_registry_destroy(media);
    return failed || port_assembly_fresh_default_create();
}

static C_INT port_assembly_rtc_transaction(STD_SIZE_T fail_at)
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
    core_machine *machine = STD_NULL;
    C_INT failed = core_machine_create(&machine_config, &machine) != TYPE_STATUS_OK;

    if (!failed) {
        core_machine_port_set_test_allocation(&machine->executor_port, &allocation);
        failed |= core_machine_configure_rtc_cmos(machine, &rtc_config) !=
                TYPE_STATUS_NO_MEMORY || machine->rtc_cmos_configured ||
            STD_MEMCMP(&machine->shared_rtc, &rtc_zero, sizeof(rtc_zero)) != 0 ||
            STD_MEMCMP(&machine->rtc_cmos_config, &config_zero,
                sizeof(config_zero)) != 0 ||
            core_machine_port_has_write(&machine->executor_port, 0x0070u) ||
            core_machine_port_has_read(&machine->executor_port, 0x0071u) ||
            core_machine_port_has_write(&machine->executor_port, 0x0071u);
        allocation.fail_at = 0u;
        allocation.attempts = 0u;
        failed |= core_machine_configure_rtc_cmos(machine, &rtc_config) !=
            TYPE_STATUS_OK || !machine->rtc_cmos_configured;
    }
    core_machine_destroy(machine);
    return failed || port_assembly_fresh_default_create();
}

static C_INT port_assembly_hdc_transaction(C_VOID)
{
    const core_machine_config machine_config = {
        .memory_bytes = CORE_MACHINE_MINIMUM_MEMORY_BYTES
    };
    core_machine_media_registry *media = STD_NULL;
    core_machine_hdc_topology topology = {
        .media_registry = STD_NULL, .media_id = 1u,
        .config = {.protocol = CORE_MACHINE_HDC_PROTOCOL_ATA_PIO, .irq = 14u,
            .bus.task_file = {
                .data_port = 0x01f0u, .error_features_port = 0x01f1u,
                .sector_count_port = 0x01f2u, .sector_number_port = 0x01f3u,
                .cylinder_low_port = 0x01f4u, .cylinder_high_port = 0x01f5u,
                .drive_head_port = 0x01f6u, .status_command_port = 0x01f7u,
                .alternate_status_device_control_port = 0x03f6u,
                .lba28_supported = TYPE_TRUE}}
    };
    core_machine_port_test_allocation allocation = {17u, 0u};
    core_machine_hdc hdc_zero = {0};
    core_machine_hdc_topology topology_zero = {0};
    core_machine *machine = STD_NULL;
    C_INT failed = 0;

    failed |= core_machine_media_registry_create(&media) != TYPE_STATUS_OK ||
        core_machine_create(&machine_config, &machine) != TYPE_STATUS_OK;
    topology.media_registry = media;
    if (!failed) {
        core_machine_port_set_test_allocation(&machine->executor_port, &allocation);
        failed |= core_machine_configure_hdc(machine, &topology) != TYPE_STATUS_NO_MEMORY ||
            machine->hdc_configured ||
            STD_MEMCMP(&machine->hdc, &hdc_zero, sizeof(hdc_zero)) != 0 ||
            STD_MEMCMP(&machine->hdc_topology, &topology_zero,
                sizeof(topology_zero)) != 0 ||
            core_machine_port_has_read(&machine->executor_port, 0x01f0u) ||
            core_machine_port_has_write(&machine->executor_port, 0x01f0u) ||
            core_machine_port_has_read(&machine->executor_port, 0x03f6u);
        allocation.fail_at = 0u;
        allocation.attempts = 0u;
        failed |= core_machine_configure_hdc(machine, &topology) != TYPE_STATUS_OK ||
            !machine->hdc_configured;
    }
    core_machine_destroy(machine);
    core_machine_media_registry_destroy(media);
    return failed || port_assembly_fresh_default_create();
}

C_INT main(C_VOID)
{
    C_INT failed = port_assembly_range_transaction() ||
        port_assembly_create_failure() || port_assembly_fdc_transaction();

    if (failed) return 1;
    puts("M5:T313:S3:PORT-ASSEMBLY:OK");

    failed = port_assembly_rtc_transaction(1u) || port_assembly_rtc_transaction(2u) ||
        port_assembly_hdc_transaction();

    if (failed) return 1;
    puts("M5:T313:S4:CONTROLLER-ROLLBACK:OK");
    return 0;
}
