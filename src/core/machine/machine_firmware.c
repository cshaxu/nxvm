#include "type.h"

#include "core/machine/machine.h"

type_status core_machine_firmware_invoke(core_machine *machine,
    C_INT configuring, C_INT track_operation_failures,
    type_status (*callback)(C_VOID *,
    core_machine_firmware_context *))
{
    type_status status;

    if (machine == STD_NULL || callback == STD_NULL ||
        machine->firmware_operation_active) return TYPE_STATUS_INVALID_STATE;
    machine->firmware_operation_active = 1;
    machine->firmware_context.machine = machine;
    machine->firmware_context.operation_status = TYPE_STATUS_OK;
    machine->firmware_context.track_operation_failures = track_operation_failures;
    machine->firmware_context.configuring = configuring;
    machine->firmware_context.active = 1;
    status = callback(machine->firmware_provider_context,
        &machine->firmware_context);
    if (status == TYPE_STATUS_OK &&
        machine->firmware_context.operation_status != TYPE_STATUS_OK) {
        status = machine->firmware_context.operation_status;
    }
    machine->firmware_context.active = 0;
    machine->firmware_context.track_operation_failures = 0;
    machine->firmware_context.configuring = 0;
    machine->firmware_operation_active = 0;
    return status;
}

static type_status core_machine_firmware_operation_result(
    core_machine_firmware_context *firmware, type_status status)
{
    if (firmware != STD_NULL && firmware->active &&
        firmware->track_operation_failures &&
        status != TYPE_STATUS_OK &&
        firmware->operation_status == TYPE_STATUS_OK) {
        firmware->operation_status = status;
    }
    return status;
}

static C_INT core_machine_firmware_context_is_active(
    const core_machine_firmware_context *firmware, C_INT configuring)
{
    return firmware != STD_NULL && firmware->active &&
        firmware->machine != STD_NULL &&
        firmware->configuring == configuring &&
        firmware->machine->firmware_operation_active;
}

type_status core_machine_bind_firmware_provider(core_machine *machine,
    const core_machine_firmware_provider *provider, C_VOID *provider_context)
{
    type_status status;
    STD_SIZE_T rom_mapping_boundary;

    if (!core_machine_configuration_is_open(machine) ||
        machine->firmware_provider != STD_NULL || provider == STD_NULL ||
        provider->configure == STD_NULL || provider->reset == STD_NULL) {
        return TYPE_STATUS_INVALID_ARGUMENT;
    }
    rom_mapping_boundary = machine->immutable_rom_mapping_count;
    machine->firmware_provider = provider;
    machine->firmware_provider_context = provider_context;
    status = core_machine_firmware_invoke(machine, 1, 0, provider->configure);
    if (status != TYPE_STATUS_OK) {
        core_machine_rollback_immutable_rom_mappings(machine, rom_mapping_boundary);
        machine->firmware_provider = STD_NULL;
        machine->firmware_provider_context = STD_NULL;
        STD_MEMSET(&machine->firmware_context, 0, sizeof(machine->firmware_context));
        return status;
    }
    /* The firmware supplies only its ordinary F0000h image.  Core derives the
     * CPU-selected reset-vector alias after that source exists, before the
     * configuration boundary freezes. */
    status = core_machine_register_reset_rom_alias(machine);
    if (status != TYPE_STATUS_OK) {
        core_machine_rollback_immutable_rom_mappings(machine, rom_mapping_boundary);
        machine->firmware_provider = STD_NULL;
        machine->firmware_provider_context = STD_NULL;
        STD_MEMSET(&machine->firmware_context, 0, sizeof(machine->firmware_context));
        return status;
    }
    return TYPE_STATUS_OK;
}

type_status core_machine_firmware_register_immutable_rom(
    core_machine_firmware_context *firmware, type_unsigned_32 physical_start,
    const type_unsigned_8 *image, STD_SIZE_T bytes)
{
    if (!core_machine_firmware_context_is_active(firmware, 1)) {
        return core_machine_firmware_operation_result(firmware,
            TYPE_STATUS_INVALID_STATE);
    }
    return core_machine_firmware_operation_result(firmware,
        core_machine_register_immutable_rom_mapping_from_firmware(firmware->machine,
            physical_start, image, bytes));
}

type_status core_machine_firmware_register_immutable_rom_alias(
    core_machine_firmware_context *firmware, type_unsigned_32 source_start,
    type_unsigned_32 physical_start, STD_SIZE_T bytes)
{
    if (!core_machine_firmware_context_is_active(firmware, 1)) {
        return core_machine_firmware_operation_result(firmware,
            TYPE_STATUS_INVALID_STATE);
    }
    return core_machine_firmware_operation_result(firmware,
        core_machine_register_immutable_rom_mapping_alias_from_firmware(
            firmware->machine, source_start, physical_start, bytes));
}

type_status core_machine_firmware_memory_read(
    core_machine_firmware_context *firmware, type_unsigned_32 physical,
    C_VOID *out_data, STD_SIZE_T size)
{
    if (!core_machine_firmware_context_is_active(firmware, 0) ||
        out_data == STD_NULL || size == 0u) {
        return core_machine_firmware_operation_result(firmware,
            TYPE_STATUS_INVALID_STATE);
    }
    return core_machine_firmware_operation_result(firmware,
        core_machine_memory_read_physical(&firmware->machine->executor_memory,
            physical, (type_virtual_address)out_data, size));
}

type_status core_machine_firmware_memory_write(
    core_machine_firmware_context *firmware, type_unsigned_32 physical,
    const C_VOID *data, STD_SIZE_T size)
{
    if (!core_machine_firmware_context_is_active(firmware, 0) ||
        data == STD_NULL || size == 0u) {
        return core_machine_firmware_operation_result(firmware,
            TYPE_STATUS_INVALID_STATE);
    }
    return core_machine_firmware_operation_result(firmware,
        core_machine_memory_write_physical(&firmware->machine->executor_memory,
            physical, (type_virtual_address)data, size));
}

type_status core_machine_firmware_port_read(
    core_machine_firmware_context *firmware, type_unsigned_16 port, type_unsigned_32 *out_value)
{
    if (!core_machine_firmware_context_is_active(firmware, 0) ||
        out_value == STD_NULL) {
        return core_machine_firmware_operation_result(firmware,
            TYPE_STATUS_INVALID_STATE);
    }
    {
        type_status status = core_machine_port_execute_read(
            &firmware->machine->executor_port, port);

        if (status != TYPE_STATUS_OK) {
            return core_machine_firmware_operation_result(firmware, status);
        }
    }
    *out_value = firmware->machine->executor_port.data.ioDWord;
    return TYPE_STATUS_OK;
}

type_status core_machine_firmware_port_write(
    core_machine_firmware_context *firmware, type_unsigned_16 port, type_unsigned_32 value)
{
    if (!core_machine_firmware_context_is_active(firmware, 0)) {
        return core_machine_firmware_operation_result(firmware,
            TYPE_STATUS_INVALID_STATE);
    }
    {
        type_unsigned_32 prior_value = firmware->machine->executor_port.data.ioDWord;
        type_status status;

        firmware->machine->executor_port.data.ioDWord = value;
        status = core_machine_port_execute_write(&firmware->machine->executor_port,
            port);
        if (status != TYPE_STATUS_OK) {
            firmware->machine->executor_port.data.ioDWord = prior_value;
        }
        return core_machine_firmware_operation_result(firmware, status);
    }
}

type_status core_machine_firmware_request_stop(
    core_machine_firmware_context *firmware)
{
    if (!core_machine_firmware_context_is_active(firmware, 0)) {
        return core_machine_firmware_operation_result(firmware,
            TYPE_STATUS_INVALID_STATE);
    }
    STD_ATOMIC_STORE(&firmware->machine->stop_requested, 1);
    return TYPE_STATUS_OK;
}
