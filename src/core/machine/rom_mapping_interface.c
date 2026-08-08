#include "type.h"

#include "core/machine/machine.h"

static type_status core_machine_rom_mapping_read(C_VOID *owner,
    type_unsigned_32 physical, type_virtual_address destination,
    type_native_unsigned bytes)
{
    const core_machine_immutable_rom_mapping *mapping =
        (const core_machine_immutable_rom_mapping *)owner;
    STD_SIZE_T offset;

    if (mapping == STD_NULL || mapping->image == STD_NULL || destination == 0u ||
        physical < mapping->physical_start) return TYPE_STATUS_FAULT;
    offset = (STD_SIZE_T)((uint64_t)physical - mapping->physical_start);
    if (offset > mapping->bytes || bytes > mapping->bytes - offset) {
        return TYPE_STATUS_FAULT;
    }
    STD_MEMCPY((C_VOID *)destination, mapping->image + offset, bytes);
    return TYPE_STATUS_OK;
}

static type_status core_machine_rom_mapping_write(C_VOID *owner,
    type_unsigned_32 physical, type_virtual_address source,
    type_native_unsigned bytes)
{
    (C_VOID)owner;
    (C_VOID)physical;
    (C_VOID)source;
    (C_VOID)bytes;
    return TYPE_STATUS_FAULT;
}

static type_status core_machine_rom_mapping_query(C_VOID *owner,
    type_unsigned_32 physical, type_native_unsigned bytes,
    core_machine_memory_access access)
{
    const core_machine_immutable_rom_mapping *mapping =
        (const core_machine_immutable_rom_mapping *)owner;
    STD_SIZE_T offset;

    if (mapping == STD_NULL || mapping->image == STD_NULL ||
        physical < mapping->physical_start) return TYPE_STATUS_FAULT;
    offset = (STD_SIZE_T)((uint64_t)physical - mapping->physical_start);
    if (offset > mapping->bytes || bytes > mapping->bytes - offset) {
        return TYPE_STATUS_FAULT;
    }
    return access == CORE_MACHINE_MEMORY_ACCESS_READ ? TYPE_STATUS_OK :
        TYPE_STATUS_FAULT;
}

static type_status core_machine_register_immutable_rom_mapping_internal(
    core_machine *machine, uint32_t physical_start, const uint8_t *image,
    STD_SIZE_T bytes, C_INT firmware_call)
{
    core_machine_immutable_rom_mapping *mapping;
    uint8_t *copy;
    type_status status;

    if (machine == STD_NULL || image == STD_NULL || bytes == 0u ||
        (uint64_t)physical_start + bytes > (uint64_t)TYPE_MAX_UNSIGNED_32 + 1u) {
        return TYPE_STATUS_INVALID_ARGUMENT;
    }
    if ((!firmware_call && !core_machine_configuration_is_open(machine)) ||
        (firmware_call && !(machine->firmware_operation_active &&
          machine->firmware_context.active && machine->firmware_context.configuring))) {
        return TYPE_STATUS_INVALID_STATE;
    }
    if (machine->immutable_rom_mapping_count >=
        CORE_MACHINE_IMMUTABLE_ROM_MAPPING_CAPACITY) return TYPE_STATUS_NO_MEMORY;

    copy = (uint8_t *)STD_CALLOC(1u, bytes);
    if (copy == STD_NULL) return TYPE_STATUS_NO_MEMORY;
    STD_MEMCPY(copy, image, bytes);
    mapping = &machine->immutable_rom_mappings[machine->immutable_rom_mapping_count];
    mapping->physical_start = physical_start;
    mapping->bytes = bytes;
    mapping->image = copy;
    status = core_machine_memory_register_device_provider(&machine->executor_memory,
        physical_start, bytes, core_machine_rom_mapping_read,
        core_machine_rom_mapping_write, core_machine_rom_mapping_query, mapping);
    if (status != TYPE_STATUS_OK) {
        STD_FREE(copy);
        STD_MEMSET(mapping, 0, sizeof(*mapping));
        return status;
    }
    ++machine->immutable_rom_mapping_count;
    return TYPE_STATUS_OK;
}

type_status core_machine_register_immutable_rom_mapping(
    core_machine *machine, uint32_t physical_start, const uint8_t *image,
    STD_SIZE_T bytes)
{
    return core_machine_register_immutable_rom_mapping_internal(machine,
        physical_start, image, bytes, 0);
}

type_status core_machine_register_immutable_rom_mapping_from_firmware(
    core_machine *machine, uint32_t physical_start, const uint8_t *image,
    STD_SIZE_T bytes)
{
    return core_machine_register_immutable_rom_mapping_internal(machine,
        physical_start, image, bytes, 1);
}

C_VOID core_machine_rollback_immutable_rom_mappings(core_machine *machine,
    STD_SIZE_T mapping_count)
{
    STD_SIZE_T mapping_index;

    if (machine == STD_NULL || mapping_count > machine->immutable_rom_mapping_count) {
        return;
    }
    for (mapping_index = machine->immutable_rom_mapping_count;
            mapping_index > mapping_count; --mapping_index) {
        core_machine_immutable_rom_mapping *mapping =
            &machine->immutable_rom_mappings[mapping_index - 1u];
        type_native_unsigned provider_index;

        for (provider_index = machine->executor_memory.connect.device_provider_count;
                provider_index > 0u; --provider_index) {
            core_machine_memory_device_provider *provider =
                &machine->executor_memory.connect.device_providers[provider_index - 1u];

            if (provider->owner == mapping) {
                const type_native_unsigned tail =
                    machine->executor_memory.connect.device_provider_count - 1u;

                if (provider_index - 1u != tail) {
                    machine->executor_memory.connect.device_providers[provider_index - 1u] =
                        machine->executor_memory.connect.device_providers[tail];
                }
                STD_MEMSET(&machine->executor_memory.connect.device_providers[tail], 0,
                    sizeof(machine->executor_memory.connect.device_providers[tail]));
                --machine->executor_memory.connect.device_provider_count;
                break;
            }
        }
        STD_FREE(mapping->image);
        STD_MEMSET(mapping, 0, sizeof(*mapping));
    }
    machine->immutable_rom_mapping_count = mapping_count;
}
