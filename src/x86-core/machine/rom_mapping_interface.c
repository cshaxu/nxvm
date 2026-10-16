#include "lib/types/types_interface.h"

#include "x86-core/machine/machine.h"

static lib_status x86_core_machine_rom_mapping_read(void *owner,
    lib_u32 physical, lib_uptr destination,
    lib_uptr bytes, lib_bool observe_only)
{
    const x86_core_machine_immutable_rom_mapping *mapping =
        (const x86_core_machine_immutable_rom_mapping *)owner;
    lib_size offset;

    (void)observe_only;
    if (mapping == LIB_NULL || mapping->image == LIB_NULL || destination == 0u ||
        physical < mapping->physical_start) return LIB_STATUS_INTERNAL_ERROR;
    offset = (lib_size)((lib_u64)physical - mapping->physical_start);
    if (offset > mapping->bytes || bytes > mapping->bytes - offset) {
        return LIB_STATUS_INTERNAL_ERROR;
    }
    lib_memory_copy((void *)destination, mapping->image + offset, bytes);
    return LIB_STATUS_OK;
}

static lib_status x86_core_machine_rom_mapping_write(void *owner,
    lib_u32 physical, lib_uptr source,
    lib_uptr bytes)
{
    const x86_core_machine_immutable_rom_mapping *mapping =
        (const x86_core_machine_immutable_rom_mapping *)owner;
    lib_size offset;

    if (mapping == LIB_NULL || mapping->image == LIB_NULL || source == 0u ||
        physical < mapping->physical_start) return LIB_STATUS_INTERNAL_ERROR;
    offset = (lib_size)((lib_u64)physical - mapping->physical_start);
    if (offset > mapping->bytes || bytes > mapping->bytes - offset) {
        return LIB_STATUS_INTERNAL_ERROR;
    }
    /* A selected board RAM mapping has already won route selection before this
     * ordinary ROM provider.  Otherwise a decoded ROM consumes a write on the
     * bus without changing its immutable bytes. */
    return LIB_STATUS_OK;
}

static lib_status x86_core_machine_rom_mapping_query(void *owner,
    lib_u32 physical, lib_uptr bytes,
    x86_core_machine_memory_access access)
{
    const x86_core_machine_immutable_rom_mapping *mapping =
        (const x86_core_machine_immutable_rom_mapping *)owner;
    lib_size offset;

    if (mapping == LIB_NULL || mapping->image == LIB_NULL ||
        physical < mapping->physical_start) return LIB_STATUS_INTERNAL_ERROR;
    offset = (lib_size)((lib_u64)physical - mapping->physical_start);
    if (offset > mapping->bytes || bytes > mapping->bytes - offset) {
        return LIB_STATUS_INTERNAL_ERROR;
    }
    (void)access;
    return LIB_STATUS_OK;
}

static const x86_core_machine_memory_device_callbacks rom_callbacks = {
    x86_core_machine_rom_mapping_read, x86_core_machine_rom_mapping_write,
    x86_core_machine_rom_mapping_query
};

static lib_status x86_core_machine_register_immutable_rom_mapping_internal(
    x86_core_machine *machine, lib_u32 physical_start, const lib_u8 *image,
    lib_size bytes, lib_i32 firmware_call)
{
    const x86_core_machine_memory_device_route route = {
        physical_start, bytes, rom_callbacks,
        X86_CORE_MACHINE_MEMORY_PROVIDER_STANDARD
    };
    x86_core_machine_immutable_rom_mapping *mapping;
    lib_u8 *copy;
    lib_status status;

    if (machine == LIB_NULL || image == LIB_NULL || bytes == 0u ||
        (lib_u64)physical_start + bytes > (lib_u64)LIB_UINT32_MAX + 1u) {
        return LIB_STATUS_INVALID_ARGUMENT;
    }
    if ((!firmware_call && !x86_core_machine_configuration_is_open(machine)) ||
        (firmware_call && !(machine->firmware_operation_active &&
          machine->firmware_context.active && machine->firmware_context.configuring))) {
        return LIB_STATUS_INVALID_STATE;
    }
    if (machine->immutable_rom_mapping_count >=
        X86_CORE_MACHINE_IMMUTABLE_ROM_MAPPING_CAPACITY) return LIB_STATUS_NO_MEMORY;

    copy = (lib_u8 *)lib_allocate_zero(1u, bytes);
    if (copy == LIB_NULL) return LIB_STATUS_NO_MEMORY;
    lib_memory_copy(copy, image, bytes);
    mapping = &machine->immutable_rom_mappings[machine->immutable_rom_mapping_count];
    mapping->physical_start = physical_start;
    mapping->bytes = bytes;
    mapping->image = copy;
    mapping->owns_image = LIB_TRUE;
    status = x86_core_machine_install_memory_device_routes(machine, &route, 1u,
        LIB_NULL, LIB_NULL, mapping);
    if (status != LIB_STATUS_OK) {
        lib_release(copy);
        lib_memory_set(mapping, 0, sizeof(*mapping));
        return status;
    }
    ++machine->immutable_rom_mapping_count;
    return LIB_STATUS_OK;
}

lib_status x86_core_machine_register_immutable_rom_mapping(
    x86_core_machine *machine, lib_u32 physical_start, const lib_u8 *image,
    lib_size bytes)
{
    return x86_core_machine_register_immutable_rom_mapping_internal(machine,
        physical_start, image, bytes, 0);
}

static lib_status x86_core_machine_register_immutable_rom_mapping_alias_internal(
    x86_core_machine *machine, lib_u32 source_start,
    lib_u32 physical_start, lib_size bytes, lib_i32 firmware_call,
    lib_u8 raw_decode)
{
    const x86_core_machine_memory_device_route route = {
        physical_start, bytes, rom_callbacks,
        raw_decode ? X86_CORE_MACHINE_MEMORY_PROVIDER_RESET_OVERLAY :
            X86_CORE_MACHINE_MEMORY_PROVIDER_OVERLAY
    };
    x86_core_machine_immutable_rom_mapping *source = LIB_NULL;
    x86_core_machine_immutable_rom_mapping *mapping;
    lib_size index;
    lib_size source_offset;
    lib_status status;

    if (machine == LIB_NULL || bytes == 0u ||
        (lib_u64)physical_start + bytes >
            (lib_u64)LIB_UINT32_MAX + 1u) {
        return LIB_STATUS_INVALID_ARGUMENT;
    }
    if ((!firmware_call && !x86_core_machine_configuration_is_open(machine)) ||
        (firmware_call && !(machine->firmware_operation_active &&
          machine->firmware_context.active && machine->firmware_context.configuring))) {
        return LIB_STATUS_INVALID_STATE;
    }
    if (machine->immutable_rom_mapping_count >=
        X86_CORE_MACHINE_IMMUTABLE_ROM_MAPPING_CAPACITY) return LIB_STATUS_NO_MEMORY;

    for (index = 0u; index < machine->immutable_rom_mapping_count; ++index) {
        x86_core_machine_immutable_rom_mapping *candidate =
            &machine->immutable_rom_mappings[index];

        if (source_start >= candidate->physical_start &&
            (lib_u64)source_start - candidate->physical_start + bytes <=
                candidate->bytes) {
            source = candidate;
            break;
        }
    }
    if (source == LIB_NULL) return LIB_STATUS_INVALID_ARGUMENT;

    source_offset = (lib_size)((lib_u64)source_start -
        source->physical_start);
    mapping = &machine->immutable_rom_mappings[machine->immutable_rom_mapping_count];
    mapping->physical_start = physical_start;
    mapping->bytes = bytes;
    mapping->image = source->image + source_offset;
    mapping->owns_image = LIB_FALSE;
    status = x86_core_machine_install_memory_device_routes(machine, &route, 1u,
        LIB_NULL, LIB_NULL, mapping);
    if (status != LIB_STATUS_OK) {
        lib_memory_set(mapping, 0, sizeof(*mapping));
        return status;
    }
    ++machine->immutable_rom_mapping_count;
    return LIB_STATUS_OK;
}

lib_status x86_core_machine_register_immutable_rom_mapping_alias(
    x86_core_machine *machine, lib_u32 source_start,
    lib_u32 physical_start, lib_size bytes)
{
    return x86_core_machine_register_immutable_rom_mapping_alias_internal(machine,
        source_start, physical_start, bytes, 0, LIB_FALSE);
}

lib_status x86_core_machine_register_immutable_rom_mapping_reset_alias(
    x86_core_machine *machine, lib_u32 source_start,
    lib_u32 physical_start, lib_size bytes)
{
    return x86_core_machine_register_immutable_rom_mapping_alias_internal(machine,
        source_start, physical_start, bytes, 0, LIB_TRUE);
}

lib_status x86_core_machine_register_immutable_rom_mapping_from_firmware(
    x86_core_machine *machine, lib_u32 physical_start, const lib_u8 *image,
    lib_size bytes)
{
    return x86_core_machine_register_immutable_rom_mapping_internal(machine,
        physical_start, image, bytes, 1);
}

lib_status x86_core_machine_register_immutable_rom_mapping_alias_from_firmware(
    x86_core_machine *machine, lib_u32 source_start,
    lib_u32 physical_start, lib_size bytes)
{
    return x86_core_machine_register_immutable_rom_mapping_alias_internal(machine,
        source_start, physical_start, bytes, 1, LIB_FALSE);
}

void x86_core_machine_rollback_immutable_rom_mappings(x86_core_machine *machine,
    lib_size mapping_count)
{
    lib_size mapping_index;

    if (machine == LIB_NULL || mapping_count > machine->immutable_rom_mapping_count) {
        return;
    }
    for (mapping_index = machine->immutable_rom_mapping_count;
            mapping_index > mapping_count; --mapping_index) {
        x86_core_machine_immutable_rom_mapping *mapping =
            &machine->immutable_rom_mappings[mapping_index - 1u];

        (void)x86_core_machine_remove_memory_device_routes(machine, mapping);
        if (mapping->owns_image) lib_release(mapping->image);
        lib_memory_set(mapping, 0, sizeof(*mapping));
    }
    machine->immutable_rom_mapping_count = mapping_count;
}

lib_bool x86_core_machine_immutable_rom_mapping_contains(
    const x86_core_machine *machine, lib_u32 physical_start, lib_size bytes)
{
    if (machine == LIB_NULL || bytes == 0u ||
        bytes > (lib_u64)LIB_UINT32_MAX + 1u - physical_start) return LIB_FALSE;
    for (lib_size index = 0u; index < machine->immutable_rom_mapping_count; ++index) {
        const x86_core_machine_immutable_rom_mapping *mapping =
            &machine->immutable_rom_mappings[index];

        if (physical_start >= mapping->physical_start &&
            (lib_u64)physical_start - mapping->physical_start + bytes <=
                mapping->bytes) return LIB_TRUE;
    }
    return LIB_FALSE;
}

lib_status x86_core_machine_register_immutable_rom_mapping_reset_window(
    x86_core_machine *machine, lib_u32 source_start,
    lib_u32 physical_start, lib_size bytes)
{
    lib_size boundary;
    lib_u64 window_end;
    lib_status status = LIB_STATUS_INVALID_ARGUMENT;

    if (machine == LIB_NULL || bytes == 0u ||
        bytes > (lib_u64)LIB_UINT32_MAX + 1u - source_start ||
        bytes > (lib_u64)LIB_UINT32_MAX + 1u - physical_start) {
        return LIB_STATUS_INVALID_ARGUMENT;
    }
    if (!x86_core_machine_configuration_is_open(machine)) return LIB_STATUS_INVALID_STATE;
    boundary = machine->immutable_rom_mapping_count;
    window_end = (lib_u64)source_start + bytes;
    for (lib_size index = 0u; index < boundary; ++index) {
        const x86_core_machine_immutable_rom_mapping *mapping =
            &machine->immutable_rom_mappings[index];
        lib_u64 end = (lib_u64)mapping->physical_start + mapping->bytes;
        lib_u32 start;

        if (end <= source_start || mapping->physical_start >= window_end) continue;
        start = mapping->physical_start < source_start ? source_start :
            mapping->physical_start;
        if (end > window_end) end = window_end;
        status = x86_core_machine_register_immutable_rom_mapping_reset_alias(machine,
            start, physical_start + (start - source_start), (lib_size)(end - start));
        if (status != LIB_STATUS_OK) {
            x86_core_machine_rollback_immutable_rom_mappings(machine, boundary);
            return status;
        }
    }
    return status;
}
