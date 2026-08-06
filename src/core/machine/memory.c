/* Copyright 2012-2014 Neko. */

/* VRAM is the random accessing memory module.  */

#include "type.h"

#include "core/machine/pit.h"
#include "core/machine/memory.h"
#include "core/machine/port.h"

/* Allocates memory for virtual machine ram */
static type_unsigned_32 core_machine_memory_wrap_a20(const t_ram *ram,
    type_unsigned_32 offset)
{
    return offset & (ram->data.flagA20 ? TYPE_MAX_UNSIGNED_32 : ~VRAM_BIT_A20);
}

static type_status core_machine_memory_offset(const t_ram *ram,
    type_unsigned_32 physical, STD_SIZE_T size, STD_SIZE_T *out_offset)
{
    STD_SIZE_T offset;
    type_native_unsigned index;

    if (ram == STD_NULL || out_offset == STD_NULL || ram->connect.backing == 0u) {
        return TYPE_STATUS_INVALID_ARGUMENT;
    }
    offset = (STD_SIZE_T)core_machine_memory_wrap_a20(ram, physical);
    for (index = 0u; index < ram->connect.mapping_count; ++index) {
        const core_machine_memory_mapping *mapping = &ram->connect.mappings[index];
        if (physical >= mapping->physical_start &&
            (uint64_t)physical - mapping->physical_start + size <= mapping->bytes) {
            offset = (STD_SIZE_T)mapping->backing_start +
                (STD_SIZE_T)((uint64_t)physical - mapping->physical_start);
            break;
        }
    }
    if (offset > ram->connect.installed_bytes ||
        size > ram->connect.installed_bytes - offset) {
        return TYPE_STATUS_FAULT;
    }
    *out_offset = offset;
    return TYPE_STATUS_OK;
}

static const core_machine_memory_device_provider *
core_machine_memory_device_provider_find(const t_ram *ram,
    type_unsigned_32 physical, STD_SIZE_T bytes)
{
    type_native_unsigned index;

    if (ram == STD_NULL || bytes == 0u) return STD_NULL;
    for (index = 0u; index < ram->connect.device_provider_count; ++index) {
        const core_machine_memory_device_provider *provider =
            &ram->connect.device_providers[index];
        if (physical >= provider->physical_start &&
            (uint64_t)physical - provider->physical_start + bytes <=
                provider->bytes) {
            return provider;
        }
    }
    return STD_NULL;
}

static C_INT core_machine_memory_access_is_valid(core_machine_memory_access access)
{
    return access == CORE_MACHINE_MEMORY_ACCESS_READ ||
        access == CORE_MACHINE_MEMORY_ACCESS_WRITE;
}

/* Resolve one frozen physical route. Provider queries are metadata-only; data
 * callbacks remain exclusive to actual read/write operations. */
static type_status core_machine_memory_route_resolve(const t_ram *ram,
    type_unsigned_32 physical, type_native_unsigned bytes,
    core_machine_memory_access access,
    const core_machine_memory_device_provider **out_provider,
    STD_SIZE_T *out_offset)
{
    const core_machine_memory_device_provider *provider;
    type_unsigned_32 wrapped;
    type_status status;

    if (ram == STD_NULL || out_provider == STD_NULL || out_offset == STD_NULL ||
        bytes == 0u || !core_machine_memory_access_is_valid(access)) {
        return TYPE_STATUS_INVALID_ARGUMENT;
    }
    wrapped = core_machine_memory_wrap_a20(ram, physical);
    provider = core_machine_memory_device_provider_find(ram, wrapped, bytes);
    if (provider != STD_NULL) {
        status = provider->query(provider->owner, wrapped, bytes, access);
        if (status == TYPE_STATUS_OK) {
            *out_provider = provider;
            return TYPE_STATUS_OK;
        }
        if (status != TYPE_STATUS_UNSUPPORTED) return status;
    }
    status = core_machine_memory_offset(ram, physical, bytes, out_offset);
    if (status != TYPE_STATUS_OK) return status;
    *out_provider = STD_NULL;
    return TYPE_STATUS_OK;
}

/* Allocates one core-owned RAM backing. Callers retain the t_ram, never backing. */
type_status core_machine_memory_allocate_for(t_ram *ram, STD_SIZE_T bytes) {
    C_VOID *backing;

    if (ram == STD_NULL || bytes == 0u) return TYPE_STATUS_INVALID_ARGUMENT;
    backing = STD_CALLOC(1u, bytes);
    if (backing == STD_NULL) return TYPE_STATUS_NO_MEMORY;
    STD_FREE((C_VOID *)ram->connect.backing);
    ram->connect.backing = (type_virtual_address)backing;
    ram->connect.installed_bytes = bytes;
    ram->connect.backing_capacity = bytes;
    return TYPE_STATUS_OK;
}

type_status core_machine_memory_register_mapping(t_ram *ram,
    type_unsigned_32 physical_start,
    type_unsigned_32 backing_start, STD_SIZE_T bytes)
{
    core_machine_memory_mapping *mapping;

    if (ram == STD_NULL || ram->connect.mappings_frozen || bytes == 0u ||
        backing_start > ram->connect.installed_bytes ||
        bytes > ram->connect.installed_bytes - backing_start) {
        return TYPE_STATUS_INVALID_ARGUMENT;
    }
    if (ram->connect.mapping_count >= CORE_MACHINE_MEMORY_MAPPING_CAPACITY) {
        return TYPE_STATUS_NO_MEMORY;
    }
    mapping = &ram->connect.mappings[ram->connect.mapping_count++];
    mapping->physical_start = physical_start;
    mapping->backing_start = backing_start;
    mapping->bytes = bytes;
    return TYPE_STATUS_OK;
}

type_status core_machine_memory_register_write_observer(t_ram *ram,
    core_machine_memory_write_observer callback, C_VOID *owner)
{
    core_machine_memory_write_observer_slot *slot;

    if (ram == STD_NULL || callback == STD_NULL || owner == STD_NULL ||
        ram->connect.mappings_frozen) {
        return TYPE_STATUS_INVALID_ARGUMENT;
    }
    if (ram->connect.write_observer_count >=
        CORE_MACHINE_MEMORY_WRITE_OBSERVER_CAPACITY) {
        return TYPE_STATUS_NO_MEMORY;
    }
    slot = &ram->connect.write_observers[ram->connect.write_observer_count++];
    slot->callback = callback;
    slot->owner = owner;
    return TYPE_STATUS_OK;
}

type_status core_machine_memory_register_device_provider(t_ram *ram,
    type_unsigned_32 physical_start, STD_SIZE_T bytes,
    core_machine_memory_device_read read, core_machine_memory_device_write write,
    core_machine_memory_device_query query, C_VOID *owner)
{
    core_machine_memory_device_provider *provider;
    type_native_unsigned index;
    uint64_t end;

    if (ram == STD_NULL || bytes == 0u || read == STD_NULL || write == STD_NULL ||
        query == STD_NULL || owner == STD_NULL || ram->connect.mappings_frozen) {
        return TYPE_STATUS_INVALID_ARGUMENT;
    }
    end = (uint64_t)physical_start + bytes;
    if (end > (uint64_t)TYPE_MAX_UNSIGNED_32 + 1u ||
        ram->connect.device_provider_count >=
            CORE_MACHINE_MEMORY_DEVICE_PROVIDER_CAPACITY) {
        return TYPE_STATUS_NO_MEMORY;
    }
    for (index = 0u; index < ram->connect.device_provider_count; ++index) {
        const core_machine_memory_device_provider *existing =
            &ram->connect.device_providers[index];
        uint64_t existing_end = (uint64_t)existing->physical_start + existing->bytes;

        if ((uint64_t)physical_start < existing_end &&
            (uint64_t)existing->physical_start < end) {
            return TYPE_STATUS_INVALID_ARGUMENT;
        }
    }
    provider = &ram->connect.device_providers[ram->connect.device_provider_count++];
    provider->physical_start = physical_start;
    provider->bytes = bytes;
    provider->read = read;
    provider->write = write;
    provider->query = query;
    provider->owner = owner;
    return TYPE_STATUS_OK;
}

C_VOID core_machine_memory_freeze_mappings(t_ram *ram)
{
    if (ram != STD_NULL) ram->connect.mappings_frozen = TYPE_TRUE;
}
static C_VOID core_machine_memory_read_a20(t_port *port, type_unsigned_16 port_id,
    C_VOID *owner)
{
    t_ram *ram = (t_ram *)owner;

    (C_VOID)port_id;
    if (ram == STD_NULL) return;
    port->data.ioByte = ram->data.flagA20 ? VRAM_FLAG_A20 : TYPE_ZERO_8;
}
static C_VOID core_machine_memory_write_a20(t_port *port, type_unsigned_16 port_id,
    C_VOID *owner)
{
    t_ram *ram = (t_ram *)owner;

    (C_VOID)port_id;
    if (ram == STD_NULL) return;
    ram->data.flagA20 = TYPE_GET_BIT(port->data.ioByte, VRAM_FLAG_A20);
}

type_status core_machine_memory_read_physical(t_ram *ram, type_unsigned_32 physical,
    type_virtual_address destination, type_native_unsigned byte)
{
    STD_SIZE_T offset;
    const core_machine_memory_device_provider *provider;
    type_status status;

    if (ram == STD_NULL || destination == 0u || byte == 0u) {
        return TYPE_STATUS_INVALID_ARGUMENT;
    }
    status = core_machine_memory_route_resolve(ram, physical, byte,
        CORE_MACHINE_MEMORY_ACCESS_READ, &provider, &offset);
    if (status != TYPE_STATUS_OK) return status;
    if (provider != STD_NULL) {
        status = provider->read(provider->owner,
            core_machine_memory_wrap_a20(ram, physical), destination, byte);
        if (status != TYPE_STATUS_UNSUPPORTED) return status;
        status = core_machine_memory_offset(ram, physical, byte, &offset);
        if (status != TYPE_STATUS_OK) return status;
    }
    STD_MEMCPY((C_VOID *)destination,
        (C_VOID *)(ram->connect.backing + offset), byte);
    return TYPE_STATUS_OK;
}
type_status core_machine_memory_write_physical(t_ram *ram, type_unsigned_32 physical,
    type_virtual_address source, type_native_unsigned byte)
{
    STD_SIZE_T offset;
    type_native_unsigned index;
    const core_machine_memory_device_provider *provider;
    type_status status;

    if (ram == STD_NULL || source == 0u || byte == 0u) {
        return TYPE_STATUS_INVALID_ARGUMENT;
    }
    status = core_machine_memory_route_resolve(ram, physical, byte,
        CORE_MACHINE_MEMORY_ACCESS_WRITE, &provider, &offset);
    if (status != TYPE_STATUS_OK) return status;
    if (provider != STD_NULL) {
        status = provider->write(provider->owner,
            core_machine_memory_wrap_a20(ram, physical), source, byte);
        if (status != TYPE_STATUS_UNSUPPORTED) return status;
        status = core_machine_memory_offset(ram, physical, byte, &offset);
        if (status != TYPE_STATUS_OK) return status;
    }
    STD_MEMCPY((C_VOID *)(ram->connect.backing + offset),
        (C_VOID *)source, byte);
    for (index = 0u; index < ram->connect.write_observer_count; ++index) {
        core_machine_memory_write_observer_slot *slot =
            &ram->connect.write_observers[index];
        slot->callback(slot->owner, physical, byte);
    }
    return TYPE_STATUS_OK;
}

type_status core_machine_memory_query_physical(const t_ram *ram,
    type_unsigned_32 physical, type_native_unsigned bytes,
    core_machine_memory_access access, core_machine_memory_route *out_route)
{
    const core_machine_memory_device_provider *provider;
    STD_SIZE_T offset;
    type_status status;

    if (out_route == STD_NULL) return TYPE_STATUS_INVALID_ARGUMENT;
    status = core_machine_memory_route_resolve(ram, physical, bytes, access,
        &provider, &offset);
    if (status != TYPE_STATUS_OK) return status;
    *out_route = provider == STD_NULL ? CORE_MACHINE_MEMORY_ROUTE_ORDINARY_RAM :
        CORE_MACHINE_MEMORY_ROUTE_PROVIDER;
    return TYPE_STATUS_OK;
}

C_VOID core_machine_memory_initialize(t_ram *ram)
{
    if (ram == STD_NULL) return;
    STD_MEMSET((C_VOID *)ram, TYPE_ZERO_8, sizeof(*ram));
    (C_VOID)core_machine_memory_allocate_for(ram, 1u << 24);
}

C_VOID core_machine_memory_reset(t_ram *ram)
{
    if (ram == STD_NULL || ram->connect.backing == 0u) return;
    STD_MEMSET((C_VOID *)&ram->data, TYPE_ZERO_8, sizeof(ram->data));
    STD_MEMSET((C_VOID *)ram->connect.backing, TYPE_ZERO_8,
        ram->connect.backing_capacity);
}

C_VOID core_machine_memory_finalize(t_ram *ram)
{
    if (ram == STD_NULL) return;
    if (ram->connect.backing != 0u) {
        STD_FREE((C_VOID *)ram->connect.backing);
    }
    ram->connect.backing = 0u;
    ram->connect.installed_bytes = 0u;
    ram->connect.backing_capacity = 0u;
}

C_VOID core_machine_memory_register_ports(t_ram *ram, t_port *port)
{
    core_machine_port_add_read(port, 0x0092,
        core_machine_memory_read_a20, ram);
    core_machine_port_add_write(port, 0x0092,
        core_machine_memory_write_a20, ram);
}

type_status core_machine_memory_read_real_from(t_ram *ram, uint16_t segment,
    uint16_t offset, C_VOID *out_data, STD_SIZE_T size)
{
    type_unsigned_32 physical;

    if (ram == STD_NULL || out_data == STD_NULL) return TYPE_STATUS_INVALID_ARGUMENT;
    physical = core_machine_memory_wrap_a20(ram,
        (TYPE_MASK_UNSIGNED_16(segment) << 4) + TYPE_MASK_UNSIGNED_16(offset));
    return core_machine_memory_read_physical(ram, physical,
        (type_virtual_address)out_data, size);
}

type_status core_machine_memory_write_real_to(t_ram *ram, uint16_t segment,
    uint16_t offset, const C_VOID *in_data, STD_SIZE_T size)
{
    type_unsigned_32 physical;

    if (ram == STD_NULL || in_data == STD_NULL) return TYPE_STATUS_INVALID_ARGUMENT;
    physical = core_machine_memory_wrap_a20(ram,
        (TYPE_MASK_UNSIGNED_16(segment) << 4) + TYPE_MASK_UNSIGNED_16(offset));
    return core_machine_memory_write_physical(ram, physical,
        (type_virtual_address)in_data, size);
}
