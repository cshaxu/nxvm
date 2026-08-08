#include "type.h"

#include "core/machine/firmware_interface.h"
#include "core/machine/machine_interface.h"
#include "core/machine/rom_mapping_interface.h"
#include "../support/core_machine_executor_fixture.h"
#include "../support/core_machine_cpu_fixture.h"

typedef struct firmware_probe {
    core_machine *machine;
    core_machine_firmware_context *expired;
    C_INT configure_calls;
    C_INT reset_calls;
    C_INT after_run_calls;
    C_INT reentry_rejected;
} firmware_probe;

typedef struct firmware_failed_probe {
    core_machine_firmware_context *expired;
    C_INT configure_calls;
} firmware_failed_probe;

static type_status firmware_probe_configure(C_VOID *opaque,
    core_machine_firmware_context *firmware)
{
    firmware_probe *probe = (firmware_probe *)opaque;
    const uint8_t code = 0x90u;
    type_status status;

    if (probe == STD_NULL || firmware == STD_NULL) return TYPE_STATUS_FAULT;
    ++probe->configure_calls;
    probe->reentry_rejected =
        core_machine_reset(probe->machine) == TYPE_STATUS_INVALID_STATE &&
        core_machine_register_immutable_rom_mapping(probe->machine, 0xffff0u,
            &code, sizeof(code)) == TYPE_STATUS_INVALID_STATE;
    status = core_machine_firmware_register_immutable_rom(firmware, 0xe0000u,
        &code, sizeof(code));
    if (status != TYPE_STATUS_OK) return status;
    return core_machine_firmware_register_immutable_rom(firmware, 0xffff0u,
        &code, sizeof(code));
}

static type_status firmware_failed_probe_configure(C_VOID *opaque,
    core_machine_firmware_context *firmware)
{
    firmware_failed_probe *probe = (firmware_failed_probe *)opaque;
    const uint8_t code = 0x90u;

    if (probe == STD_NULL || firmware == STD_NULL ||
        core_machine_firmware_register_immutable_rom(firmware, 0xe0000u,
            &code, sizeof(code)) != TYPE_STATUS_OK) {
        return TYPE_STATUS_FAULT;
    }
    ++probe->configure_calls;
    probe->expired = firmware;
    return TYPE_STATUS_FAULT;
}

static type_status firmware_failed_probe_reset(C_VOID *opaque,
    core_machine_firmware_context *firmware)
{
    (C_VOID)opaque;
    (C_VOID)firmware;
    return TYPE_STATUS_FAULT;
}

static type_status firmware_probe_reset(C_VOID *opaque,
    core_machine_firmware_context *firmware)
{
    firmware_probe *probe = (firmware_probe *)opaque;
    uint8_t value = 0x5au;

    if (probe == STD_NULL) return TYPE_STATUS_FAULT;
    ++probe->reset_calls;
    probe->expired = firmware;
    if (core_machine_request_stop(probe->machine) != TYPE_STATUS_INVALID_STATE) {
        return TYPE_STATUS_FAULT;
    }
    return core_machine_firmware_memory_write(firmware, 0x500u, &value,
        sizeof(value));
}

static type_status firmware_probe_after_run(C_VOID *opaque,
    core_machine_firmware_context *firmware)
{
    firmware_probe *probe = (firmware_probe *)opaque;
    uint8_t value = 0u;

    if (probe == STD_NULL || core_machine_firmware_memory_read(firmware,
            0x500u, &value, sizeof(value)) != TYPE_STATUS_OK || value != 0x5au) {
        return TYPE_STATUS_FAULT;
    }
    ++probe->after_run_calls;
    probe->expired = firmware;
    return core_machine_firmware_port_write(firmware, 0x03d8u, 0x05u);
}

static const core_machine_firmware_provider firmware_probe_provider = {
    firmware_probe_configure,
    firmware_probe_reset,
    firmware_probe_after_run
};

static const core_machine_firmware_provider firmware_failed_probe_provider = {
    firmware_failed_probe_configure,
    firmware_failed_probe_reset,
    STD_NULL
};

C_INT main(C_VOID)
{
    core_machine *machine = STD_NULL;
    core_machine_run_budget budget = { 1u, 0u };
    core_machine_run_result result;
    firmware_probe probe = {0};
    firmware_failed_probe failed_probe = {0};
    core_machine_memory_route route = CORE_MACHINE_MEMORY_ROUTE_ORDINARY_RAM;
    const uint8_t prior_rom = 0xebu;
    uint8_t value = 0u;
    uint32_t port_value = 0u;
    C_INT failed = 0;
    type_status run_status;
    core_machine_lifecycle lifecycle = CORE_MACHINE_INITIALIZED;

    failed |= test_core_machine_create_executor(
        CORE_MACHINE_MINIMUM_MEMORY_BYTES, &machine) != TYPE_STATUS_OK;
    failed |= core_machine_register_immutable_rom_mapping(machine, 0xd0000u,
        &prior_rom, sizeof(prior_rom)) != TYPE_STATUS_OK;
    failed |= core_machine_bind_firmware_provider(machine,
        &firmware_failed_probe_provider, &failed_probe) != TYPE_STATUS_FAULT;
    failed |= failed_probe.configure_calls != 1;
    failed |= test_core_machine_fixture_query_configuration_memory_route(machine,
        0xd0000u, sizeof(prior_rom),
        CORE_MACHINE_MEMORY_ACCESS_READ, &route) != TYPE_STATUS_OK ||
        route != CORE_MACHINE_MEMORY_ROUTE_PROVIDER;
    failed |= test_core_machine_fixture_query_configuration_memory_route(machine,
        0xe0000u, sizeof(prior_rom),
        CORE_MACHINE_MEMORY_ACCESS_READ, &route) != TYPE_STATUS_OK ||
        route != CORE_MACHINE_MEMORY_ROUTE_ORDINARY_RAM;
    failed |= core_machine_firmware_memory_read(failed_probe.expired, 0xe0000u,
        &value, sizeof(value)) != TYPE_STATUS_INVALID_STATE;
    probe.machine = machine;
    failed |= core_machine_bind_firmware_provider(machine,
        &firmware_probe_provider, &probe) != TYPE_STATUS_OK;
    failed |= core_machine_freeze_execution_providers(machine) != TYPE_STATUS_OK;
    failed |= core_machine_bind_firmware_provider(machine,
        &firmware_probe_provider, &probe) != TYPE_STATUS_INVALID_ARGUMENT;
    failed |= core_machine_reset(machine) != TYPE_STATUS_OK;
    failed |= probe.configure_calls != 1 || probe.reset_calls != 1 ||
        !probe.reentry_rejected;
    failed |= core_machine_firmware_memory_read(probe.expired, 0x500u, &value,
        sizeof(value)) != TYPE_STATUS_INVALID_STATE;
    failed |= core_machine_get_lifecycle(machine, &lifecycle) != TYPE_STATUS_OK ||
        lifecycle != CORE_MACHINE_STOPPED;
    run_status = core_machine_run(machine, budget, &result);
    failed |= run_status != TYPE_STATUS_OK ||
        result.reason != CORE_MACHINE_STOP_BUDGET || probe.after_run_calls != 1;
    failed |= core_machine_firmware_port_read(probe.expired, 0x03d8u, &port_value) !=
        TYPE_STATUS_INVALID_STATE;
    core_machine_destroy(machine);
    if (failed) {
        fprintf(stderr, "firmware-probe failed-configure=%d configure=%d reset=%d after=%d reentry=%d life=%d run=%d reason=%d\\n",
            failed_probe.configure_calls,
            probe.configure_calls, probe.reset_calls, probe.after_run_calls,
            probe.reentry_rejected, (int)lifecycle, (int)run_status, (int)result.reason);
        return 1;
    }
    puts("M5:T297:S3:FIRMWARE-CAPABILITY:OK");
    return 0;
}
