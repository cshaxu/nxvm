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
    if (ram->data.flagA20) return offset;
    if (ram->connect.a20_wrap_policy == CORE_MACHINE_A20_WRAP_FIRST_TO_SECOND_MIB) {
        if (offset >= VRAM_BIT_A20 && offset < 2u * VRAM_BIT_A20) {
            return offset - VRAM_BIT_A20;
        }
        return offset;
    }
    return offset & ~VRAM_BIT_A20;
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
            (type_unsigned_64)physical - mapping->physical_start + size <= mapping->bytes) {
            offset = (STD_SIZE_T)mapping->backing_start +
                (STD_SIZE_T)((type_unsigned_64)physical - mapping->physical_start);
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

static C_INT core_machine_memory_access_is_valid(core_machine_memory_access access)
{
    return access == CORE_MACHINE_MEMORY_ACCESS_READ ||
        access == CORE_MACHINE_MEMORY_ACCESS_WRITE;
}

/* Resolve one frozen physical route.  A provider may decline a range with
 * TYPE_STATUS_UNSUPPORTED so a lower registered provider or ordinary RAM owns
 * it; any other query result is terminal. */
static type_status core_machine_memory_route_resolve(const t_ram *ram,
    type_unsigned_32 physical, type_native_unsigned bytes,
    core_machine_memory_access access,
    const core_machine_memory_device_provider **out_provider,
    type_unsigned_32 *out_provider_physical,
    STD_SIZE_T *out_offset)
{
    type_native_unsigned index;
    type_unsigned_32 wrapped;
    type_status status;

    if (ram == STD_NULL || out_provider == STD_NULL ||
        out_provider_physical == STD_NULL || out_offset == STD_NULL ||
        bytes == 0u || !core_machine_memory_access_is_valid(access)) {
        return TYPE_STATUS_INVALID_ARGUMENT;
    }
    for (index = 0u; index < ram->connect.device_provider_count; ++index) {
        const core_machine_memory_device_provider *provider =
            &ram->connect.device_providers[index];

        if (!provider->pre_a20 || physical < provider->physical_start ||
            (type_unsigned_64)physical - provider->physical_start + bytes >
                provider->bytes) continue;
        status = provider->query(provider->owner, physical, bytes, access);
        if (status == TYPE_STATUS_OK) {
            *out_provider = provider;
            *out_provider_physical = physical;
            return TYPE_STATUS_OK;
        }
        if (status != TYPE_STATUS_UNSUPPORTED) return status;
    }
    wrapped = core_machine_memory_wrap_a20(ram, physical);
    for (index = 0u; index < ram->connect.device_provider_count; ++index) {
        const core_machine_memory_device_provider *provider =
            &ram->connect.device_providers[index];

        if (!provider->replacement || wrapped < provider->physical_start ||
            (type_unsigned_64)wrapped - provider->physical_start + bytes >
                provider->bytes) continue;
        status = provider->query(provider->owner, wrapped, bytes, access);
        if (status == TYPE_STATUS_OK) {
            *out_provider = provider;
            *out_provider_physical = wrapped;
            return TYPE_STATUS_OK;
        }
        if (status != TYPE_STATUS_UNSUPPORTED) return status;
    }
    for (index = 0u; index < ram->connect.device_provider_count; ++index) {
        const core_machine_memory_device_provider *provider =
            &ram->connect.device_providers[index];

        if (provider->replacement || wrapped < provider->physical_start ||
            (type_unsigned_64)wrapped - provider->physical_start + bytes >
                provider->bytes) continue;
        status = provider->query(provider->owner, wrapped, bytes, access);
        if (status == TYPE_STATUS_OK) {
            *out_provider = provider;
            *out_provider_physical = wrapped;
            return TYPE_STATUS_OK;
        }
        if (status != TYPE_STATUS_UNSUPPORTED) return status;
    }
    status = core_machine_memory_offset(ram, physical, bytes, out_offset);
    if (status != TYPE_STATUS_OK) return status;
    *out_provider = STD_NULL;
    *out_provider_physical = wrapped;
    return TYPE_STATUS_OK;
}

/* Reset-cache fetches use the CPU's architected high reset address before the
 * board has an opportunity to apply A20 routing.  A present provider is the
 * sole ROM owner; no provider disposition leaves the caller free to use its
 * ordinary explicit backing-memory reset route. */
static type_status core_machine_memory_reset_provider_resolve(const t_ram *ram,
    type_unsigned_32 physical, type_native_unsigned bytes,
    const core_machine_memory_device_provider **out_provider)
{
    type_native_unsigned index;
    type_status status;

    if (ram == STD_NULL || out_provider == STD_NULL || bytes == 0u) {
        return TYPE_STATUS_INVALID_ARGUMENT;
    }
    /* An alias is deliberately an ordinary-route overlay.  At reset it is the
     * architected ROM source, so examine such providers first rather than
     * letting an unpopulated-memory fallback hide the CPU reset vector. */
    for (index = 0u; index < ram->connect.device_provider_count; ++index) {
        const core_machine_memory_device_provider *provider =
            &ram->connect.device_providers[index];

        if (!provider->overlay || physical < provider->physical_start ||
            (type_unsigned_64)physical - provider->physical_start + bytes >
                provider->bytes) continue;
        status = provider->query(provider->owner, physical, bytes,
            CORE_MACHINE_MEMORY_ACCESS_READ);
        if (status == TYPE_STATUS_OK) {
            *out_provider = provider;
            return TYPE_STATUS_OK;
        }
        if (status != TYPE_STATUS_UNSUPPORTED) return status;
    }
    for (index = 0u; index < ram->connect.device_provider_count; ++index) {
        const core_machine_memory_device_provider *provider =
            &ram->connect.device_providers[index];

        if (provider->overlay || physical < provider->physical_start ||
            (type_unsigned_64)physical - provider->physical_start + bytes >
                provider->bytes) continue;
        status = provider->query(provider->owner, physical, bytes,
            CORE_MACHINE_MEMORY_ACCESS_READ);
        if (status == TYPE_STATUS_OK) {
            *out_provider = provider;
            return TYPE_STATUS_OK;
        }
        if (status != TYPE_STATUS_UNSUPPORTED) return status;
    }
    return TYPE_STATUS_UNSUPPORTED;
}
/* Allocates one core-owned RAM backing. Callers retain the t_ram, never backing. */
static type_status core_machine_memory_allocate_for_with_test(t_ram *ram,
    STD_SIZE_T bytes, core_machine_memory_test_allocation *test_allocation)
{
    C_VOID *backing;

    if (ram == STD_NULL || bytes == 0u) return TYPE_STATUS_INVALID_ARGUMENT;
    if (test_allocation != STD_NULL) {
        ++test_allocation->attempts;
        if (test_allocation->fail) return TYPE_STATUS_NO_MEMORY;
    }
    backing = STD_CALLOC(1u, bytes);
    if (backing == STD_NULL) return TYPE_STATUS_NO_MEMORY;
    STD_FREE((C_VOID *)ram->connect.backing);
    ram->connect.backing = (type_virtual_address)backing;
    ram->connect.installed_bytes = bytes;
    ram->connect.backing_capacity = bytes;
    return TYPE_STATUS_OK;
}

static type_unsigned_8 core_machine_memory_odd_parity(type_unsigned_8 value)
{
    type_unsigned_8 parity = 0u;
    while (value != 0u) { parity ^= value & 1u; value >>= 1u; }
    return parity;
}

type_status core_machine_memory_allocate_for(t_ram *ram, STD_SIZE_T bytes)
{
    return core_machine_memory_allocate_for_with_test(ram, bytes, STD_NULL);
}

type_status core_machine_memory_enable_parity(t_ram *ram, STD_SIZE_T bytes,
    core_machine_memory_parity_fault_observer fault, C_VOID *owner)
{
    type_unsigned_8 *parity;

    if (ram == STD_NULL || fault == STD_NULL || owner == STD_NULL || bytes == 0u ||
        bytes > ram->connect.installed_bytes || ram->connect.mappings_frozen ||
        ram->connect.parity != 0u) return TYPE_STATUS_INVALID_ARGUMENT;
    parity = (type_unsigned_8 *)STD_CALLOC(bytes, sizeof(*parity));
    if (parity == STD_NULL) return TYPE_STATUS_NO_MEMORY;
    ram->connect.parity = (type_virtual_address)parity;
    ram->connect.parity_bytes = bytes;
    ram->connect.parity_fault = fault;
    ram->connect.parity_owner = owner;
    return TYPE_STATUS_OK;
}

type_status core_machine_memory_register_mapping(t_ram *ram,
    type_unsigned_32 physical_start,
    type_unsigned_32 backing_start, STD_SIZE_T bytes)
{
    core_machine_memory_mapping *mapping;

    if (ram == STD_NULL || ram->connect.mappings_frozen || bytes == 0u ||
        backing_start > ram->connect.installed_bytes ||
        bytes > ram->connect.installed_bytes - backing_start ||
        (type_unsigned_64)physical_start + bytes >
            (type_unsigned_64)TYPE_MAX_UNSIGNED_32 + 1u) {
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

static type_status core_machine_memory_validate_write_observer(const t_ram *ram,
    core_machine_memory_write_observer callback, C_VOID *owner)
{
    if (ram == STD_NULL || callback == STD_NULL || owner == STD_NULL ||
        ram->connect.mappings_frozen) {
        return TYPE_STATUS_INVALID_ARGUMENT;
    }
    if (ram->connect.write_observer_count >=
        CORE_MACHINE_MEMORY_WRITE_OBSERVER_CAPACITY) {
        return TYPE_STATUS_NO_MEMORY;
    }
    return TYPE_STATUS_OK;
}

static type_status core_machine_memory_validate_device_provider(const t_ram *ram,
    type_unsigned_32 physical_start, STD_SIZE_T bytes,
    core_machine_memory_device_read read, core_machine_memory_device_write write,
    core_machine_memory_device_query query, C_VOID *owner, type_bool overlay)
{
    type_native_unsigned index;
    type_unsigned_64 end;

    if (ram == STD_NULL || bytes == 0u || read == STD_NULL || write == STD_NULL ||
        query == STD_NULL || owner == STD_NULL || ram->connect.mappings_frozen) {
        return TYPE_STATUS_INVALID_ARGUMENT;
    }
    end = (type_unsigned_64)physical_start + bytes;
    if (end > (type_unsigned_64)TYPE_MAX_UNSIGNED_32 + 1u ||
        ram->connect.device_provider_count >=
            CORE_MACHINE_MEMORY_DEVICE_PROVIDER_LIMIT) {
        return TYPE_STATUS_NO_MEMORY;
    }
    if (!overlay) {
        for (index = 0u; index < ram->connect.device_provider_count; ++index) {
            const core_machine_memory_device_provider *existing =
                &ram->connect.device_providers[index];
            const type_unsigned_64 existing_end =
                (type_unsigned_64)existing->physical_start + existing->bytes;

            if (!existing->overlay && (type_unsigned_64)physical_start < existing_end &&
                (type_unsigned_64)existing->physical_start < end) {
                return TYPE_STATUS_INVALID_ARGUMENT;
            }
        }
    }
    return TYPE_STATUS_OK;
}

static type_status core_machine_memory_reserve_device_provider(t_ram *ram)
{
    core_machine_memory_device_provider *providers;
    type_native_unsigned capacity;

    if (ram == STD_NULL) return TYPE_STATUS_INVALID_ARGUMENT;
    if (ram->connect.device_provider_count < ram->connect.device_provider_capacity) {
        return TYPE_STATUS_OK;
    }
    if (ram->connect.device_provider_capacity >= CORE_MACHINE_MEMORY_DEVICE_PROVIDER_LIMIT) {
        return TYPE_STATUS_NO_MEMORY;
    }
    capacity = ram->connect.device_provider_capacity == 0u ?
        CORE_MACHINE_MEMORY_DEVICE_PROVIDER_INITIAL_CAPACITY :
        ram->connect.device_provider_capacity * 2u;
    if (capacity > CORE_MACHINE_MEMORY_DEVICE_PROVIDER_LIMIT) {
        capacity = CORE_MACHINE_MEMORY_DEVICE_PROVIDER_LIMIT;
    }
    if (ram->connect.device_provider_test_allocation != STD_NULL) {
        ++ram->connect.device_provider_test_allocation->attempts;
        if (ram->connect.device_provider_test_allocation->fail) {
            return TYPE_STATUS_NO_MEMORY;
        }
    }
    providers = (core_machine_memory_device_provider *)STD_CALLOC(capacity,
        sizeof(*providers));
    if (providers == STD_NULL) return TYPE_STATUS_NO_MEMORY;
    if (ram->connect.device_provider_count != 0u) {
        STD_MEMCPY(providers, ram->connect.device_providers,
            ram->connect.device_provider_count * sizeof(*providers));
    }
    STD_FREE(ram->connect.device_providers);
    ram->connect.device_providers = providers;
    ram->connect.device_provider_capacity = capacity;
    return TYPE_STATUS_OK;
}
static C_VOID core_machine_memory_append_write_observer(t_ram *ram,
    core_machine_memory_write_observer callback, C_VOID *owner)
{
    core_machine_memory_write_observer_slot *slot =
        &ram->connect.write_observers[ram->connect.write_observer_count++];

    slot->callback = callback;
    slot->owner = owner;
}

static C_VOID core_machine_memory_append_device_provider(t_ram *ram,
    type_unsigned_32 physical_start, STD_SIZE_T bytes,
    core_machine_memory_device_read read, core_machine_memory_device_write write,
    core_machine_memory_device_query query, C_VOID *owner, type_bool overlay,
    type_bool pre_a20, type_bool replacement)
{
    core_machine_memory_device_provider *provider =
        &ram->connect.device_providers[ram->connect.device_provider_count++];

    provider->physical_start = physical_start;
    provider->bytes = bytes;
    provider->read = read;
    provider->write = write;
    provider->query = query;
    provider->owner = owner;
    provider->overlay = overlay;
    provider->pre_a20 = pre_a20;
    provider->replacement = replacement;
}

type_status core_machine_memory_register_write_observer(t_ram *ram,
    core_machine_memory_write_observer callback, C_VOID *owner)
{
    type_status status = core_machine_memory_validate_write_observer(ram,
        callback, owner);

    if (status != TYPE_STATUS_OK) return status;
    core_machine_memory_append_write_observer(ram, callback, owner);
    return TYPE_STATUS_OK;
}

type_status core_machine_memory_register_device_provider(t_ram *ram,
    type_unsigned_32 physical_start, STD_SIZE_T bytes,
    core_machine_memory_device_read read, core_machine_memory_device_write write,
    core_machine_memory_device_query query, C_VOID *owner)
{
    type_status status = core_machine_memory_validate_device_provider(ram,
        physical_start, bytes, read, write, query, owner, TYPE_FALSE);

    if (status != TYPE_STATUS_OK) return status;
    status = core_machine_memory_reserve_device_provider(ram);
    if (status != TYPE_STATUS_OK) return status;
    core_machine_memory_append_device_provider(ram, physical_start, bytes, read,
        write, query, owner, TYPE_FALSE, TYPE_FALSE, TYPE_FALSE);
    return TYPE_STATUS_OK;
}

type_status core_machine_memory_register_overlay_device_provider(t_ram *ram,
    type_unsigned_32 physical_start, STD_SIZE_T bytes,
    core_machine_memory_device_read read, core_machine_memory_device_write write,
    core_machine_memory_device_query query, C_VOID *owner)
{
    type_status status = core_machine_memory_validate_device_provider(ram,
        physical_start, bytes, read, write, query, owner, TYPE_TRUE);

    if (status != TYPE_STATUS_OK) return status;
    status = core_machine_memory_reserve_device_provider(ram);
    if (status != TYPE_STATUS_OK) return status;
    core_machine_memory_append_device_provider(ram, physical_start, bytes, read,
        write, query, owner, TYPE_TRUE, TYPE_FALSE, TYPE_FALSE);
    return TYPE_STATUS_OK;
}

type_status core_machine_memory_register_pre_a20_overlay_device_provider(t_ram *ram,
    type_unsigned_32 physical_start, STD_SIZE_T bytes,
    core_machine_memory_device_read read, core_machine_memory_device_write write,
    core_machine_memory_device_query query, C_VOID *owner)
{
    type_status status = core_machine_memory_validate_device_provider(ram,
        physical_start, bytes, read, write, query, owner, TYPE_TRUE);

    if (status != TYPE_STATUS_OK) return status;
    status = core_machine_memory_reserve_device_provider(ram);
    if (status != TYPE_STATUS_OK) return status;
    core_machine_memory_append_device_provider(ram, physical_start, bytes, read,
        write, query, owner, TYPE_TRUE, TYPE_TRUE, TYPE_FALSE);
    return TYPE_STATUS_OK;
}

type_status core_machine_memory_register_replacement_device_provider(t_ram *ram,
    type_unsigned_32 physical_start, STD_SIZE_T bytes,
    core_machine_memory_device_read read, core_machine_memory_device_write write,
    core_machine_memory_device_query query, C_VOID *owner)
{
    type_status status = core_machine_memory_validate_device_provider(ram,
        physical_start, bytes, read, write, query, owner, TYPE_TRUE);

    if (status != TYPE_STATUS_OK) return status;
    status = core_machine_memory_reserve_device_provider(ram);
    if (status != TYPE_STATUS_OK) return status;
    core_machine_memory_append_device_provider(ram, physical_start, bytes, read,
        write, query, owner, TYPE_TRUE, TYPE_FALSE, TYPE_TRUE);
    return TYPE_STATUS_OK;
}

type_status core_machine_memory_register_device_provider_and_write_observer(
    t_ram *ram, type_unsigned_32 physical_start, STD_SIZE_T bytes,
    core_machine_memory_device_read read, core_machine_memory_device_write write,
    core_machine_memory_device_query query, C_VOID *owner,
    core_machine_memory_write_observer callback)
{
    type_status status = core_machine_memory_validate_device_provider(ram,
        physical_start, bytes, read, write, query, owner, TYPE_FALSE);

    if (status != TYPE_STATUS_OK) return status;
    status = core_machine_memory_validate_write_observer(ram, callback, owner);
    if (status != TYPE_STATUS_OK) return status;
    status = core_machine_memory_reserve_device_provider(ram);
    if (status != TYPE_STATUS_OK) return status;
    core_machine_memory_append_device_provider(ram, physical_start, bytes, read,
        write, query, owner, TYPE_FALSE, TYPE_FALSE, TYPE_FALSE);
    core_machine_memory_append_write_observer(ram, callback, owner);
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
    type_unsigned_32 provider_physical;
    type_status status;

    if (ram == STD_NULL || destination == 0u || byte == 0u) {
        return TYPE_STATUS_INVALID_ARGUMENT;
    }
    status = core_machine_memory_route_resolve(ram, physical, byte,
        CORE_MACHINE_MEMORY_ACCESS_READ, &provider, &provider_physical, &offset);
    if (status != TYPE_STATUS_OK) return status;
    if (provider == STD_NULL && byte > 1u) {
        type_native_unsigned index;
        for (index = 0u; index < byte; ++index) {
            const core_machine_memory_device_provider *single_provider;
            STD_SIZE_T single_offset;
            status = core_machine_memory_route_resolve(ram,
                physical + (type_unsigned_32)index, 1u,
                CORE_MACHINE_MEMORY_ACCESS_READ, &single_provider,
                &provider_physical, &single_offset);
            if (status != TYPE_STATUS_OK) return status;
            if (single_provider != STD_NULL) {
                for (index = 0u; index < byte; ++index) {
                    status = core_machine_memory_read_physical(ram,
                        physical + (type_unsigned_32)index, destination + index, 1u);
                    if (status != TYPE_STATUS_OK) return status;
                }
                return TYPE_STATUS_OK;
            }
        }
    }
    if (provider != STD_NULL) {
        status = provider->read(provider->owner, provider_physical, destination, byte);
        if (status != TYPE_STATUS_UNSUPPORTED) return status;
        status = core_machine_memory_offset(ram, physical, byte, &offset);
        if (status != TYPE_STATUS_OK) return status;
    }
    STD_MEMCPY((C_VOID *)destination,
        (C_VOID *)(ram->connect.backing + offset), byte);
    if (ram->connect.parity != 0u && offset < ram->connect.parity_bytes) {
        type_native_unsigned index;
        type_native_unsigned checked = byte;
        if (checked > ram->connect.parity_bytes - offset) checked =
            ram->connect.parity_bytes - offset;
        for (index = 0u; index < checked; ++index) {
            if (((type_unsigned_8 *)ram->connect.parity)[offset + index] !=
                core_machine_memory_odd_parity(((type_unsigned_8 *)destination)[index])) {
                ram->connect.parity_fault(ram->connect.parity_owner,
                    physical + (type_unsigned_32)index);
                break;
            }
        }
    }
    return TYPE_STATUS_OK;
}

type_status core_machine_memory_read_reset_physical(t_ram *ram,
    type_unsigned_32 physical, type_virtual_address destination,
    type_native_unsigned bytes)
{
    const core_machine_memory_device_provider *provider;
    type_status status;

    if (ram == STD_NULL || destination == 0u || bytes == 0u) {
        return TYPE_STATUS_INVALID_ARGUMENT;
    }
    status = core_machine_memory_reset_provider_resolve(ram, physical, bytes,
        &provider);
    if (status != TYPE_STATUS_OK) return status;
    return provider->read(provider->owner, physical, destination, bytes);
}
type_status core_machine_memory_write_physical(t_ram *ram, type_unsigned_32 physical,
    type_virtual_address source, type_native_unsigned byte)
{
    STD_SIZE_T offset;
    type_native_unsigned index;
    const core_machine_memory_device_provider *provider;
    type_unsigned_32 provider_physical;
    type_status status;

    if (ram == STD_NULL || source == 0u || byte == 0u) {
        return TYPE_STATUS_INVALID_ARGUMENT;
    }
    status = core_machine_memory_route_resolve(ram, physical, byte,
        CORE_MACHINE_MEMORY_ACCESS_WRITE, &provider, &provider_physical, &offset);
    if (status != TYPE_STATUS_OK) return status;
    if (provider == STD_NULL && byte > 1u) {
        type_native_unsigned index;
        for (index = 0u; index < byte; ++index) {
            const core_machine_memory_device_provider *single_provider;
            STD_SIZE_T single_offset;
            status = core_machine_memory_route_resolve(ram,
                physical + (type_unsigned_32)index, 1u,
                CORE_MACHINE_MEMORY_ACCESS_WRITE, &single_provider,
                &provider_physical, &single_offset);
            if (status != TYPE_STATUS_OK) return status;
            if (single_provider != STD_NULL) {
                for (index = 0u; index < byte; ++index) {
                    status = core_machine_memory_write_physical(ram,
                        physical + (type_unsigned_32)index, source + index, 1u);
                    if (status != TYPE_STATUS_OK) return status;
                }
                return TYPE_STATUS_OK;
            }
        }
    }
    if (provider != STD_NULL) {
        status = provider->write(provider->owner, provider_physical, source, byte);
        if (status != TYPE_STATUS_UNSUPPORTED) return status;
        status = core_machine_memory_offset(ram, physical, byte, &offset);
        if (status != TYPE_STATUS_OK) return status;
    }
    STD_MEMCPY((C_VOID *)(ram->connect.backing + offset),
        (C_VOID *)source, byte);
    if (ram->connect.parity != 0u && offset < ram->connect.parity_bytes) {
        type_native_unsigned index;
        type_native_unsigned written = byte;
        if (written > ram->connect.parity_bytes - offset) written =
            ram->connect.parity_bytes - offset;
        for (index = 0u; index < written; ++index) ((type_unsigned_8 *)ram->connect.parity)[offset + index] =
            core_machine_memory_odd_parity(((const type_unsigned_8 *)source)[index]);
    }
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
    type_unsigned_32 provider_physical;
    STD_SIZE_T offset;
    type_status status;

    if (out_route == STD_NULL) return TYPE_STATUS_INVALID_ARGUMENT;
    status = core_machine_memory_route_resolve(ram, physical, bytes, access,
        &provider, &provider_physical, &offset);
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

type_status core_machine_memory_initialize_for(t_ram *ram, STD_SIZE_T bytes,
    core_machine_memory_test_allocation *test_allocation)
{
    if (ram == STD_NULL || bytes == 0u) return TYPE_STATUS_INVALID_ARGUMENT;
    STD_MEMSET((C_VOID *)ram, TYPE_ZERO_8, sizeof(*ram));
    return core_machine_memory_allocate_for_with_test(ram, bytes,
        test_allocation);
}

C_VOID core_machine_memory_reset(t_ram *ram)
{
    if (ram == STD_NULL || ram->connect.backing == 0u) return;
    STD_MEMSET((C_VOID *)&ram->data, TYPE_ZERO_8, sizeof(ram->data));
    STD_MEMSET((C_VOID *)ram->connect.backing, TYPE_ZERO_8,
        ram->connect.backing_capacity);
    if (ram->connect.parity != 0u) STD_MEMSET((C_VOID *)ram->connect.parity,
        TYPE_ZERO_8, ram->connect.parity_bytes);
}

C_VOID core_machine_memory_finalize(t_ram *ram)
{
    if (ram == STD_NULL) return;
    if (ram->connect.backing != 0u) {
        STD_FREE((C_VOID *)ram->connect.backing);
    }
    if (ram->connect.parity != 0u) STD_FREE((C_VOID *)ram->connect.parity);
    STD_FREE(ram->connect.device_providers);
    ram->connect.device_providers = STD_NULL;
    ram->connect.device_provider_count = 0u;
    ram->connect.device_provider_capacity = 0u;
    ram->connect.backing = 0u;
    ram->connect.installed_bytes = 0u;
    ram->connect.backing_capacity = 0u;
}

type_status core_machine_memory_set_a20_wrap_policy(t_ram *ram,
    core_machine_a20_wrap_policy policy)
{
    if (ram == STD_NULL || ram->connect.mappings_frozen ||
        (policy != CORE_MACHINE_A20_WRAP_GLOBAL_MASK &&
        policy != CORE_MACHINE_A20_WRAP_FIRST_TO_SECOND_MIB)) {
        return TYPE_STATUS_INVALID_ARGUMENT;
    }
    ram->connect.a20_wrap_policy = policy;
    return TYPE_STATUS_OK;
}
C_VOID core_machine_memory_register_ports(t_ram *ram, t_port *port)
{
    core_machine_port_add_read(port, 0x0092,
        core_machine_memory_read_a20, ram);
    core_machine_port_add_write(port, 0x0092,
        core_machine_memory_write_a20, ram);
}

type_status core_machine_memory_read_real_from(t_ram *ram, type_unsigned_16 segment,
    type_unsigned_16 offset, C_VOID *out_data, STD_SIZE_T size)
{
    type_unsigned_32 physical;

    if (ram == STD_NULL || out_data == STD_NULL) return TYPE_STATUS_INVALID_ARGUMENT;
    physical = core_machine_memory_wrap_a20(ram,
        (TYPE_MASK_UNSIGNED_16(segment) << 4) + TYPE_MASK_UNSIGNED_16(offset));
    return core_machine_memory_read_physical(ram, physical,
        (type_virtual_address)out_data, size);
}

type_status core_machine_memory_write_real_to(t_ram *ram, type_unsigned_16 segment,
    type_unsigned_16 offset, const C_VOID *in_data, STD_SIZE_T size)
{
    type_unsigned_32 physical;

    if (ram == STD_NULL || in_data == STD_NULL) return TYPE_STATUS_INVALID_ARGUMENT;
    physical = core_machine_memory_wrap_a20(ram,
        (TYPE_MASK_UNSIGNED_16(segment) << 4) + TYPE_MASK_UNSIGNED_16(offset));
    return core_machine_memory_write_physical(ram, physical,
        (type_virtual_address)in_data, size);
}
