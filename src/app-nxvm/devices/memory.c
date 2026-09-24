/* Copyright 2012-2014 Neko. */

/* VRAM is the random accessing memory module.  */
#include "lib/types/types_interface.h"
#include "app-nxvm/devices/device_support.h"



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

static lib_status core_machine_memory_offset(const t_ram *ram,
    lib_u32 physical, lib_size size, lib_size *out_offset)
{
    lib_size offset;
    lib_uptr index;

    if (ram == LIB_NULL || out_offset == LIB_NULL || ram->connect.backing == 0u) {
        return LIB_STATUS_INVALID_ARGUMENT;
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
        return LIB_STATUS_INTERNAL_ERROR;
    }
    *out_offset = offset;
    return LIB_STATUS_OK;
}

static lib_i32 core_machine_memory_access_is_valid(core_machine_memory_access access)
{
    return access == CORE_MACHINE_MEMORY_ACCESS_READ ||
        access == CORE_MACHINE_MEMORY_ACCESS_WRITE;
}

/* Resolve one frozen physical route.  A provider may decline a range with
 * LIB_STATUS_UNSUPPORTED so a lower registered provider or ordinary RAM owns
 * it; any other query result is terminal. */
static lib_status core_machine_memory_route_resolve(const t_ram *ram,
    lib_u32 physical, lib_uptr bytes,
    core_machine_memory_access access,
    const core_machine_memory_device_provider **out_provider,
    lib_u32 *out_provider_physical,
    lib_size *out_offset)
{
    lib_uptr index;
    lib_u32 wrapped;
    lib_status status;

    if (ram == LIB_NULL || out_provider == LIB_NULL ||
        out_provider_physical == LIB_NULL || out_offset == LIB_NULL ||
        bytes == 0u || !core_machine_memory_access_is_valid(access)) {
        return LIB_STATUS_INVALID_ARGUMENT;
    }
    for (index = 0u; index < ram->connect.device_provider_count; ++index) {
        const core_machine_memory_device_provider *provider =
            &ram->connect.device_providers[index];

        if (!provider->pre_a20 || physical < provider->physical_start ||
            (lib_u64)physical - provider->physical_start + bytes >
                provider->bytes) continue;
        status = provider->query(provider->owner, physical, bytes, access);
        if (status == LIB_STATUS_OK) {
            *out_provider = provider;
            *out_provider_physical = physical;
            return LIB_STATUS_OK;
        }
        if (status != LIB_STATUS_UNSUPPORTED) return status;
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
        return LIB_STATUS_OK;
    }
    for (index = 0u; index < ram->connect.device_provider_count; ++index) {
        const core_machine_memory_device_provider *provider =
            &ram->connect.device_providers[index];

        if (!provider->replacement || wrapped < provider->physical_start ||
            (lib_u64)wrapped - provider->physical_start + bytes >
                provider->bytes) continue;
        status = provider->query(provider->owner, wrapped, bytes, access);
        if (status == LIB_STATUS_OK) {
            *out_provider = provider;
            *out_provider_physical = wrapped;
            return LIB_STATUS_OK;
        }
        if (status != LIB_STATUS_UNSUPPORTED) return status;
    }
    for (index = 0u; index < ram->connect.device_provider_count; ++index) {
        const core_machine_memory_device_provider *provider =
            &ram->connect.device_providers[index];

        if (provider->replacement || provider->fallback ||
            wrapped < provider->physical_start ||
            (lib_u64)wrapped - provider->physical_start + bytes >
                provider->bytes) continue;
        status = provider->query(provider->owner, wrapped, bytes, access);
        if (status == LIB_STATUS_OK) {
            *out_provider = provider;
            *out_provider_physical = wrapped;
            return LIB_STATUS_OK;
        }
        if (status != LIB_STATUS_UNSUPPORTED) return status;
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
        if (status == LIB_STATUS_OK) {
            *out_provider = provider;
            *out_provider_physical = wrapped;
            return LIB_STATUS_OK;
        }
        if (status != LIB_STATUS_UNSUPPORTED) return status;
    }
    status = core_machine_memory_offset(ram, physical, bytes, out_offset);
    if (status != LIB_STATUS_OK) return status;
    *out_provider = LIB_NULL;
    *out_provider_physical = wrapped;
    return LIB_STATUS_OK;
}

/* Reset-cache fetches use the CPU's architected high reset address before the
 * board has an opportunity to apply A20 routing.  A present provider is the
 * sole ROM owner; no provider disposition leaves the caller free to use its
 * ordinary explicit backing-memory reset route. */
static lib_status core_machine_memory_reset_provider_resolve(const t_ram *ram,
    lib_u32 physical, lib_uptr bytes,
    const core_machine_memory_device_provider **out_provider)
{
    lib_uptr index;
    lib_status status;

    if (ram == LIB_NULL || out_provider == LIB_NULL || bytes == 0u) {
        return LIB_STATUS_INVALID_ARGUMENT;
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
        if (status == LIB_STATUS_OK) {
            *out_provider = provider;
            return LIB_STATUS_OK;
        }
        if (status != LIB_STATUS_UNSUPPORTED) return status;
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
        if (status == LIB_STATUS_OK) {
            *out_provider = provider;
            return LIB_STATUS_OK;
        }
        if (status != LIB_STATUS_UNSUPPORTED) return status;
    }
    return LIB_STATUS_UNSUPPORTED;
}
/* Allocates one core-owned RAM backing. Callers retain the t_ram, never backing. */
static lib_status core_machine_memory_allocate_for_with_test(t_ram *ram,
    lib_size bytes, core_machine_memory_test_allocation *test_allocation)
{
    void *backing;

    if (ram == LIB_NULL || bytes == 0u) return LIB_STATUS_INVALID_ARGUMENT;
    if (test_allocation != LIB_NULL) {
        ++test_allocation->attempts;
        if (test_allocation->fail) return LIB_STATUS_NO_MEMORY;
    }
    backing = lib_allocate_zero(1u, bytes);
    if (backing == LIB_NULL) return LIB_STATUS_NO_MEMORY;
    lib_release((void *)ram->connect.backing);
    ram->connect.backing = (lib_uptr)backing;
    ram->connect.installed_bytes = bytes;
    ram->connect.backing_capacity = bytes;
    return LIB_STATUS_OK;
}

static lib_u8 core_machine_memory_odd_parity(lib_u8 value)
{
    lib_u8 parity = 0u;
    while (value != 0u) { parity ^= value & 1u; value >>= 1u; }
    return parity;
}

lib_status core_machine_memory_allocate_for(t_ram *ram, lib_size bytes)
{
    return core_machine_memory_allocate_for_with_test(ram, bytes, LIB_NULL);
}

lib_status core_machine_memory_enable_parity(t_ram *ram, lib_size bytes,
    core_machine_memory_parity_fault_observer fault, void *owner)
{
    lib_u8 *parity;

    if (ram == LIB_NULL || fault == LIB_NULL || owner == LIB_NULL || bytes == 0u ||
        bytes > ram->connect.installed_bytes || ram->connect.mappings_frozen ||
        ram->connect.parity != 0u) return LIB_STATUS_INVALID_ARGUMENT;
    parity = (lib_u8 *)lib_allocate_zero(bytes, sizeof(*parity));
    if (parity == LIB_NULL) return LIB_STATUS_NO_MEMORY;
    ram->connect.parity = (lib_uptr)parity;
    ram->connect.parity_bytes = bytes;
    ram->connect.parity_fault = fault;
    ram->connect.parity_owner = owner;
    return LIB_STATUS_OK;
}

lib_status core_machine_memory_register_mapping(t_ram *ram,
    lib_u32 physical_start,
    lib_u32 backing_start, lib_size bytes, lib_u8 selected)
{
    core_machine_memory_mapping *mapping;

    if (ram == LIB_NULL || ram->connect.mappings_frozen ||
        (selected != LIB_FALSE && selected != LIB_TRUE) || bytes == 0u ||
        backing_start > ram->connect.installed_bytes ||
        bytes > ram->connect.installed_bytes - backing_start ||
        (lib_u64)physical_start + bytes >
            (lib_u64)LIB_UINT32_MAX + 1u) {
        return LIB_STATUS_INVALID_ARGUMENT;
    }
    if (ram->connect.mapping_count >= CORE_MACHINE_MEMORY_MAPPING_CAPACITY) {
        return LIB_STATUS_NO_MEMORY;
    }
    mapping = &ram->connect.mappings[ram->connect.mapping_count++];
    mapping->physical_start = physical_start;
    mapping->backing_start = backing_start;
    mapping->bytes = bytes;
    mapping->selected = selected;
    return LIB_STATUS_OK;
}

static lib_status core_machine_memory_validate_write_observer(const t_ram *ram,
    core_machine_memory_write_observer callback, void *owner)
{
    if (ram == LIB_NULL || callback == LIB_NULL || owner == LIB_NULL ||
        ram->connect.mappings_frozen) {
        return LIB_STATUS_INVALID_ARGUMENT;
    }
    if (ram->connect.write_observer_count >=
        CORE_MACHINE_MEMORY_WRITE_OBSERVER_CAPACITY) {
        return LIB_STATUS_NO_MEMORY;
    }
    return LIB_STATUS_OK;
}

static lib_status core_machine_memory_validate_device_provider(const t_ram *ram,
    lib_u32 physical_start, lib_size bytes,
    core_machine_memory_device_read read, core_machine_memory_device_write write,
    core_machine_memory_device_query query, void *owner, lib_u8 overlay)
{
    lib_uptr index;
    lib_u64 end;

    if (ram == LIB_NULL || bytes == 0u || read == LIB_NULL || write == LIB_NULL ||
        query == LIB_NULL || owner == LIB_NULL || ram->connect.mappings_frozen) {
        return LIB_STATUS_INVALID_ARGUMENT;
    }
    end = (lib_u64)physical_start + bytes;
    if (end > (lib_u64)LIB_UINT32_MAX + 1u ||
        ram->connect.device_provider_count >=
            CORE_MACHINE_MEMORY_DEVICE_PROVIDER_LIMIT) {
        return LIB_STATUS_NO_MEMORY;
    }
    if (!overlay) {
        for (index = 0u; index < ram->connect.device_provider_count; ++index) {
            const core_machine_memory_device_provider *existing =
                &ram->connect.device_providers[index];
            const lib_u64 existing_end =
                (lib_u64)existing->physical_start + existing->bytes;

            if (!existing->overlay && (lib_u64)physical_start < existing_end &&
                (lib_u64)existing->physical_start < end) {
                return LIB_STATUS_INVALID_ARGUMENT;
            }
        }
    }
    return LIB_STATUS_OK;
}

static lib_status core_machine_memory_reserve_device_provider(t_ram *ram)
{
    core_machine_memory_device_provider *providers;
    lib_uptr capacity;

    if (ram == LIB_NULL) return LIB_STATUS_INVALID_ARGUMENT;
    if (ram->connect.device_provider_count < ram->connect.device_provider_capacity) {
        return LIB_STATUS_OK;
    }
    if (ram->connect.device_provider_capacity >= CORE_MACHINE_MEMORY_DEVICE_PROVIDER_LIMIT) {
        return LIB_STATUS_NO_MEMORY;
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
            return LIB_STATUS_NO_MEMORY;
        }
    }
    providers = (core_machine_memory_device_provider *)lib_allocate_zero(capacity,
        sizeof(*providers));
    if (providers == LIB_NULL) return LIB_STATUS_NO_MEMORY;
    if (ram->connect.device_provider_count != 0u) {
        lib_memory_copy(providers, ram->connect.device_providers,
            ram->connect.device_provider_count * sizeof(*providers));
    }
    lib_release(ram->connect.device_providers);
    ram->connect.device_providers = providers;
    ram->connect.device_provider_capacity = capacity;
    return LIB_STATUS_OK;
}
static void core_machine_memory_append_write_observer(t_ram *ram,
    core_machine_memory_write_observer callback, void *owner)
{
    core_machine_memory_write_observer_slot *slot =
        &ram->connect.write_observers[ram->connect.write_observer_count++];

    slot->callback = callback;
    slot->owner = owner;
}

static void core_machine_memory_append_device_provider(t_ram *ram,
    lib_u32 physical_start, lib_size bytes,
    core_machine_memory_device_read read, core_machine_memory_device_write write,
    core_machine_memory_device_query query, void *owner, lib_u8 overlay,
    lib_u8 pre_a20, lib_u8 replacement, lib_u8 fallback)
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

lib_status core_machine_memory_register_write_observer(t_ram *ram,
    core_machine_memory_write_observer callback, void *owner)
{
    lib_status status = core_machine_memory_validate_write_observer(ram,
        callback, owner);

    if (status != LIB_STATUS_OK) return status;
    core_machine_memory_append_write_observer(ram, callback, owner);
    return LIB_STATUS_OK;
}

lib_status core_machine_memory_register_device_provider(t_ram *ram,
    lib_u32 physical_start, lib_size bytes,
    core_machine_memory_device_read read, core_machine_memory_device_write write,
    core_machine_memory_device_query query, void *owner)
{
    lib_status status = core_machine_memory_validate_device_provider(ram,
        physical_start, bytes, read, write, query, owner, LIB_FALSE);

    if (status != LIB_STATUS_OK) return status;
    status = core_machine_memory_reserve_device_provider(ram);
    if (status != LIB_STATUS_OK) return status;
    core_machine_memory_append_device_provider(ram, physical_start, bytes, read,
        write, query, owner, LIB_FALSE, LIB_FALSE, LIB_FALSE, LIB_FALSE);
    return LIB_STATUS_OK;
}

lib_status core_machine_memory_register_overlay_device_provider(t_ram *ram,
    lib_u32 physical_start, lib_size bytes,
    core_machine_memory_device_read read, core_machine_memory_device_write write,
    core_machine_memory_device_query query, void *owner)
{
    lib_status status = core_machine_memory_validate_device_provider(ram,
        physical_start, bytes, read, write, query, owner, LIB_TRUE);

    if (status != LIB_STATUS_OK) return status;
    status = core_machine_memory_reserve_device_provider(ram);
    if (status != LIB_STATUS_OK) return status;
    core_machine_memory_append_device_provider(ram, physical_start, bytes, read,
        write, query, owner, LIB_TRUE, LIB_FALSE, LIB_FALSE, LIB_FALSE);
    return LIB_STATUS_OK;
}

lib_status core_machine_memory_register_pre_a20_overlay_device_provider(t_ram *ram,
    lib_u32 physical_start, lib_size bytes,
    core_machine_memory_device_read read, core_machine_memory_device_write write,
    core_machine_memory_device_query query, void *owner)
{
    lib_status status = core_machine_memory_validate_device_provider(ram,
        physical_start, bytes, read, write, query, owner, LIB_TRUE);

    if (status != LIB_STATUS_OK) return status;
    status = core_machine_memory_reserve_device_provider(ram);
    if (status != LIB_STATUS_OK) return status;
    core_machine_memory_append_device_provider(ram, physical_start, bytes, read,
        write, query, owner, LIB_TRUE, LIB_TRUE, LIB_FALSE, LIB_FALSE);
    return LIB_STATUS_OK;
}

lib_status core_machine_memory_register_replacement_device_provider(t_ram *ram,
    lib_u32 physical_start, lib_size bytes,
    core_machine_memory_device_read read, core_machine_memory_device_write write,
    core_machine_memory_device_query query, void *owner)
{
    lib_status status = core_machine_memory_validate_device_provider(ram,
        physical_start, bytes, read, write, query, owner, LIB_TRUE);

    if (status != LIB_STATUS_OK) return status;
    status = core_machine_memory_reserve_device_provider(ram);
    if (status != LIB_STATUS_OK) return status;
    core_machine_memory_append_device_provider(ram, physical_start, bytes, read,
        write, query, owner, LIB_TRUE, LIB_FALSE, LIB_TRUE, LIB_FALSE);
    return LIB_STATUS_OK;
}

lib_status core_machine_memory_register_fallback_device_provider(t_ram *ram,
    lib_u32 physical_start, lib_size bytes,
    core_machine_memory_device_read read, core_machine_memory_device_write write,
    core_machine_memory_device_query query, void *owner)
{
    lib_status status = core_machine_memory_validate_device_provider(ram,
        physical_start, bytes, read, write, query, owner, LIB_TRUE);

    if (status != LIB_STATUS_OK) return status;
    status = core_machine_memory_reserve_device_provider(ram);
    if (status != LIB_STATUS_OK) return status;
    core_machine_memory_append_device_provider(ram, physical_start, bytes, read,
        write, query, owner, LIB_TRUE, LIB_FALSE, LIB_FALSE, LIB_TRUE);
    return LIB_STATUS_OK;
}

lib_status core_machine_memory_register_device_provider_and_write_observer(
    t_ram *ram, lib_u32 physical_start, lib_size bytes,
    core_machine_memory_device_read read, core_machine_memory_device_write write,
    core_machine_memory_device_query query, void *owner,
    core_machine_memory_write_observer callback)
{
    lib_status status = core_machine_memory_validate_device_provider(ram,
        physical_start, bytes, read, write, query, owner, LIB_FALSE);

    if (status != LIB_STATUS_OK) return status;
    status = core_machine_memory_validate_write_observer(ram, callback, owner);
    if (status != LIB_STATUS_OK) return status;
    status = core_machine_memory_reserve_device_provider(ram);
    if (status != LIB_STATUS_OK) return status;
    core_machine_memory_append_device_provider(ram, physical_start, bytes, read,
        write, query, owner, LIB_FALSE, LIB_FALSE, LIB_FALSE, LIB_FALSE);
    core_machine_memory_append_write_observer(ram, callback, owner);
    return LIB_STATUS_OK;
}
void core_machine_memory_freeze_mappings(t_ram *ram)
{
    if (ram != LIB_NULL) ram->connect.mappings_frozen = LIB_TRUE;
}
static void core_machine_memory_read_a20(t_port *port, lib_u16 port_id,
    void *owner)
{
    t_ram *ram = (t_ram *)owner;

    (void)port_id;
    if (ram == LIB_NULL) return;
    port->data.ioByte = ram->data.flagA20 ? VRAM_FLAG_A20 : 0u;
}
static void core_machine_memory_write_a20(t_port *port, lib_u16 port_id,
    void *owner)
{
    t_ram *ram = (t_ram *)owner;

    (void)port_id;
    if (ram == LIB_NULL) return;
    ram->data.flagA20 = CORE_MACHINE_BIT_IS_SET(port->data.ioByte, VRAM_FLAG_A20);
}

lib_status core_machine_memory_read_physical(t_ram *ram, lib_u32 physical,
    lib_uptr destination, lib_uptr byte)
{
    lib_size offset;
    const core_machine_memory_device_provider *provider;
    lib_u32 provider_physical;
    lib_status status;

    if (ram == LIB_NULL || destination == 0u || byte == 0u) {
        return LIB_STATUS_INVALID_ARGUMENT;
    }
    status = core_machine_memory_route_resolve(ram, physical, byte,
        CORE_MACHINE_MEMORY_ACCESS_READ, &provider, &provider_physical, &offset);
    if (status != LIB_STATUS_OK) return status;
    if (provider == LIB_NULL && byte > 1u) {
        lib_uptr index;
        for (index = 0u; index < byte; ++index) {
            const core_machine_memory_device_provider *single_provider;
            lib_size single_offset;
            status = core_machine_memory_route_resolve(ram,
                physical + (lib_u32)index, 1u,
                CORE_MACHINE_MEMORY_ACCESS_READ, &single_provider,
                &provider_physical, &single_offset);
            if (status != LIB_STATUS_OK) return status;
            if (single_provider != LIB_NULL) {
                for (index = 0u; index < byte; ++index) {
                    status = core_machine_memory_read_physical(ram,
                        physical + (lib_u32)index, destination + index, 1u);
                    if (status != LIB_STATUS_OK) return status;
                }
                return LIB_STATUS_OK;
            }
        }
    }
    if (provider != LIB_NULL) {
        status = provider->read(provider->owner, provider_physical, destination, byte);
        if (status != LIB_STATUS_UNSUPPORTED) return status;
        status = core_machine_memory_offset(ram, physical, byte, &offset);
        if (status != LIB_STATUS_OK) return status;
    }
    lib_memory_copy((void *)destination,
        (void *)(ram->connect.backing + offset), byte);
    if (ram->connect.parity != 0u && offset < ram->connect.parity_bytes) {
        lib_uptr index;
        lib_uptr checked = byte;
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
    return LIB_STATUS_OK;
}

lib_status core_machine_memory_read_reset_physical(t_ram *ram,
    lib_u32 physical, lib_uptr destination,
    lib_uptr bytes)
{
    const core_machine_memory_device_provider *provider;
    lib_status status;

    if (ram == LIB_NULL || destination == 0u || bytes == 0u) {
        return LIB_STATUS_INVALID_ARGUMENT;
    }
    status = core_machine_memory_reset_provider_resolve(ram, physical, bytes,
        &provider);
    if (status != LIB_STATUS_OK) return status;
    return provider->read(provider->owner, physical, destination, bytes);
}
lib_status core_machine_memory_write_physical(t_ram *ram, lib_u32 physical,
    lib_uptr source, lib_uptr byte)
{
    lib_size offset;
    lib_uptr index;
    const core_machine_memory_device_provider *provider;
    lib_u32 provider_physical;
    lib_status status;

    if (ram == LIB_NULL || source == 0u || byte == 0u) {
        return LIB_STATUS_INVALID_ARGUMENT;
    }
    status = core_machine_memory_route_resolve(ram, physical, byte,
        CORE_MACHINE_MEMORY_ACCESS_WRITE, &provider, &provider_physical, &offset);
    if (status != LIB_STATUS_OK) return status;
    if (provider == LIB_NULL && byte > 1u) {
        lib_uptr index;
        for (index = 0u; index < byte; ++index) {
            const core_machine_memory_device_provider *single_provider;
            lib_size single_offset;
            status = core_machine_memory_route_resolve(ram,
                physical + (lib_u32)index, 1u,
                CORE_MACHINE_MEMORY_ACCESS_WRITE, &single_provider,
                &provider_physical, &single_offset);
            if (status != LIB_STATUS_OK) return status;
            if (single_provider != LIB_NULL) {
                for (index = 0u; index < byte; ++index) {
                    status = core_machine_memory_write_physical(ram,
                        physical + (lib_u32)index, source + index, 1u);
                    if (status != LIB_STATUS_OK) return status;
                }
                return LIB_STATUS_OK;
            }
        }
    }
    if (provider != LIB_NULL) {
        status = provider->write(provider->owner, provider_physical, source, byte);
        if (status != LIB_STATUS_UNSUPPORTED) return status;
        status = core_machine_memory_offset(ram, physical, byte, &offset);
        if (status != LIB_STATUS_OK) return status;
    }
    lib_memory_copy((void *)(ram->connect.backing + offset),
        (void *)source, byte);
    if (ram->connect.parity != 0u && offset < ram->connect.parity_bytes) {
        lib_uptr index;
        lib_uptr written = byte;
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
    return LIB_STATUS_OK;
}

lib_status core_machine_memory_query_physical(const t_ram *ram,
    lib_u32 physical, lib_uptr bytes,
    core_machine_memory_access access, core_machine_memory_route *out_route)
{
    const core_machine_memory_device_provider *provider;
    lib_u32 provider_physical;
    lib_size offset;
    lib_status status;

    if (out_route == LIB_NULL) return LIB_STATUS_INVALID_ARGUMENT;
    status = core_machine_memory_route_resolve(ram, physical, bytes, access,
        &provider, &provider_physical, &offset);
    if (status != LIB_STATUS_OK) return status;
    *out_route = provider == LIB_NULL ? CORE_MACHINE_MEMORY_ROUTE_ORDINARY_RAM :
        CORE_MACHINE_MEMORY_ROUTE_PROVIDER;
    return LIB_STATUS_OK;
}

lib_status core_machine_memory_initialize_for(t_ram *ram, lib_size bytes,
    core_machine_memory_test_allocation *test_allocation)
{
    if (ram == LIB_NULL || bytes == 0u) return LIB_STATUS_INVALID_ARGUMENT;
    lib_memory_set((void *)ram, 0u, sizeof(*ram));
    return core_machine_memory_allocate_for_with_test(ram, bytes,
        test_allocation);
}

void core_machine_memory_reset(t_ram *ram)
{
    if (ram == LIB_NULL || ram->connect.backing == 0u) return;
    lib_memory_set((void *)&ram->data, 0u, sizeof(ram->data));
    lib_memory_set((void *)ram->connect.backing, 0u,
        ram->connect.backing_capacity);
    if (ram->connect.parity != 0u) lib_memory_set((void *)ram->connect.parity,
        0u, ram->connect.parity_bytes);
}

void core_machine_memory_finalize(t_ram *ram)
{
    if (ram == LIB_NULL) return;
    if (ram->connect.backing != 0u) {
        lib_release((void *)ram->connect.backing);
    }
    if (ram->connect.parity != 0u) lib_release((void *)ram->connect.parity);
    lib_release(ram->connect.device_providers);
    ram->connect.device_providers = LIB_NULL;
    ram->connect.device_provider_count = 0u;
    ram->connect.device_provider_capacity = 0u;
    ram->connect.backing = 0u;
    ram->connect.installed_bytes = 0u;
    ram->connect.backing_capacity = 0u;
}

lib_status core_machine_memory_set_a20_wrap_policy(t_ram *ram,
    core_machine_a20_wrap_policy policy)
{
    if (ram == LIB_NULL || ram->connect.mappings_frozen ||
        (policy != CORE_MACHINE_A20_WRAP_GLOBAL_MASK &&
        policy != CORE_MACHINE_A20_WRAP_FIRST_TO_SECOND_MIB)) {
        return LIB_STATUS_INVALID_ARGUMENT;
    }
    ram->connect.a20_wrap_policy = policy;
    return LIB_STATUS_OK;
}
void core_machine_memory_register_ports(t_ram *ram, t_port *port)
{
    core_machine_port_add_read(port, 0x0092,
        core_machine_memory_read_a20, ram);
    core_machine_port_add_write(port, 0x0092,
        core_machine_memory_write_a20, ram);
}

lib_status core_machine_memory_read_real_from(t_ram *ram, lib_u16 segment,
    lib_u16 offset, void *out_data, lib_size size)
{
    lib_u32 physical;

    if (ram == LIB_NULL || out_data == LIB_NULL) return LIB_STATUS_INVALID_ARGUMENT;
    physical = core_machine_memory_wrap_a20(ram,
        (CORE_MACHINE_MASK_U16(segment) << 4) + CORE_MACHINE_MASK_U16(offset));
    return core_machine_memory_read_physical(ram, physical,
        (lib_uptr)out_data, size);
}

lib_status core_machine_memory_write_real_to(t_ram *ram, lib_u16 segment,
    lib_u16 offset, const void *in_data, lib_size size)
{
    lib_u32 physical;

    if (ram == LIB_NULL || in_data == LIB_NULL) return LIB_STATUS_INVALID_ARGUMENT;
    physical = core_machine_memory_wrap_a20(ram,
        (CORE_MACHINE_MASK_U16(segment) << 4) + CORE_MACHINE_MASK_U16(offset));
    return core_machine_memory_write_physical(ram, physical,
        (lib_uptr)in_data, size);
}
