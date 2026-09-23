/* Copyright 2012-2014 Neko. */

/* VRAM is the random accessing memory module.  */
#include "lib/types/types_interface.h"

#include "type.h"

#include "app-nxvm/devices/pit.h"
#include "app-nxvm/devices/memory.h"
#include "app-nxvm/devices/port.h"

/* Allocates memory for virtual machine ram */
static lib_u32 core_machine_memory_wrap_a20(const t_ram *ram,
    lib_u32 offset)
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
    lib_u32 physical, lib_size size, lib_size *out_offset)
{
    lib_size offset;
    type_native_unsigned index;

    if (ram == LIB_NULL || out_offset == LIB_NULL || ram->connect.backing == 0u) {
        return TYPE_STATUS_INVALID_ARGUMENT;
    }
    offset = (lib_size)core_machine_memory_wrap_a20(ram, physical);
    for (index = 0u; index < ram->connect.mapping_count; ++index) {
        const core_machine_memory_mapping *mapping = &ram->connect.mappings[index];
        if (physical >= mapping->physical_start &&
            (lib_u64)physical - mapping->physical_start + size <= mapping->bytes) {
            offset = (lib_size)mapping->backing_start +
                (lib_size)((lib_u64)physical - mapping->physical_start);
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
    lib_u32 physical, type_native_unsigned bytes,
    core_machine_memory_access access,
    const core_machine_memory_device_provider **out_provider,
    lib_u32 *out_provider_physical,
    lib_size *out_offset)
{
    type_native_unsigned index;
    lib_u32 wrapped;
    type_status status;

    if (ram == LIB_NULL || out_provider == LIB_NULL ||
        out_provider_physical == LIB_NULL || out_offset == LIB_NULL ||
        bytes == 0u || !core_machine_memory_access_is_valid(access)) {
        return TYPE_STATUS_INVALID_ARGUMENT;
    }
    for (index = 0u; index < ram->connect.device_provider_count; ++index) {
        const core_machine_memory_device_provider *provider =
            &ram->connect.device_providers[index];

        if (!provider->pre_a20 || physical < provider->physical_start ||
            (lib_u64)physical - provider->physical_start + bytes >
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
    /* A frozen board RAM alias is a selected physical decode, not an ordinary
     * backing fallback.  It therefore wins over any lower ordinary device
     * route while preserving the raw pre-A20 reset route above. */
    for (index = 0u; index < ram->connect.mapping_count; ++index) {
        const core_machine_memory_mapping *mapping = &ram->connect.mappings[index];

        if (!mapping->selected || physical < mapping->physical_start ||
            (lib_u64)physical - mapping->physical_start + bytes >
                mapping->bytes) continue;
        *out_offset = (lib_size)mapping->backing_start +
            (lib_size)((lib_u64)physical - mapping->physical_start);
        *out_provider = LIB_NULL;
        *out_provider_physical = wrapped;
        return TYPE_STATUS_OK;
    }
    for (index = 0u; index < ram->connect.device_provider_count; ++index) {
        const core_machine_memory_device_provider *provider =
            &ram->connect.device_providers[index];

        if (!provider->replacement || wrapped < provider->physical_start ||
            (lib_u64)wrapped - provider->physical_start + bytes >
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

        if (provider->replacement || provider->fallback ||
            wrapped < provider->physical_start ||
            (lib_u64)wrapped - provider->physical_start + bytes >
                provider->bytes) continue;
        status = provider->query(provider->owner, wrapped, bytes, access);
        if (status == TYPE_STATUS_OK) {
            *out_provider = provider;
            *out_provider_physical = wrapped;
            return TYPE_STATUS_OK;
        }
        if (status != TYPE_STATUS_UNSUPPORTED) return status;
    }
    /* An unpopulated board window is an explicit fallback: an installed
     * device or ROM alias has first claim. */
    for (index = 0u; index < ram->connect.device_provider_count; ++index) {
        const core_machine_memory_device_provider *provider =
            &ram->connect.device_providers[index];

        if (provider->replacement || !provider->fallback ||
            wrapped < provider->physical_start ||
            (lib_u64)wrapped - provider->physical_start + bytes >
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
    *out_provider = LIB_NULL;
    *out_provider_physical = wrapped;
    return TYPE_STATUS_OK;
}

/* Reset-cache fetches use the CPU's architected high reset address before the
 * board has an opportunity to apply A20 routing.  A present provider is the
 * sole ROM owner; no provider disposition leaves the caller free to use its
 * ordinary explicit backing-memory reset route. */
static type_status core_machine_memory_reset_provider_resolve(const t_ram *ram,
    lib_u32 physical, type_native_unsigned bytes,
    const core_machine_memory_device_provider **out_provider)
{
    type_native_unsigned index;
    type_status status;

    if (ram == LIB_NULL || out_provider == LIB_NULL || bytes == 0u) {
        return TYPE_STATUS_INVALID_ARGUMENT;
    }
    /* An alias is deliberately an ordinary-route overlay.  At reset it is the
     * architected ROM source, so examine such providers first rather than
     * letting an unpopulated-memory fallback hide the CPU reset vector. */
    for (index = 0u; index < ram->connect.device_provider_count; ++index) {
        const core_machine_memory_device_provider *provider =
            &ram->connect.device_providers[index];

        if (provider->fallback || !provider->overlay ||
            physical < provider->physical_start ||
            (lib_u64)physical - provider->physical_start + bytes >
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

        if (provider->fallback || provider->overlay ||
            physical < provider->physical_start ||
            (lib_u64)physical - provider->physical_start + bytes >
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
    lib_size bytes, core_machine_memory_test_allocation *test_allocation)
{
    C_VOID *backing;

    if (ram == LIB_NULL || bytes == 0u) return TYPE_STATUS_INVALID_ARGUMENT;
    if (test_allocation != LIB_NULL) {
        ++test_allocation->attempts;
        if (test_allocation->fail) return TYPE_STATUS_NO_MEMORY;
    }
    backing = lib_allocate_zero(1u, bytes);
    if (backing == LIB_NULL) return TYPE_STATUS_NO_MEMORY;
    lib_release((C_VOID *)ram->connect.backing);
    ram->connect.backing = (type_virtual_address)backing;
    ram->connect.installed_bytes = bytes;
    ram->connect.backing_capacity = bytes;
    return TYPE_STATUS_OK;
}

static lib_u8 core_machine_memory_odd_parity(lib_u8 value)
{
    lib_u8 parity = 0u;
    while (value != 0u) { parity ^= value & 1u; value >>= 1u; }
    return parity;
}

type_status core_machine_memory_allocate_for(t_ram *ram, lib_size bytes)
{
    return core_machine_memory_allocate_for_with_test(ram, bytes, LIB_NULL);
}

type_status core_machine_memory_enable_parity(t_ram *ram, lib_size bytes,
    core_machine_memory_parity_fault_observer fault, C_VOID *owner)
{
    lib_u8 *parity;

    if (ram == LIB_NULL || fault == LIB_NULL || owner == LIB_NULL || bytes == 0u ||
        bytes > ram->connect.installed_bytes || ram->connect.mappings_frozen ||
        ram->connect.parity != 0u) return TYPE_STATUS_INVALID_ARGUMENT;
    parity = (lib_u8 *)lib_allocate_zero(bytes, sizeof(*parity));
    if (parity == LIB_NULL) return TYPE_STATUS_NO_MEMORY;
    ram->connect.parity = (type_virtual_address)parity;
    ram->connect.parity_bytes = bytes;
    ram->connect.parity_fault = fault;
    ram->connect.parity_owner = owner;
    return TYPE_STATUS_OK;
}

type_status core_machine_memory_register_mapping(t_ram *ram,
    lib_u32 physical_start,
    lib_u32 backing_start, lib_size bytes, type_bool selected)
{
    core_machine_memory_mapping *mapping;

    if (ram == LIB_NULL || ram->connect.mappings_frozen ||
        (selected != LIB_FALSE && selected != LIB_TRUE) || bytes == 0u ||
        backing_start > ram->connect.installed_bytes ||
        bytes > ram->connect.installed_bytes - backing_start ||
        (lib_u64)physical_start + bytes >
            (lib_u64)TYPE_MAX_UNSIGNED_32 + 1u) {
        return TYPE_STATUS_INVALID_ARGUMENT;
    }
    if (ram->connect.mapping_count >= CORE_MACHINE_MEMORY_MAPPING_CAPACITY) {
        return TYPE_STATUS_NO_MEMORY;
    }
    mapping = &ram->connect.mappings[ram->connect.mapping_count++];
    mapping->physical_start = physical_start;
    mapping->backing_start = backing_start;
    mapping->bytes = bytes;
    mapping->selected = selected;
    return TYPE_STATUS_OK;
}

static type_status core_machine_memory_validate_write_observer(const t_ram *ram,
    core_machine_memory_write_observer callback, C_VOID *owner)
{
    if (ram == LIB_NULL || callback == LIB_NULL || owner == LIB_NULL ||
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
    lib_u32 physical_start, lib_size bytes,
    core_machine_memory_device_read read, core_machine_memory_device_write write,
    core_machine_memory_device_query query, C_VOID *owner, type_bool overlay)
{
    type_native_unsigned index;
    lib_u64 end;

    if (ram == LIB_NULL || bytes == 0u || read == LIB_NULL || write == LIB_NULL ||
        query == LIB_NULL || owner == LIB_NULL || ram->connect.mappings_frozen) {
        return TYPE_STATUS_INVALID_ARGUMENT;
    }
    end = (lib_u64)physical_start + bytes;
    if (end > (lib_u64)TYPE_MAX_UNSIGNED_32 + 1u ||
        ram->connect.device_provider_count >=
            CORE_MACHINE_MEMORY_DEVICE_PROVIDER_LIMIT) {
        return TYPE_STATUS_NO_MEMORY;
    }
    if (!overlay) {
        for (index = 0u; index < ram->connect.device_provider_count; ++index) {
            const core_machine_memory_device_provider *existing =
                &ram->connect.device_providers[index];
            const lib_u64 existing_end =
                (lib_u64)existing->physical_start + existing->bytes;

            if (!existing->overlay && (lib_u64)physical_start < existing_end &&
                (lib_u64)existing->physical_start < end) {
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

    if (ram == LIB_NULL) return TYPE_STATUS_INVALID_ARGUMENT;
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
    if (ram->connect.device_provider_test_allocation != LIB_NULL) {
        ++ram->connect.device_provider_test_allocation->attempts;
        if (ram->connect.device_provider_test_allocation->fail) {
            return TYPE_STATUS_NO_MEMORY;
        }
    }
    providers = (core_machine_memory_device_provider *)lib_allocate_zero(capacity,
        sizeof(*providers));
    if (providers == LIB_NULL) return TYPE_STATUS_NO_MEMORY;
    if (ram->connect.device_provider_count != 0u) {
        lib_memory_copy(providers, ram->connect.device_providers,
            ram->connect.device_provider_count * sizeof(*providers));
    }
    lib_release(ram->connect.device_providers);
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
    lib_u32 physical_start, lib_size bytes,
    core_machine_memory_device_read read, core_machine_memory_device_write write,
    core_machine_memory_device_query query, C_VOID *owner, type_bool overlay,
    type_bool pre_a20, type_bool replacement, type_bool fallback)
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
    provider->fallback = fallback;
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
    lib_u32 physical_start, lib_size bytes,
    core_machine_memory_device_read read, core_machine_memory_device_write write,
    core_machine_memory_device_query query, C_VOID *owner)
{
    type_status status = core_machine_memory_validate_device_provider(ram,
        physical_start, bytes, read, write, query, owner, LIB_FALSE);

    if (status != TYPE_STATUS_OK) return status;
    status = core_machine_memory_reserve_device_provider(ram);
    if (status != TYPE_STATUS_OK) return status;
    core_machine_memory_append_device_provider(ram, physical_start, bytes, read,
        write, query, owner, LIB_FALSE, LIB_FALSE, LIB_FALSE, LIB_FALSE);
    return TYPE_STATUS_OK;
}

type_status core_machine_memory_register_overlay_device_provider(t_ram *ram,
    lib_u32 physical_start, lib_size bytes,
    core_machine_memory_device_read read, core_machine_memory_device_write write,
    core_machine_memory_device_query query, C_VOID *owner)
{
    type_status status = core_machine_memory_validate_device_provider(ram,
        physical_start, bytes, read, write, query, owner, LIB_TRUE);

    if (status != TYPE_STATUS_OK) return status;
    status = core_machine_memory_reserve_device_provider(ram);
    if (status != TYPE_STATUS_OK) return status;
    core_machine_memory_append_device_provider(ram, physical_start, bytes, read,
        write, query, owner, LIB_TRUE, LIB_FALSE, LIB_FALSE, LIB_FALSE);
    return TYPE_STATUS_OK;
}

type_status core_machine_memory_register_pre_a20_overlay_device_provider(t_ram *ram,
    lib_u32 physical_start, lib_size bytes,
    core_machine_memory_device_read read, core_machine_memory_device_write write,
    core_machine_memory_device_query query, C_VOID *owner)
{
    type_status status = core_machine_memory_validate_device_provider(ram,
        physical_start, bytes, read, write, query, owner, LIB_TRUE);

    if (status != TYPE_STATUS_OK) return status;
    status = core_machine_memory_reserve_device_provider(ram);
    if (status != TYPE_STATUS_OK) return status;
    core_machine_memory_append_device_provider(ram, physical_start, bytes, read,
        write, query, owner, LIB_TRUE, LIB_TRUE, LIB_FALSE, LIB_FALSE);
    return TYPE_STATUS_OK;
}

type_status core_machine_memory_register_replacement_device_provider(t_ram *ram,
    lib_u32 physical_start, lib_size bytes,
    core_machine_memory_device_read read, core_machine_memory_device_write write,
    core_machine_memory_device_query query, C_VOID *owner)
{
    type_status status = core_machine_memory_validate_device_provider(ram,
        physical_start, bytes, read, write, query, owner, LIB_TRUE);

    if (status != TYPE_STATUS_OK) return status;
    status = core_machine_memory_reserve_device_provider(ram);
    if (status != TYPE_STATUS_OK) return status;
    core_machine_memory_append_device_provider(ram, physical_start, bytes, read,
        write, query, owner, LIB_TRUE, LIB_FALSE, LIB_TRUE, LIB_FALSE);
    return TYPE_STATUS_OK;
}

type_status core_machine_memory_register_fallback_device_provider(t_ram *ram,
    lib_u32 physical_start, lib_size bytes,
    core_machine_memory_device_read read, core_machine_memory_device_write write,
    core_machine_memory_device_query query, C_VOID *owner)
{
    type_status status = core_machine_memory_validate_device_provider(ram,
        physical_start, bytes, read, write, query, owner, LIB_TRUE);

    if (status != TYPE_STATUS_OK) return status;
    status = core_machine_memory_reserve_device_provider(ram);
    if (status != TYPE_STATUS_OK) return status;
    core_machine_memory_append_device_provider(ram, physical_start, bytes, read,
        write, query, owner, LIB_TRUE, LIB_FALSE, LIB_FALSE, LIB_TRUE);
    return TYPE_STATUS_OK;
}

type_status core_machine_memory_register_device_provider_and_write_observer(
    t_ram *ram, lib_u32 physical_start, lib_size bytes,
    core_machine_memory_device_read read, core_machine_memory_device_write write,
    core_machine_memory_device_query query, C_VOID *owner,
    core_machine_memory_write_observer callback)
{
    type_status status = core_machine_memory_validate_device_provider(ram,
        physical_start, bytes, read, write, query, owner, LIB_FALSE);

    if (status != TYPE_STATUS_OK) return status;
    status = core_machine_memory_validate_write_observer(ram, callback, owner);
    if (status != TYPE_STATUS_OK) return status;
    status = core_machine_memory_reserve_device_provider(ram);
    if (status != TYPE_STATUS_OK) return status;
    core_machine_memory_append_device_provider(ram, physical_start, bytes, read,
        write, query, owner, LIB_FALSE, LIB_FALSE, LIB_FALSE, LIB_FALSE);
    core_machine_memory_append_write_observer(ram, callback, owner);
    return TYPE_STATUS_OK;
}
C_VOID core_machine_memory_freeze_mappings(t_ram *ram)
{
    if (ram != LIB_NULL) ram->connect.mappings_frozen = LIB_TRUE;
}
static C_VOID core_machine_memory_read_a20(t_port *port, lib_u16 port_id,
    C_VOID *owner)
{
    t_ram *ram = (t_ram *)owner;

    (C_VOID)port_id;
    if (ram == LIB_NULL) return;
    port->data.ioByte = ram->data.flagA20 ? VRAM_FLAG_A20 : TYPE_ZERO_8;
}
static C_VOID core_machine_memory_write_a20(t_port *port, lib_u16 port_id,
    C_VOID *owner)
{
    t_ram *ram = (t_ram *)owner;

    (C_VOID)port_id;
    if (ram == LIB_NULL) return;
    ram->data.flagA20 = TYPE_GET_BIT(port->data.ioByte, VRAM_FLAG_A20);
}

type_status core_machine_memory_read_physical(t_ram *ram, lib_u32 physical,
    type_virtual_address destination, type_native_unsigned byte)
{
    lib_size offset;
    const core_machine_memory_device_provider *provider;
    lib_u32 provider_physical;
    type_status status;

    if (ram == LIB_NULL || destination == 0u || byte == 0u) {
        return TYPE_STATUS_INVALID_ARGUMENT;
    }
    status = core_machine_memory_route_resolve(ram, physical, byte,
        CORE_MACHINE_MEMORY_ACCESS_READ, &provider, &provider_physical, &offset);
    if (status != TYPE_STATUS_OK) return status;
    if (provider == LIB_NULL && byte > 1u) {
        type_native_unsigned index;
        for (index = 0u; index < byte; ++index) {
            const core_machine_memory_device_provider *single_provider;
            lib_size single_offset;
            status = core_machine_memory_route_resolve(ram,
                physical + (lib_u32)index, 1u,
                CORE_MACHINE_MEMORY_ACCESS_READ, &single_provider,
                &provider_physical, &single_offset);
            if (status != TYPE_STATUS_OK) return status;
            if (single_provider != LIB_NULL) {
                for (index = 0u; index < byte; ++index) {
                    status = core_machine_memory_read_physical(ram,
                        physical + (lib_u32)index, destination + index, 1u);
                    if (status != TYPE_STATUS_OK) return status;
                }
                return TYPE_STATUS_OK;
            }
        }
    }
    if (provider != LIB_NULL) {
        status = provider->read(provider->owner, provider_physical, destination, byte);
        if (status != TYPE_STATUS_UNSUPPORTED) return status;
        status = core_machine_memory_offset(ram, physical, byte, &offset);
        if (status != TYPE_STATUS_OK) return status;
    }
    lib_memory_copy((C_VOID *)destination,
        (C_VOID *)(ram->connect.backing + offset), byte);
    if (ram->connect.parity != 0u && offset < ram->connect.parity_bytes) {
        type_native_unsigned index;
        type_native_unsigned checked = byte;
        if (checked > ram->connect.parity_bytes - offset) checked =
            ram->connect.parity_bytes - offset;
        for (index = 0u; index < checked; ++index) {
            if (((lib_u8 *)ram->connect.parity)[offset + index] !=
                core_machine_memory_odd_parity(((lib_u8 *)destination)[index])) {
                ram->connect.parity_fault(ram->connect.parity_owner,
                    physical + (lib_u32)index);
                break;
            }
        }
    }
    return TYPE_STATUS_OK;
}

type_status core_machine_memory_read_reset_physical(t_ram *ram,
    lib_u32 physical, type_virtual_address destination,
    type_native_unsigned bytes)
{
    const core_machine_memory_device_provider *provider;
    type_status status;

    if (ram == LIB_NULL || destination == 0u || bytes == 0u) {
        return TYPE_STATUS_INVALID_ARGUMENT;
    }
    status = core_machine_memory_reset_provider_resolve(ram, physical, bytes,
        &provider);
    if (status != TYPE_STATUS_OK) return status;
    return provider->read(provider->owner, physical, destination, bytes);
}
type_status core_machine_memory_write_physical(t_ram *ram, lib_u32 physical,
    type_virtual_address source, type_native_unsigned byte)
{
    lib_size offset;
    type_native_unsigned index;
    const core_machine_memory_device_provider *provider;
    lib_u32 provider_physical;
    type_status status;

    if (ram == LIB_NULL || source == 0u || byte == 0u) {
        return TYPE_STATUS_INVALID_ARGUMENT;
    }
    status = core_machine_memory_route_resolve(ram, physical, byte,
        CORE_MACHINE_MEMORY_ACCESS_WRITE, &provider, &provider_physical, &offset);
    if (status != TYPE_STATUS_OK) return status;
    if (provider == LIB_NULL && byte > 1u) {
        type_native_unsigned index;
        for (index = 0u; index < byte; ++index) {
            const core_machine_memory_device_provider *single_provider;
            lib_size single_offset;
            status = core_machine_memory_route_resolve(ram,
                physical + (lib_u32)index, 1u,
                CORE_MACHINE_MEMORY_ACCESS_WRITE, &single_provider,
                &provider_physical, &single_offset);
            if (status != TYPE_STATUS_OK) return status;
            if (single_provider != LIB_NULL) {
                for (index = 0u; index < byte; ++index) {
                    status = core_machine_memory_write_physical(ram,
                        physical + (lib_u32)index, source + index, 1u);
                    if (status != TYPE_STATUS_OK) return status;
                }
                return TYPE_STATUS_OK;
            }
        }
    }
    if (provider != LIB_NULL) {
        status = provider->write(provider->owner, provider_physical, source, byte);
        if (status != TYPE_STATUS_UNSUPPORTED) return status;
        status = core_machine_memory_offset(ram, physical, byte, &offset);
        if (status != TYPE_STATUS_OK) return status;
    }
    lib_memory_copy((C_VOID *)(ram->connect.backing + offset),
        (C_VOID *)source, byte);
    if (ram->connect.parity != 0u && offset < ram->connect.parity_bytes) {
        type_native_unsigned index;
        type_native_unsigned written = byte;
        if (written > ram->connect.parity_bytes - offset) written =
            ram->connect.parity_bytes - offset;
        for (index = 0u; index < written; ++index) ((lib_u8 *)ram->connect.parity)[offset + index] =
            core_machine_memory_odd_parity(((const lib_u8 *)source)[index]);
    }
    for (index = 0u; index < ram->connect.write_observer_count; ++index) {
        core_machine_memory_write_observer_slot *slot =
            &ram->connect.write_observers[index];
        slot->callback(slot->owner, physical, byte);
    }
    return TYPE_STATUS_OK;
}

type_status core_machine_memory_query_physical(const t_ram *ram,
    lib_u32 physical, type_native_unsigned bytes,
    core_machine_memory_access access, core_machine_memory_route *out_route)
{
    const core_machine_memory_device_provider *provider;
    lib_u32 provider_physical;
    lib_size offset;
    type_status status;

    if (out_route == LIB_NULL) return TYPE_STATUS_INVALID_ARGUMENT;
    status = core_machine_memory_route_resolve(ram, physical, bytes, access,
        &provider, &provider_physical, &offset);
    if (status != TYPE_STATUS_OK) return status;
    *out_route = provider == LIB_NULL ? CORE_MACHINE_MEMORY_ROUTE_ORDINARY_RAM :
        CORE_MACHINE_MEMORY_ROUTE_PROVIDER;
    return TYPE_STATUS_OK;
}

type_status core_machine_memory_initialize_for(t_ram *ram, lib_size bytes,
    core_machine_memory_test_allocation *test_allocation)
{
    if (ram == LIB_NULL || bytes == 0u) return TYPE_STATUS_INVALID_ARGUMENT;
    lib_memory_set((C_VOID *)ram, TYPE_ZERO_8, sizeof(*ram));
    return core_machine_memory_allocate_for_with_test(ram, bytes,
        test_allocation);
}

C_VOID core_machine_memory_reset(t_ram *ram)
{
    if (ram == LIB_NULL || ram->connect.backing == 0u) return;
    lib_memory_set((C_VOID *)&ram->data, TYPE_ZERO_8, sizeof(ram->data));
    lib_memory_set((C_VOID *)ram->connect.backing, TYPE_ZERO_8,
        ram->connect.backing_capacity);
    if (ram->connect.parity != 0u) lib_memory_set((C_VOID *)ram->connect.parity,
        TYPE_ZERO_8, ram->connect.parity_bytes);
}

C_VOID core_machine_memory_finalize(t_ram *ram)
{
    if (ram == LIB_NULL) return;
    if (ram->connect.backing != 0u) {
        lib_release((C_VOID *)ram->connect.backing);
    }
    if (ram->connect.parity != 0u) lib_release((C_VOID *)ram->connect.parity);
    lib_release(ram->connect.device_providers);
    ram->connect.device_providers = LIB_NULL;
    ram->connect.device_provider_count = 0u;
    ram->connect.device_provider_capacity = 0u;
    ram->connect.backing = 0u;
    ram->connect.installed_bytes = 0u;
    ram->connect.backing_capacity = 0u;
}

type_status core_machine_memory_set_a20_wrap_policy(t_ram *ram,
    core_machine_a20_wrap_policy policy)
{
    if (ram == LIB_NULL || ram->connect.mappings_frozen ||
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

type_status core_machine_memory_read_real_from(t_ram *ram, lib_u16 segment,
    lib_u16 offset, C_VOID *out_data, lib_size size)
{
    lib_u32 physical;

    if (ram == LIB_NULL || out_data == LIB_NULL) return TYPE_STATUS_INVALID_ARGUMENT;
    physical = core_machine_memory_wrap_a20(ram,
        (TYPE_MASK_UNSIGNED_16(segment) << 4) + TYPE_MASK_UNSIGNED_16(offset));
    return core_machine_memory_read_physical(ram, physical,
        (type_virtual_address)out_data, size);
}

type_status core_machine_memory_write_real_to(t_ram *ram, lib_u16 segment,
    lib_u16 offset, const C_VOID *in_data, lib_size size)
{
    lib_u32 physical;

    if (ram == LIB_NULL || in_data == LIB_NULL) return TYPE_STATUS_INVALID_ARGUMENT;
    physical = core_machine_memory_wrap_a20(ram,
        (TYPE_MASK_UNSIGNED_16(segment) << 4) + TYPE_MASK_UNSIGNED_16(offset));
    return core_machine_memory_write_physical(ram, physical,
        (type_virtual_address)in_data, size);
}
