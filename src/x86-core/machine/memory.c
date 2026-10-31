/* Copyright 2012-2014 Neko. */

#include "lib/types/types_interface.h"



#include "x86-core/machine/memory.h"

static lib_u32 x86_core_machine_memory_decode_address(const t_ram *ram,
    lib_u32 offset)
{
    if (ram->data.address_clear_mask == 0u ||
        offset < ram->connect.address_decode.first ||
        offset > ram->connect.address_decode.last) return offset;
    return offset & ~ram->data.address_clear_mask;
}

static lib_status x86_core_machine_memory_offset(const t_ram *ram,
    lib_u32 physical, lib_u32 decoded, lib_size size, lib_size *out_offset)
{
    lib_size offset;
    lib_uptr index;

    if (ram == LIB_NULL || out_offset == LIB_NULL || ram->connect.backing == 0u) {
        return LIB_STATUS_INVALID_ARGUMENT;
    }
    offset = (lib_size)decoded;
    for (index = 0u; index < ram->connect.mapping_count; ++index) {
        const x86_core_machine_memory_mapping *mapping = &ram->connect.mappings[index];
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

static lib_i32 x86_core_machine_memory_access_is_valid(x86_core_machine_memory_access access)
{
    return access == X86_CORE_MACHINE_MEMORY_ACCESS_READ ||
        access == X86_CORE_MACHINE_MEMORY_ACCESS_WRITE;
}

static void x86_core_machine_memory_note_provider_failure(
    const x86_core_machine_memory_device_provider *provider,
    lib_u32 physical, lib_uptr bytes, x86_core_machine_memory_access access,
    lib_status status)
{
    if (provider == LIB_NULL || provider->failure == LIB_NULL ||
        status == LIB_STATUS_OK || status == LIB_STATUS_UNSUPPORTED) return;
    provider->failure(provider->owner, physical, bytes, access, status);
}

/* Resolve one frozen physical route.  A provider may decline a range with
 * LIB_STATUS_UNSUPPORTED so a lower registered provider or ordinary RAM owns
 * it; any other query result is terminal. */
static lib_status x86_core_machine_memory_route_resolve(const t_ram *ram,
    lib_u32 physical, lib_uptr bytes,
    x86_core_machine_memory_access access, lib_bool notify_failure,
    const x86_core_machine_memory_device_provider **out_provider,
    lib_u32 *out_provider_physical,
    lib_size *out_offset, lib_bool cpu_access)
{
    lib_uptr index;
    lib_u32 wrapped;
    lib_status status;

    if (ram == LIB_NULL || out_provider == LIB_NULL ||
        out_provider_physical == LIB_NULL || out_offset == LIB_NULL ||
        bytes == 0u || !x86_core_machine_memory_access_is_valid(access) ||
        (notify_failure != LIB_FALSE && notify_failure != LIB_TRUE) ||
        (cpu_access != LIB_FALSE && cpu_access != LIB_TRUE)) {
        return LIB_STATUS_INVALID_ARGUMENT;
    }
    for (index = 0u; index < ram->connect.device_provider_count; ++index) {
        const x86_core_machine_memory_device_provider *provider =
            &ram->connect.device_providers[index];

        if (!provider->raw_decode || (provider->cpu_only && !cpu_access) ||
            physical < provider->physical_start ||
            (lib_u64)physical - provider->physical_start + bytes >
                provider->bytes) continue;
        status = provider->query(provider->owner, physical, bytes, access);
        if (status == LIB_STATUS_OK) {
            *out_provider = provider;
            *out_provider_physical = physical;
            return LIB_STATUS_OK;
        }
        if (status != LIB_STATUS_UNSUPPORTED) {
            if (notify_failure)
                x86_core_machine_memory_note_provider_failure(provider, physical,
                    bytes, access, status);
            return status;
        }
    }
    wrapped = x86_core_machine_memory_decode_address(ram, physical);
    /* A frozen board RAM alias is a selected physical decode, not an ordinary
     * backing fallback.  It therefore wins over any lower ordinary device
     * route while preserving the raw reset route above. */
    for (index = 0u; index < ram->connect.mapping_count; ++index) {
        const x86_core_machine_memory_mapping *mapping = &ram->connect.mappings[index];

        if (!mapping->selected || physical < mapping->physical_start ||
            (lib_u64)physical - mapping->physical_start + bytes >
                mapping->bytes) continue;
        *out_offset = (lib_size)mapping->backing_start +
            (lib_size)((lib_u64)physical - mapping->physical_start);
        *out_provider = LIB_NULL;
        *out_provider_physical = wrapped;
        return LIB_STATUS_OK;
    }
    /* Match the frozen windows once, then retain tier and registration order. */
    lib_u8 matches[X86_CORE_MACHINE_MEMORY_DEVICE_PROVIDER_LIMIT];
    lib_uptr match_count = 0u;
    for (index = 0u; index < ram->connect.device_provider_count; ++index) {
        const x86_core_machine_memory_device_provider *provider =
            &ram->connect.device_providers[index];
        if ((provider->cpu_only && !cpu_access) || wrapped < provider->physical_start ||
            (lib_u64)wrapped - provider->physical_start + bytes > provider->bytes) continue;
        matches[match_count++] = (lib_u8)index;
    }
    /* Replacement, ordinary, then fallback; preserve registration order. */
    for (lib_u8 tier = 0u; tier < 3u; ++tier) {
        for (index = 0u; index < match_count; ++index) {
            const x86_core_machine_memory_device_provider *provider =
                &ram->connect.device_providers[matches[index]];
            const lib_u8 provider_tier = provider->replacement ? 0u :
                provider->fallback ? 2u : 1u;

            if (provider_tier != tier) continue;
            status = provider->query(provider->owner, wrapped, bytes, access);
            if (status == LIB_STATUS_OK) {
                *out_provider = provider;
                *out_provider_physical = wrapped;
                return LIB_STATUS_OK;
            }
            if (status != LIB_STATUS_UNSUPPORTED) {
                if (notify_failure)
                    x86_core_machine_memory_note_provider_failure(provider, wrapped,
                        bytes, access, status);
                return status;
            }
        }
    }
    status = x86_core_machine_memory_offset(ram, physical, wrapped, bytes, out_offset);
    if (status != LIB_STATUS_OK) return status;
    *out_provider = LIB_NULL;
    *out_provider_physical = wrapped;
    return LIB_STATUS_OK;
}

/* A conservative geometric proof: no aliases or address-line changes, and
 * no provider intersects any byte. Otherwise use the existing byte queries. */
static lib_bool x86_core_machine_memory_is_plain_span(const t_ram *ram, lib_u32 physical, lib_uptr bytes)
{
    if (ram->data.address_clear_mask != 0u || ram->connect.mapping_count != 0u ||
        bytes > (lib_u64)LIB_UINT32_MAX + 1u - physical) return LIB_FALSE;
    for (lib_uptr index = 0u; index < ram->connect.device_provider_count; ++index) {
        const x86_core_machine_memory_device_provider *provider =
            &ram->connect.device_providers[index];
        if ((lib_u64)physical < (lib_u64)provider->physical_start + provider->bytes &&
            (lib_u64)provider->physical_start < (lib_u64)physical + bytes)
            return LIB_FALSE;
    }
    return LIB_TRUE;
}

/* Reset-cache fetches use the CPU's architected high reset address before the
 * board has an opportunity to apply address-line decoding.  A present provider is the
 * sole ROM owner; no provider disposition leaves the caller free to use its
 * ordinary explicit backing-memory reset route. */
static lib_status x86_core_machine_memory_reset_provider_resolve(const t_ram *ram,
    lib_u32 physical, lib_uptr bytes,
    const x86_core_machine_memory_device_provider **out_provider)
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
        const x86_core_machine_memory_device_provider *provider =
            &ram->connect.device_providers[index];

        if (provider->fallback || !provider->overlay ||
            physical < provider->physical_start ||
            (lib_u64)physical - provider->physical_start + bytes >
                provider->bytes) continue;
        status = provider->query(provider->owner, physical, bytes,
            X86_CORE_MACHINE_MEMORY_ACCESS_READ);
        if (status == LIB_STATUS_OK) {
            *out_provider = provider;
            return LIB_STATUS_OK;
        }
        if (status != LIB_STATUS_UNSUPPORTED) {
            x86_core_machine_memory_note_provider_failure(provider, physical, bytes,
                X86_CORE_MACHINE_MEMORY_ACCESS_READ, status);
            return status;
        }
    }
    for (index = 0u; index < ram->connect.device_provider_count; ++index) {
        const x86_core_machine_memory_device_provider *provider =
            &ram->connect.device_providers[index];

        if (provider->fallback || provider->overlay ||
            physical < provider->physical_start ||
            (lib_u64)physical - provider->physical_start + bytes >
                provider->bytes) continue;
        status = provider->query(provider->owner, physical, bytes,
            X86_CORE_MACHINE_MEMORY_ACCESS_READ);
        if (status == LIB_STATUS_OK) {
            *out_provider = provider;
            return LIB_STATUS_OK;
        }
        if (status != LIB_STATUS_UNSUPPORTED) {
            x86_core_machine_memory_note_provider_failure(provider, physical, bytes,
                X86_CORE_MACHINE_MEMORY_ACCESS_READ, status);
            return status;
        }
    }
    return LIB_STATUS_UNSUPPORTED;
}
/* Allocates one core-owned RAM backing. Callers retain the t_ram, never backing. */
static lib_status x86_core_machine_memory_allocate_for_with_test(t_ram *ram,
    lib_size bytes, x86_core_machine_memory_test_allocation *test_allocation)
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

static lib_u8 x86_core_machine_memory_odd_parity(lib_u8 value)
{
    lib_u8 parity = 0u;
    while (value != 0u) { parity ^= value & 1u; value >>= 1u; }
    return parity;
}

lib_status x86_core_machine_memory_allocate_for(t_ram *ram, lib_size bytes)
{
    return x86_core_machine_memory_allocate_for_with_test(ram, bytes, LIB_NULL);
}

lib_status x86_core_machine_memory_enable_parity(t_ram *ram, lib_size bytes,
    x86_core_machine_memory_parity_fault_observer fault, void *owner)
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

void x86_core_machine_memory_release_parity(t_ram *ram)
{
    if (ram == LIB_NULL) return;
    if (ram->connect.parity != 0u) lib_release((void *)ram->connect.parity);
    ram->connect.parity = 0u;
    ram->connect.parity_bytes = 0u;
    ram->connect.parity_fault = LIB_NULL;
    ram->connect.parity_owner = LIB_NULL;
}

lib_status x86_core_machine_memory_register_mapping(t_ram *ram,
    lib_u32 physical_start,
    lib_u32 backing_start, lib_size bytes, lib_u8 selected)
{
    x86_core_machine_memory_mapping *mapping;

    if (ram == LIB_NULL || ram->connect.mappings_frozen ||
        (selected != LIB_FALSE && selected != LIB_TRUE) || bytes == 0u ||
        backing_start > ram->connect.installed_bytes ||
        bytes > ram->connect.installed_bytes - backing_start ||
        (lib_u64)physical_start + bytes >
            (lib_u64)LIB_UINT32_MAX + 1u) {
        return LIB_STATUS_INVALID_ARGUMENT;
    }
    if (ram->connect.mapping_count >= X86_CORE_MACHINE_MEMORY_MAPPING_CAPACITY) {
        return LIB_STATUS_NO_MEMORY;
    }
    mapping = &ram->connect.mappings[ram->connect.mapping_count++];
    mapping->physical_start = physical_start;
    mapping->backing_start = backing_start;
    mapping->bytes = bytes;
    mapping->selected = selected;
    mapping->writable = LIB_TRUE;
    return LIB_STATUS_OK;
}

static lib_status x86_core_machine_memory_validate_write_observer(const t_ram *ram,
    x86_core_machine_memory_write_observer callback, void *owner)
{
    if (ram == LIB_NULL || callback == LIB_NULL || owner == LIB_NULL ||
        ram->connect.mappings_frozen) {
        return LIB_STATUS_INVALID_ARGUMENT;
    }
    if (ram->connect.write_observer_count >=
        X86_CORE_MACHINE_MEMORY_WRITE_OBSERVER_CAPACITY) {
        return LIB_STATUS_NO_MEMORY;
    }
    return LIB_STATUS_OK;
}

static lib_status x86_core_machine_memory_validate_device_provider(const t_ram *ram,
    lib_u32 physical_start, lib_size bytes,
    x86_core_machine_memory_device_read read, x86_core_machine_memory_device_write write,
    x86_core_machine_memory_device_query query, void *owner, lib_u8 overlay)
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
            X86_CORE_MACHINE_MEMORY_DEVICE_PROVIDER_LIMIT) {
        return LIB_STATUS_NO_MEMORY;
    }
    if (!overlay) {
        for (index = 0u; index < ram->connect.device_provider_count; ++index) {
            const x86_core_machine_memory_device_provider *existing =
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

static lib_status x86_core_machine_memory_reserve_device_provider(t_ram *ram)
{
    x86_core_machine_memory_device_provider *providers;
    lib_uptr capacity;

    if (ram == LIB_NULL) return LIB_STATUS_INVALID_ARGUMENT;
    if (ram->connect.device_provider_count < ram->connect.device_provider_capacity) {
        return LIB_STATUS_OK;
    }
    if (ram->connect.device_provider_capacity >= X86_CORE_MACHINE_MEMORY_DEVICE_PROVIDER_LIMIT) {
        return LIB_STATUS_NO_MEMORY;
    }
    capacity = ram->connect.device_provider_capacity == 0u ?
        X86_CORE_MACHINE_MEMORY_DEVICE_PROVIDER_INITIAL_CAPACITY :
        ram->connect.device_provider_capacity * 2u;
    if (capacity > X86_CORE_MACHINE_MEMORY_DEVICE_PROVIDER_LIMIT) {
        capacity = X86_CORE_MACHINE_MEMORY_DEVICE_PROVIDER_LIMIT;
    }
    if (ram->connect.device_provider_test_allocation != LIB_NULL) {
        ++ram->connect.device_provider_test_allocation->attempts;
        if (ram->connect.device_provider_test_allocation->fail) {
            return LIB_STATUS_NO_MEMORY;
        }
    }
    providers = (x86_core_machine_memory_device_provider *)lib_allocate_zero(capacity,
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
static void x86_core_machine_memory_append_write_observer(t_ram *ram,
    x86_core_machine_memory_write_observer callback, void *owner)
{
    x86_core_machine_memory_write_observer_slot *slot =
        &ram->connect.write_observers[ram->connect.write_observer_count++];

    slot->callback = callback;
    slot->owner = owner;
}

static void x86_core_machine_memory_append_device_provider(t_ram *ram,
    lib_u32 physical_start, lib_size bytes,
    x86_core_machine_memory_device_read read, x86_core_machine_memory_device_write write,
    x86_core_machine_memory_device_query query, void *owner,
    x86_core_machine_memory_device_failure_observer failure, lib_u8 overlay,
    lib_u8 raw_decode, lib_u8 replacement, lib_u8 fallback)
{
    x86_core_machine_memory_device_provider *provider =
        &ram->connect.device_providers[ram->connect.device_provider_count++];

    provider->physical_start = physical_start;
    provider->bytes = bytes;
    provider->read = read;
    provider->read_instruction = LIB_NULL;
    provider->cache_probe = LIB_NULL;
    provider->write = write;
    provider->query = query;
    provider->failure = failure;
    provider->owner = owner;
    provider->overlay = overlay;
    provider->raw_decode = raw_decode;
    provider->cpu_only = LIB_FALSE;
    provider->replacement = replacement;
    provider->fallback = fallback;
}

lib_status x86_core_machine_memory_register_write_observer(t_ram *ram,
    x86_core_machine_memory_write_observer callback, void *owner)
{
    lib_status status = x86_core_machine_memory_validate_write_observer(ram,
        callback, owner);

    if (status != LIB_STATUS_OK) return status;
    x86_core_machine_memory_append_write_observer(ram, callback, owner);
    return LIB_STATUS_OK;
}

lib_status x86_core_machine_memory_register_device_provider(t_ram *ram,
    lib_u32 physical_start, lib_size bytes,
    x86_core_machine_memory_device_read read, x86_core_machine_memory_device_write write,
    x86_core_machine_memory_device_query query, void *owner,
    x86_core_machine_memory_device_failure_observer failure)
{
    lib_status status = x86_core_machine_memory_validate_device_provider(ram,
        physical_start, bytes, read, write, query, owner, LIB_FALSE);

    if (status != LIB_STATUS_OK) return status;
    status = x86_core_machine_memory_reserve_device_provider(ram);
    if (status != LIB_STATUS_OK) return status;
    x86_core_machine_memory_append_device_provider(ram, physical_start, bytes, read,
        write, query, owner, failure, LIB_FALSE, LIB_FALSE, LIB_FALSE, LIB_FALSE);
    return LIB_STATUS_OK;
}

lib_status x86_core_machine_memory_register_overlay_device_provider(t_ram *ram,
    lib_u32 physical_start, lib_size bytes,
    x86_core_machine_memory_device_read read, x86_core_machine_memory_device_write write,
    x86_core_machine_memory_device_query query, void *owner,
    x86_core_machine_memory_device_failure_observer failure)
{
    lib_status status = x86_core_machine_memory_validate_device_provider(ram,
        physical_start, bytes, read, write, query, owner, LIB_TRUE);

    if (status != LIB_STATUS_OK) return status;
    status = x86_core_machine_memory_reserve_device_provider(ram);
    if (status != LIB_STATUS_OK) return status;
    x86_core_machine_memory_append_device_provider(ram, physical_start, bytes, read,
        write, query, owner, failure, LIB_TRUE, LIB_FALSE, LIB_FALSE, LIB_FALSE);
    return LIB_STATUS_OK;
}

lib_status x86_core_machine_memory_register_raw_overlay_device_provider(t_ram *ram,
    lib_u32 physical_start, lib_size bytes,
    x86_core_machine_memory_device_read read, x86_core_machine_memory_device_write write,
    x86_core_machine_memory_device_query query, void *owner,
    x86_core_machine_memory_device_failure_observer failure)
{
    lib_status status = x86_core_machine_memory_validate_device_provider(ram,
        physical_start, bytes, read, write, query, owner, LIB_TRUE);

    if (status != LIB_STATUS_OK) return status;
    status = x86_core_machine_memory_reserve_device_provider(ram);
    if (status != LIB_STATUS_OK) return status;
    x86_core_machine_memory_append_device_provider(ram, physical_start, bytes, read,
        write, query, owner, failure, LIB_TRUE, LIB_TRUE, LIB_FALSE, LIB_FALSE);
    return LIB_STATUS_OK;
}

lib_status x86_core_machine_memory_register_replacement_device_provider(t_ram *ram,
    lib_u32 physical_start, lib_size bytes,
    x86_core_machine_memory_device_read read, x86_core_machine_memory_device_write write,
    x86_core_machine_memory_device_query query, void *owner,
    x86_core_machine_memory_device_failure_observer failure)
{
    lib_status status = x86_core_machine_memory_validate_device_provider(ram,
        physical_start, bytes, read, write, query, owner, LIB_TRUE);

    if (status != LIB_STATUS_OK) return status;
    status = x86_core_machine_memory_reserve_device_provider(ram);
    if (status != LIB_STATUS_OK) return status;
    x86_core_machine_memory_append_device_provider(ram, physical_start, bytes, read,
        write, query, owner, failure, LIB_TRUE, LIB_FALSE, LIB_TRUE, LIB_FALSE);
    return LIB_STATUS_OK;
}

lib_status x86_core_machine_memory_register_fallback_device_provider(t_ram *ram,
    lib_u32 physical_start, lib_size bytes,
    x86_core_machine_memory_device_read read, x86_core_machine_memory_device_write write,
    x86_core_machine_memory_device_query query, void *owner,
    x86_core_machine_memory_device_failure_observer failure)
{
    lib_status status = x86_core_machine_memory_validate_device_provider(ram,
        physical_start, bytes, read, write, query, owner, LIB_TRUE);

    if (status != LIB_STATUS_OK) return status;
    status = x86_core_machine_memory_reserve_device_provider(ram);
    if (status != LIB_STATUS_OK) return status;
    x86_core_machine_memory_append_device_provider(ram, physical_start, bytes, read,
        write, query, owner, failure, LIB_TRUE, LIB_FALSE, LIB_FALSE, LIB_TRUE);
    return LIB_STATUS_OK;
}

void x86_core_machine_memory_freeze_mappings(t_ram *ram)
{
    if (ram != LIB_NULL) ram->connect.mappings_frozen = LIB_TRUE;
}
void x86_core_machine_memory_unregister_owner(t_ram *ram, const void *owner)
{
    lib_size retained = 0u;

    if (ram == LIB_NULL || owner == LIB_NULL) return;
    for (lib_size index = 0u; index < ram->connect.device_provider_count; ++index) {
        if (ram->connect.device_providers[index].owner != owner)
            ram->connect.device_providers[retained++] = ram->connect.device_providers[index];
    }
    ram->connect.device_provider_count = retained;
    retained = 0u;
    for (lib_size index = 0u; index < ram->connect.write_observer_count; ++index) {
        if (ram->connect.write_observers[index].owner != owner)
            ram->connect.write_observers[retained++] = ram->connect.write_observers[index];
    }
    ram->connect.write_observer_count = retained;
}
static lib_status x86_core_machine_memory_read_routed(t_ram *ram, lib_u32 physical,
    lib_uptr destination, lib_uptr byte, lib_bool observe_only,
    lib_bool instruction_fetch, lib_bool cpu_access)
{
    lib_size offset;
    const x86_core_machine_memory_device_provider *provider;
    lib_u32 provider_physical;
    lib_status status;

    if (ram == LIB_NULL || destination == 0u || byte == 0u) {
        return LIB_STATUS_INVALID_ARGUMENT;
    }
    status = x86_core_machine_memory_route_resolve(ram, physical, byte,
        X86_CORE_MACHINE_MEMORY_ACCESS_READ, LIB_TRUE, &provider,
        &provider_physical, &offset, cpu_access);
    if (status != LIB_STATUS_OK) return status;
    if (provider == LIB_NULL && byte > 1u && !x86_core_machine_memory_is_plain_span(ram, physical, byte)) {
        lib_uptr index;
        for (index = 0u; index < byte; ++index) {
            const x86_core_machine_memory_device_provider *single_provider;
            lib_size single_offset;
            status = x86_core_machine_memory_route_resolve(ram,
                physical + (lib_u32)index, 1u,
                X86_CORE_MACHINE_MEMORY_ACCESS_READ, LIB_TRUE, &single_provider,
                &provider_physical, &single_offset, cpu_access);
            if (status != LIB_STATUS_OK) return status;
            if (single_provider != LIB_NULL) {
                for (index = 0u; index < byte; ++index) {
                    status = x86_core_machine_memory_read_routed(ram,
                        physical + (lib_u32)index, destination + index, 1u,
                        observe_only, instruction_fetch, cpu_access);
                    if (status != LIB_STATUS_OK) return status;
                }
                return LIB_STATUS_OK;
            }
        }
    }
    if (provider != LIB_NULL) {
        x86_core_machine_memory_device_read read = instruction_fetch &&
            provider->read_instruction != LIB_NULL ? provider->read_instruction : provider->read;

        status = read(provider->owner, provider_physical, destination, byte,
            observe_only);
        if (status == LIB_STATUS_UNSUPPORTED && read != provider->read) {
            status = provider->read(provider->owner, provider_physical, destination,
                byte, observe_only);
        }
        if (status != LIB_STATUS_UNSUPPORTED) {
            x86_core_machine_memory_note_provider_failure(provider, provider_physical,
                byte, X86_CORE_MACHINE_MEMORY_ACCESS_READ, status);
            return status;
        }
        status = x86_core_machine_memory_offset(ram, physical,
            x86_core_machine_memory_decode_address(ram, physical), byte, &offset);
        if (status != LIB_STATUS_OK) return status;
    }
    lib_memory_copy((void *)destination,
        (void *)(ram->connect.backing + offset), byte);
    if (!observe_only && ram->connect.parity != 0u &&
        offset < ram->connect.parity_bytes) {
        lib_uptr index;
        lib_uptr checked = byte;
        if (checked > ram->connect.parity_bytes - offset) checked =
            ram->connect.parity_bytes - offset;
        for (index = 0u; index < checked; ++index) {
            if (((lib_u8 *)ram->connect.parity)[offset + index] !=
                x86_core_machine_memory_odd_parity(((lib_u8 *)destination)[index])) {
                ram->connect.parity_fault(ram->connect.parity_owner,
                    physical + (lib_u32)index);
                break;
            }
        }
    }
    return LIB_STATUS_OK;
}

lib_status x86_core_machine_memory_read_physical(t_ram *ram, lib_u32 physical,
    lib_uptr destination, lib_uptr bytes, lib_bool cpu_access)
{
    return x86_core_machine_memory_read_routed(ram, physical, destination, bytes,
        LIB_FALSE, LIB_FALSE, cpu_access);
}

lib_status x86_core_machine_memory_read_instruction_physical(t_ram *ram,
    lib_u32 physical, lib_uptr destination, lib_uptr bytes)
{
    return x86_core_machine_memory_read_routed(ram, physical, destination, bytes,
        LIB_FALSE, LIB_TRUE, LIB_TRUE);
}

lib_status x86_core_machine_memory_inspect_physical(t_ram *ram, lib_u32 physical,
    lib_uptr destination, lib_uptr bytes, lib_bool reset_fetch)
{
    const x86_core_machine_memory_device_provider *provider;
    lib_status status;

    if (ram == LIB_NULL || destination == 0u || bytes == 0u)
        return LIB_STATUS_INVALID_ARGUMENT;
    if (reset_fetch) {
        status = x86_core_machine_memory_reset_provider_resolve(ram, physical,
            bytes, &provider);
        if (status == LIB_STATUS_OK) {
            status = provider->read(provider->owner, physical, destination,
                bytes, LIB_TRUE);
            if (status != LIB_STATUS_UNSUPPORTED)
                x86_core_machine_memory_note_provider_failure(provider, physical, bytes,
                    X86_CORE_MACHINE_MEMORY_ACCESS_READ, status);
        }
        if (status != LIB_STATUS_UNSUPPORTED) return status;
    }
    return x86_core_machine_memory_read_routed(ram, physical, destination, bytes,
        LIB_TRUE, LIB_FALSE, LIB_TRUE);
}

lib_status x86_core_machine_memory_read_reset_physical(t_ram *ram,
    lib_u32 physical, lib_uptr destination,
    lib_uptr bytes)
{
    const x86_core_machine_memory_device_provider *provider;
    lib_status status;

    if (ram == LIB_NULL || destination == 0u || bytes == 0u) {
        return LIB_STATUS_INVALID_ARGUMENT;
    }
    status = x86_core_machine_memory_reset_provider_resolve(ram, physical, bytes,
        &provider);
    if (status != LIB_STATUS_OK) return status;
    status = provider->read(provider->owner, physical, destination, bytes,
        LIB_FALSE);
    if (status != LIB_STATUS_UNSUPPORTED)
        x86_core_machine_memory_note_provider_failure(provider, physical, bytes,
            X86_CORE_MACHINE_MEMORY_ACCESS_READ, status);
    return status;
}
static void x86_core_machine_memory_snoop_written(t_ram *ram, lib_u32 physical,
    lib_u32 decoded, lib_size bytes)
{
    if (ram->connect.cpu_snoop == LIB_NULL) return;
    ram->connect.cpu_snoop(ram->connect.cpu_snoop_context, physical, bytes);
    if (decoded != physical)
        ram->connect.cpu_snoop(ram->connect.cpu_snoop_context, decoded, bytes);
}

lib_status x86_core_machine_memory_write_physical(t_ram *ram, lib_u32 physical,
    lib_uptr source, lib_uptr byte, lib_bool cpu_access)
{
    lib_size offset;
    lib_uptr index;
    const x86_core_machine_memory_device_provider *provider;
    lib_u32 provider_physical;
    lib_status status;

    if (ram == LIB_NULL || source == 0u || byte == 0u) {
        return LIB_STATUS_INVALID_ARGUMENT;
    }
    status = x86_core_machine_memory_route_resolve(ram, physical, byte,
        X86_CORE_MACHINE_MEMORY_ACCESS_WRITE, LIB_TRUE, &provider,
        &provider_physical, &offset, cpu_access);
    if (status != LIB_STATUS_OK) return status;
    if (provider == LIB_NULL && byte > 1u && !x86_core_machine_memory_is_plain_span(ram, physical, byte)) {
        lib_uptr index;
        for (index = 0u; index < byte; ++index) {
            const x86_core_machine_memory_device_provider *single_provider;
            lib_size single_offset;
            status = x86_core_machine_memory_route_resolve(ram,
                physical + (lib_u32)index, 1u,
                X86_CORE_MACHINE_MEMORY_ACCESS_WRITE, LIB_TRUE, &single_provider,
                &provider_physical, &single_offset, cpu_access);
            if (status != LIB_STATUS_OK) return status;
            if (single_provider != LIB_NULL) {
                for (index = 0u; index < byte; ++index) {
                    status = x86_core_machine_memory_write_physical(ram,
                        physical + (lib_u32)index, source + index, 1u, cpu_access);
                    if (status != LIB_STATUS_OK) return status;
                }
                return LIB_STATUS_OK;
            }
        }
    }
    if (provider != LIB_NULL) {
        status = provider->write(provider->owner, provider_physical, source, byte);
        /* An error can follow a provider's accepted prefix; cancellation is
         * not rollback. UNSUPPORTED alone declines this route without effects. */
        if (status != LIB_STATUS_UNSUPPORTED)
            x86_core_machine_memory_snoop_written(ram, physical, provider_physical, byte);
        if (status != LIB_STATUS_UNSUPPORTED) {
            x86_core_machine_memory_note_provider_failure(provider, provider_physical,
                byte, X86_CORE_MACHINE_MEMORY_ACCESS_WRITE, status);
            return status;
        }
        status = x86_core_machine_memory_offset(ram, physical,
            x86_core_machine_memory_decode_address(ram, physical), byte, &offset);
        if (status != LIB_STATUS_OK) return status;
    }
    for (index = 0u; index < ram->connect.mapping_count; ++index) {
        const x86_core_machine_memory_mapping *mapping = &ram->connect.mappings[index];

        if (mapping->selected && !mapping->writable &&
            physical >= mapping->physical_start &&
            (lib_u64)physical - mapping->physical_start + byte <= mapping->bytes) {
            x86_core_machine_memory_snoop_written(ram, physical, (lib_u32)offset, byte);
            return LIB_STATUS_OK;
        }
    }
    lib_memory_copy((void *)(ram->connect.backing + offset),
        (void *)source, byte);
    x86_core_machine_memory_snoop_written(ram, physical, (lib_u32)offset, byte);
    if (ram->connect.parity != 0u && offset < ram->connect.parity_bytes) {
        lib_uptr index;
        lib_uptr written = byte;
        if (written > ram->connect.parity_bytes - offset) written =
            ram->connect.parity_bytes - offset;
        for (index = 0u; index < written; ++index) ((lib_u8 *)ram->connect.parity)[offset + index] =
            x86_core_machine_memory_odd_parity(((const lib_u8 *)source)[index]);
    }
    for (index = 0u; index < ram->connect.write_observer_count; ++index) {
        x86_core_machine_memory_write_observer_slot *slot =
            &ram->connect.write_observers[index];
        slot->callback(slot->owner, physical, byte);
    }
    return LIB_STATUS_OK;
}

lib_status x86_core_machine_memory_query_physical(const t_ram *ram,
    lib_u32 physical, lib_uptr bytes,
    x86_core_machine_memory_access access, x86_core_machine_memory_route *out_route,
    lib_bool cpu_access)
{
    const x86_core_machine_memory_device_provider *provider;
    lib_u32 provider_physical;
    lib_size offset;
    lib_status status;

    if (out_route == LIB_NULL) return LIB_STATUS_INVALID_ARGUMENT;
    status = x86_core_machine_memory_route_resolve(ram, physical, bytes, access,
        LIB_TRUE, &provider, &provider_physical, &offset, cpu_access);
    if (status != LIB_STATUS_OK) return status;
    *out_route = provider == LIB_NULL ? X86_CORE_MACHINE_MEMORY_ROUTE_ORDINARY_RAM :
        X86_CORE_MACHINE_MEMORY_ROUTE_PROVIDER;
    return LIB_STATUS_OK;
}

/* Core cache-enable policy: direct RAM only. Prove the whole span is plain
 * or resolve each byte so a narrow provider cannot disappear into RAM. */
lib_status x86_core_machine_memory_query_cpu_cache(const t_ram *ram,
    lib_u32 physical, lib_bool reset_fetch, lib_bool check_fill,
    lib_u32 *out_address, lib_bool *out_fill_enabled)
{
    lib_u32 first;
    lib_size first_offset = 0u;
    const x86_core_machine_memory_device_provider *first_provider = LIB_NULL;
    lib_u32 first_provider_address = 0u;

    if (ram == LIB_NULL || out_address == LIB_NULL || out_fill_enabled == LIB_NULL)
        return LIB_STATUS_INVALID_ARGUMENT;
    *out_address = reset_fetch ? physical : x86_core_machine_memory_decode_address(ram, physical);
    *out_fill_enabled = LIB_FALSE;
    if (reset_fetch || !check_fill) return LIB_STATUS_OK;
    first = *out_address & ~15u;
    for (lib_uptr index = 0u; index < ram->connect.mapping_count; ++index) {
        const x86_core_machine_memory_mapping *mapping = &ram->connect.mappings[index];
        if ((lib_u64)first < (lib_u64)mapping->physical_start + mapping->bytes &&
            (lib_u64)mapping->physical_start < (lib_u64)first + 16u)
            return LIB_STATUS_OK;
    }
    if (x86_core_machine_memory_is_plain_span(ram, first, 16u)) {
        lib_size offset;
        *out_fill_enabled = x86_core_machine_memory_offset(ram, first, first, 16u, &offset) ==
            LIB_STATUS_OK && offset == first;
        return LIB_STATUS_OK;
    }
    for (lib_u32 byte = 0u; byte < 16u; ++byte) {
        const x86_core_machine_memory_device_provider *provider;
        lib_u32 provider_physical;
        lib_size offset;
        lib_status status = x86_core_machine_memory_route_resolve(ram, first + byte,
            1u, X86_CORE_MACHINE_MEMORY_ACCESS_READ, LIB_FALSE, &provider,
            &provider_physical, &offset, LIB_TRUE);

        /* A failed speculative line query disables fills. The requested data
         * access still reports its own route failure through the normal bus. */
        if (status != LIB_STATUS_OK) return LIB_STATUS_OK;
        if (byte == 0u) {
            first_provider = provider;
            first_provider_address = provider_physical;
            if (provider == LIB_NULL) first_offset = offset;
            if (provider != LIB_NULL && (provider->cache_probe == LIB_NULL ||
                provider->read_instruction != LIB_NULL ||
                !provider->cache_probe(provider->owner, provider_physical, 16u)))
                return LIB_STATUS_OK;
        }
        if (provider != first_provider) return LIB_STATUS_OK;
        if (provider != LIB_NULL) {
            if (provider_physical != first_provider_address + byte) return LIB_STATUS_OK;
        } else if (offset != first_offset + byte || offset !=
                x86_core_machine_memory_decode_address(ram, first + byte)) {
            return LIB_STATUS_OK;
        }
    }
    if (first_provider != LIB_NULL) {
        const x86_core_machine_memory_device_provider *line_provider;
        lib_u32 line_physical;
        lib_size line_offset;
        const lib_status status = x86_core_machine_memory_route_resolve(ram,
            first, 16u, X86_CORE_MACHINE_MEMORY_ACCESS_READ, LIB_FALSE,
            &line_provider, &line_physical, &line_offset, LIB_TRUE);

        /* The actual fill is a whole-line access; it must not select a
         * different provider from the bytewise route proof. */
        if (status != LIB_STATUS_OK || line_provider != first_provider ||
            line_physical != first_provider_address) return LIB_STATUS_OK;
    }
    *out_fill_enabled = LIB_TRUE;
    return LIB_STATUS_OK;
}

lib_status x86_core_machine_memory_initialize_for(t_ram *ram, lib_size bytes,
    x86_core_machine_memory_test_allocation *test_allocation)
{
    if (ram == LIB_NULL || bytes == 0u) return LIB_STATUS_INVALID_ARGUMENT;
    lib_memory_set((void *)ram, 0u, sizeof(*ram));
    return x86_core_machine_memory_allocate_for_with_test(ram, bytes,
        test_allocation);
}

void x86_core_machine_memory_reset(t_ram *ram)
{
    if (ram == LIB_NULL || ram->connect.backing == 0u) return;
    lib_memory_set((void *)&ram->data, 0u, sizeof(ram->data));
    ram->data.address_clear_mask = ram->connect.address_decode.reset_clear_mask;
    lib_memory_set((void *)ram->connect.backing, 0u,
        ram->connect.backing_capacity);
    if (ram->connect.parity != 0u) lib_memory_set((void *)ram->connect.parity,
        0u, ram->connect.parity_bytes);
}

void x86_core_machine_memory_finalize(t_ram *ram)
{
    if (ram == LIB_NULL) return;
    if (ram->connect.backing != 0u) {
        lib_release((void *)ram->connect.backing);
    }
    x86_core_machine_memory_release_parity(ram);
    lib_release(ram->connect.device_providers);
    ram->connect.device_providers = LIB_NULL;
    ram->connect.device_provider_count = 0u;
    ram->connect.device_provider_capacity = 0u;
    ram->connect.backing = 0u;
    ram->connect.installed_bytes = 0u;
    ram->connect.backing_capacity = 0u;
}

lib_status x86_core_machine_memory_read_real_from(t_ram *ram, lib_u16 segment,
    lib_u16 offset, void *out_data, lib_size size)
{
    lib_u32 physical;

    if (ram == LIB_NULL || out_data == LIB_NULL) return LIB_STATUS_INVALID_ARGUMENT;
    physical = x86_core_machine_memory_decode_address(ram,
        (((lib_u16)(segment)) << 4) + ((lib_u16)(offset)));
    return x86_core_machine_memory_read_physical(ram, physical,
        (lib_uptr)out_data, size, LIB_TRUE);
}

lib_status x86_core_machine_memory_write_real_to(t_ram *ram, lib_u16 segment,
    lib_u16 offset, const void *in_data, lib_size size)
{
    lib_u32 physical;

    if (ram == LIB_NULL || in_data == LIB_NULL) return LIB_STATUS_INVALID_ARGUMENT;
    physical = x86_core_machine_memory_decode_address(ram,
        (((lib_u16)(segment)) << 4) + ((lib_u16)(offset)));
    return x86_core_machine_memory_write_physical(ram, physical,
        (lib_uptr)in_data, size, LIB_TRUE);
}
