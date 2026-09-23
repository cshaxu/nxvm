#include "lib/types/types_interface.h"
#include "type.h"

#include "app-nxvm/devices/machine.h"

static type_status core_machine_rom_mapping_read(C_VOID *owner,
    lib_u32 physical, type_virtual_address destination,
    type_native_unsigned bytes)
{
    const core_machine_immutable_rom_mapping *mapping =
        (const core_machine_immutable_rom_mapping *)owner;
    lib_size offset;

    if (mapping == LIB_NULL || mapping->image == LIB_NULL || destination == 0u ||
        physical < mapping->physical_start) return TYPE_STATUS_FAULT;
    offset = (lib_size)((lib_u64)physical - mapping->physical_start);
    if (offset > mapping->bytes || bytes > mapping->bytes - offset) {
        return TYPE_STATUS_FAULT;
    }
    lib_memory_copy((C_VOID *)destination, mapping->image + offset, bytes);
    return TYPE_STATUS_OK;
}

static type_status core_machine_rom_mapping_write(C_VOID *owner,
    lib_u32 physical, type_virtual_address source,
    type_native_unsigned bytes)
{
    const core_machine_immutable_rom_mapping *mapping =
        (const core_machine_immutable_rom_mapping *)owner;
    lib_size offset;

    if (mapping == LIB_NULL || mapping->image == LIB_NULL || source == 0u ||
        physical < mapping->physical_start) return TYPE_STATUS_FAULT;
    offset = (lib_size)((lib_u64)physical - mapping->physical_start);
    if (offset > mapping->bytes || bytes > mapping->bytes - offset) {
        return TYPE_STATUS_FAULT;
    }
    /* A selected board RAM mapping has already won route selection before this
     * ordinary ROM provider.  Otherwise a decoded ROM consumes a write on the
     * bus without changing its immutable bytes. */
    return TYPE_STATUS_OK;
}

static type_status core_machine_rom_mapping_query(C_VOID *owner,
    lib_u32 physical, type_native_unsigned bytes,
    core_machine_memory_access access)
{
    const core_machine_immutable_rom_mapping *mapping =
        (const core_machine_immutable_rom_mapping *)owner;
    lib_size offset;

    if (mapping == LIB_NULL || mapping->image == LIB_NULL ||
        physical < mapping->physical_start) return TYPE_STATUS_FAULT;
    offset = (lib_size)((lib_u64)physical - mapping->physical_start);
    if (offset > mapping->bytes || bytes > mapping->bytes - offset) {
        return TYPE_STATUS_FAULT;
    }
    (C_VOID)access;
    return TYPE_STATUS_OK;
}

static type_status core_machine_register_immutable_rom_mapping_internal(
    core_machine *machine, lib_u32 physical_start, const lib_u8 *image,
    lib_size bytes, C_INT firmware_call)
{
    core_machine_immutable_rom_mapping *mapping;
    lib_u8 *copy;
    type_status status;

    if (machine == LIB_NULL || image == LIB_NULL || bytes == 0u ||
        (lib_u64)physical_start + bytes > (lib_u64)TYPE_MAX_UNSIGNED_32 + 1u) {
        return TYPE_STATUS_INVALID_ARGUMENT;
    }
    if ((!firmware_call && !core_machine_configuration_is_open(machine)) ||
        (firmware_call && !(machine->firmware_operation_active &&
          machine->firmware_context.active && machine->firmware_context.configuring))) {
        return TYPE_STATUS_INVALID_STATE;
    }
    if (machine->immutable_rom_mapping_count >=
        CORE_MACHINE_IMMUTABLE_ROM_MAPPING_CAPACITY) return TYPE_STATUS_NO_MEMORY;

    copy = (lib_u8 *)lib_allocate_zero(1u, bytes);
    if (copy == LIB_NULL) return TYPE_STATUS_NO_MEMORY;
    lib_memory_copy(copy, image, bytes);
    mapping = &machine->immutable_rom_mappings[machine->immutable_rom_mapping_count];
    mapping->physical_start = physical_start;
    mapping->bytes = bytes;
    mapping->image = copy;
    mapping->owns_image = LIB_TRUE;
    status = core_machine_memory_register_device_provider(&machine->executor_memory,
        physical_start, bytes, core_machine_rom_mapping_read,
        core_machine_rom_mapping_write, core_machine_rom_mapping_query, mapping);
    if (status != TYPE_STATUS_OK) {
        lib_release(copy);
        lib_memory_set(mapping, 0, sizeof(*mapping));
        return status;
    }
    ++machine->immutable_rom_mapping_count;
    return TYPE_STATUS_OK;
}

type_status core_machine_register_immutable_rom_mapping(
    core_machine *machine, lib_u32 physical_start, const lib_u8 *image,
    lib_size bytes)
{
    return core_machine_register_immutable_rom_mapping_internal(machine,
        physical_start, image, bytes, 0);
}

static type_status core_machine_register_immutable_rom_mapping_alias_internal(
    core_machine *machine, lib_u32 source_start,
    lib_u32 physical_start, lib_size bytes, C_INT firmware_call,
    type_bool pre_a20)
{
    core_machine_immutable_rom_mapping *source = LIB_NULL;
    core_machine_immutable_rom_mapping *mapping;
    lib_size index;
    lib_size source_offset;
    type_status status;

    if (machine == LIB_NULL || bytes == 0u ||
        (lib_u64)physical_start + bytes >
            (lib_u64)TYPE_MAX_UNSIGNED_32 + 1u) {
        return TYPE_STATUS_INVALID_ARGUMENT;
    }
    if ((!firmware_call && !core_machine_configuration_is_open(machine)) ||
        (firmware_call && !(machine->firmware_operation_active &&
          machine->firmware_context.active && machine->firmware_context.configuring))) {
        return TYPE_STATUS_INVALID_STATE;
    }
    if (machine->immutable_rom_mapping_count >=
        CORE_MACHINE_IMMUTABLE_ROM_MAPPING_CAPACITY) return TYPE_STATUS_NO_MEMORY;

    for (index = 0u; index < machine->immutable_rom_mapping_count; ++index) {
        core_machine_immutable_rom_mapping *candidate =
            &machine->immutable_rom_mappings[index];

        if (source_start >= candidate->physical_start &&
            (lib_u64)source_start - candidate->physical_start + bytes <=
                candidate->bytes) {
            source = candidate;
            break;
        }
    }
    if (source == LIB_NULL) return TYPE_STATUS_INVALID_ARGUMENT;

    source_offset = (lib_size)((lib_u64)source_start -
        source->physical_start);
    mapping = &machine->immutable_rom_mappings[machine->immutable_rom_mapping_count];
    mapping->physical_start = physical_start;
    mapping->bytes = bytes;
    mapping->image = source->image + source_offset;
    mapping->owns_image = LIB_FALSE;
    status = (pre_a20 ? core_machine_memory_register_pre_a20_overlay_device_provider :
        core_machine_memory_register_overlay_device_provider)(&machine->executor_memory,
            physical_start, bytes, core_machine_rom_mapping_read,
            core_machine_rom_mapping_write, core_machine_rom_mapping_query, mapping);
    if (status != TYPE_STATUS_OK) {
        lib_memory_set(mapping, 0, sizeof(*mapping));
        return status;
    }
    ++machine->immutable_rom_mapping_count;
    return TYPE_STATUS_OK;
}

type_status core_machine_register_immutable_rom_mapping_alias(
    core_machine *machine, lib_u32 source_start,
    lib_u32 physical_start, lib_size bytes)
{
    return core_machine_register_immutable_rom_mapping_alias_internal(machine,
        source_start, physical_start, bytes, 0, LIB_FALSE);
}

type_status core_machine_register_immutable_rom_mapping_reset_alias(
    core_machine *machine, lib_u32 source_start,
    lib_u32 physical_start, lib_size bytes)
{
    return core_machine_register_immutable_rom_mapping_alias_internal(machine,
        source_start, physical_start, bytes, 0, LIB_TRUE);
}

type_status core_machine_register_immutable_rom_mapping_from_firmware(
    core_machine *machine, lib_u32 physical_start, const lib_u8 *image,
    lib_size bytes)
{
    return core_machine_register_immutable_rom_mapping_internal(machine,
        physical_start, image, bytes, 1);
}

type_status core_machine_register_immutable_rom_mapping_alias_from_firmware(
    core_machine *machine, lib_u32 source_start,
    lib_u32 physical_start, lib_size bytes)
{
    return core_machine_register_immutable_rom_mapping_alias_internal(machine,
        source_start, physical_start, bytes, 1, LIB_FALSE);
}

C_VOID core_machine_rollback_immutable_rom_mappings(core_machine *machine,
    lib_size mapping_count)
{
    lib_size mapping_index;

    if (machine == LIB_NULL || mapping_count > machine->immutable_rom_mapping_count) {
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
                lib_memory_set(&machine->executor_memory.connect.device_providers[tail], 0,
                    sizeof(machine->executor_memory.connect.device_providers[tail]));
                --machine->executor_memory.connect.device_provider_count;
                break;
            }
        }
        if (mapping->owns_image) lib_release(mapping->image);
        lib_memory_set(mapping, 0, sizeof(*mapping));
    }
    machine->immutable_rom_mapping_count = mapping_count;
}
