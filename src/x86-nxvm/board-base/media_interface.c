#include "lib/types/types_interface.h"
#include "x86-nxvm/board-base/media_interface.h"

typedef struct x86_nxvm_board_media_binding {
    x86_nxvm_board_media_id id;
    void *context;
    const x86_nxvm_board_media_provider *provider;
} x86_nxvm_board_media_binding;

struct x86_nxvm_board_media_registry {
    x86_nxvm_board_media_binding bindings[X86_NXVM_BOARD_MEDIA_MAX_DEVICES];
    lib_u32 binding_count;
    lib_u8 frozen;
};

static void x86_nxvm_board_media_set_result(x86_nxvm_board_media_result *out_result,
    x86_nxvm_board_media_result result)
{
    if (out_result != LIB_NULL) *out_result = result;
}

static const x86_nxvm_board_media_binding *x86_nxvm_board_media_find(
    const x86_nxvm_board_media_registry *registry, x86_nxvm_board_media_id id)
{
    lib_u32 index;

    if (registry == LIB_NULL || id == X86_NXVM_BOARD_MEDIA_ID_INVALID) return LIB_NULL;
    for (index = 0u; index < registry->binding_count; ++index) {
        if (registry->bindings[index].id == id) return &registry->bindings[index];
    }
    return LIB_NULL;
}

static lib_status x86_nxvm_board_media_get_binding(
    const x86_nxvm_board_media_registry *registry, x86_nxvm_board_media_id id,
    const x86_nxvm_board_media_binding **out_binding,
    x86_nxvm_board_media_result *out_result)
{
    const x86_nxvm_board_media_binding *binding;

    x86_nxvm_board_media_set_result(out_result, X86_NXVM_BOARD_MEDIA_RESULT_INVALID_RANGE);
    if (out_binding == LIB_NULL || out_result == LIB_NULL || registry == LIB_NULL ||
        !registry->frozen) return LIB_STATUS_INVALID_STATE;
    binding = x86_nxvm_board_media_find(registry, id);
    if (binding == LIB_NULL) return LIB_STATUS_INVALID_ARGUMENT;
    *out_binding = binding;
    x86_nxvm_board_media_set_result(out_result, X86_NXVM_BOARD_MEDIA_RESULT_OK);
    return LIB_STATUS_OK;
}

static lib_status x86_nxvm_board_media_get_sector_range(
    const x86_nxvm_board_media_registry *registry, x86_nxvm_board_media_id id,
    lib_u64 logical_sector, lib_u32 sector_count, lib_u64 *out_offset,
    lib_u32 *out_byte_count, x86_nxvm_board_media_result *out_result)
{
    x86_nxvm_board_media_info info;
    lib_status status;
    lib_u64 byte_count;

    if (out_offset == LIB_NULL || out_byte_count == LIB_NULL || out_result == LIB_NULL ||
        sector_count == 0u)
        return LIB_STATUS_INVALID_ARGUMENT;
    status = x86_nxvm_board_media_query(registry, id, &info, out_result);
    if (status != LIB_STATUS_OK || *out_result != X86_NXVM_BOARD_MEDIA_RESULT_OK)
        return status;
    if ((info.capabilities & X86_NXVM_BOARD_MEDIA_CAPABILITY_GEOMETRY_KNOWN) == 0u ||
        info.geometry.bytes_per_sector == 0u ||
        logical_sector >= info.geometry.logical_sector_count ||
        sector_count > info.geometry.logical_sector_count - logical_sector) {
        x86_nxvm_board_media_set_result(out_result, X86_NXVM_BOARD_MEDIA_RESULT_INVALID_RANGE);
        return LIB_STATUS_OK;
    }
    byte_count = (lib_u64)sector_count * info.geometry.bytes_per_sector;
    if (byte_count > LIB_UINT32_MAX || logical_sector > LIB_UINT64_MAX /
            info.geometry.bytes_per_sector) {
        x86_nxvm_board_media_set_result(out_result, X86_NXVM_BOARD_MEDIA_RESULT_INVALID_RANGE);
        return LIB_STATUS_OK;
    }
    *out_offset = logical_sector * info.geometry.bytes_per_sector;
    *out_byte_count = (lib_u32)byte_count;
    return LIB_STATUS_OK;
}

lib_status x86_nxvm_board_media_registry_create(x86_nxvm_board_media_registry **out_registry)
{
    x86_nxvm_board_media_registry *registry;

    if (out_registry == LIB_NULL) return LIB_STATUS_INVALID_ARGUMENT;
    *out_registry = LIB_NULL;
    registry = (x86_nxvm_board_media_registry *)lib_allocate_zero(1u, sizeof(*registry));
    if (registry == LIB_NULL) return LIB_STATUS_NO_MEMORY;
    *out_registry = registry;
    return LIB_STATUS_OK;
}

lib_status x86_nxvm_board_media_registry_bind(x86_nxvm_board_media_registry *registry,
    x86_nxvm_board_media_id id, void *context,
    const x86_nxvm_board_media_provider *provider)
{
    x86_nxvm_board_media_binding *binding;

    if (registry == LIB_NULL || provider == LIB_NULL || provider->query == LIB_NULL ||
        id == X86_NXVM_BOARD_MEDIA_ID_INVALID) return LIB_STATUS_INVALID_ARGUMENT;
    if (registry->frozen) return LIB_STATUS_INVALID_STATE;
    if (x86_nxvm_board_media_find(registry, id) != LIB_NULL ||
        registry->binding_count >= X86_NXVM_BOARD_MEDIA_MAX_DEVICES)
        return LIB_STATUS_INVALID_ARGUMENT;
    binding = &registry->bindings[registry->binding_count++];
    binding->id = id;
    binding->context = context;
    binding->provider = provider;
    return LIB_STATUS_OK;
}

lib_status x86_nxvm_board_media_registry_freeze(x86_nxvm_board_media_registry *registry)
{
    if (registry == LIB_NULL) return LIB_STATUS_INVALID_ARGUMENT;
    if (registry->frozen) return LIB_STATUS_INVALID_STATE;
    registry->frozen = LIB_TRUE;
    return LIB_STATUS_OK;
}

void x86_nxvm_board_media_registry_destroy(x86_nxvm_board_media_registry *registry)
{
    lib_release(registry);
}

lib_status x86_nxvm_board_media_query(const x86_nxvm_board_media_registry *registry,
    x86_nxvm_board_media_id id, x86_nxvm_board_media_info *out_info,
    x86_nxvm_board_media_result *out_result)
{
    const x86_nxvm_board_media_binding *binding;
    lib_status status;

    if (out_info == LIB_NULL || out_result == LIB_NULL) return LIB_STATUS_INVALID_ARGUMENT;
    lib_memory_set(out_info, 0u, sizeof(*out_info));
    status = x86_nxvm_board_media_get_binding(registry, id, &binding, out_result);
    if (status != LIB_STATUS_OK) return status;
    if (binding->provider->query == LIB_NULL) return LIB_STATUS_UNSUPPORTED;
    x86_nxvm_board_media_set_result(out_result,
        binding->provider->query(binding->context, out_info));
    out_info->id = id;
    return LIB_STATUS_OK;
}

lib_status x86_nxvm_board_media_read_bytes(const x86_nxvm_board_media_registry *registry,
    x86_nxvm_board_media_id id, lib_u64 offset, void *buffer, lib_u32 byte_count,
    x86_nxvm_board_media_result *out_result)
{
    const x86_nxvm_board_media_binding *binding;
    lib_status status;

    if (buffer == LIB_NULL || byte_count == 0u || out_result == LIB_NULL)
        return LIB_STATUS_INVALID_ARGUMENT;
    status = x86_nxvm_board_media_get_binding(registry, id, &binding, out_result);
    if (status != LIB_STATUS_OK) return status;
    if (binding->provider->read_bytes == LIB_NULL) return LIB_STATUS_UNSUPPORTED;
    x86_nxvm_board_media_set_result(out_result,
        binding->provider->read_bytes(binding->context, offset, buffer, byte_count));
    return LIB_STATUS_OK;
}

lib_status x86_nxvm_board_media_write_bytes(const x86_nxvm_board_media_registry *registry,
    x86_nxvm_board_media_id id, lib_u64 offset, const void *buffer,
    lib_u32 byte_count, x86_nxvm_board_media_result *out_result)
{
    const x86_nxvm_board_media_binding *binding;
    lib_status status;

    if (buffer == LIB_NULL || byte_count == 0u || out_result == LIB_NULL)
        return LIB_STATUS_INVALID_ARGUMENT;
    status = x86_nxvm_board_media_get_binding(registry, id, &binding, out_result);
    if (status != LIB_STATUS_OK) return status;
    if (binding->provider->write_bytes == LIB_NULL) return LIB_STATUS_UNSUPPORTED;
    x86_nxvm_board_media_set_result(out_result,
        binding->provider->write_bytes(binding->context, offset, buffer, byte_count));
    return LIB_STATUS_OK;
}

lib_status x86_nxvm_board_media_read_sectors(const x86_nxvm_board_media_registry *registry,
    x86_nxvm_board_media_id id, lib_u64 logical_sector, lib_u32 sector_count,
    void *buffer, x86_nxvm_board_media_result *out_result)
{
    lib_u64 offset;
    lib_u32 byte_count;
    lib_status status = x86_nxvm_board_media_get_sector_range(registry, id,
        logical_sector, sector_count, &offset, &byte_count, out_result);

    if (status != LIB_STATUS_OK || *out_result != X86_NXVM_BOARD_MEDIA_RESULT_OK)
        return status;
    return x86_nxvm_board_media_read_bytes(registry, id, offset, buffer, byte_count,
        out_result);
}

static lib_status x86_nxvm_board_media_get_record_binding(
    const x86_nxvm_board_media_registry *registry, x86_nxvm_board_media_id id,
    lib_u64 sector, const x86_nxvm_board_media_binding **out_binding,
    x86_nxvm_board_media_result *out_result)
{
    x86_nxvm_board_media_info info;
    lib_status status = x86_nxvm_board_media_query(registry, id, &info, out_result);

    if (status != LIB_STATUS_OK || *out_result != X86_NXVM_BOARD_MEDIA_RESULT_OK)
        return status;
    if (info.geometry.bytes_per_sector != 512u ||
        sector >= info.geometry.logical_sector_count || sector > LIB_UINT64_MAX / 512u) {
        *out_result = X86_NXVM_BOARD_MEDIA_RESULT_INVALID_RANGE;
        return LIB_STATUS_OK;
    }
    return x86_nxvm_board_media_get_binding(registry, id, out_binding, out_result);
}

lib_status x86_nxvm_board_media_read_record(const x86_nxvm_board_media_registry *registry,
    x86_nxvm_board_media_id id, lib_u64 sector, x86_nxvm_board_media_record *out_record,
    x86_nxvm_board_media_result *out_result)
{
    const x86_nxvm_board_media_binding *binding;
    x86_nxvm_board_media_record candidate = {0};
    lib_status status;

    if (out_record == LIB_NULL || out_result == LIB_NULL)
        return LIB_STATUS_INVALID_ARGUMENT;
    status = x86_nxvm_board_media_get_record_binding(registry, id, sector, &binding, out_result);
    if (status != LIB_STATUS_OK || *out_result != X86_NXVM_BOARD_MEDIA_RESULT_OK)
        return status;
    if (binding->provider->read_record != LIB_NULL) {
        *out_result = binding->provider->read_record(binding->context, sector, &candidate);
    } else {
        status = x86_nxvm_board_media_read_bytes(registry, id, sector * 512u,
            candidate.data, sizeof(candidate.data), out_result);
    }
    if (status == LIB_STATUS_OK && *out_result == X86_NXVM_BOARD_MEDIA_RESULT_OK)
        *out_record = candidate;
    return status;
}

lib_status x86_nxvm_board_media_write_record(const x86_nxvm_board_media_registry *registry,
    x86_nxvm_board_media_id id, lib_u64 sector, const x86_nxvm_board_media_record *record,
    x86_nxvm_board_media_result *out_result)
{
    const x86_nxvm_board_media_binding *binding;
    lib_status status;

    if (record == LIB_NULL || out_result == LIB_NULL)
        return LIB_STATUS_INVALID_ARGUMENT;
    status = x86_nxvm_board_media_get_record_binding(registry, id, sector, &binding, out_result);
    if (status != LIB_STATUS_OK || *out_result != X86_NXVM_BOARD_MEDIA_RESULT_OK)
        return status;
    if (binding->provider->write_record != LIB_NULL) {
        *out_result = binding->provider->write_record(binding->context, sector, record);
        return LIB_STATUS_OK;
    }
    if (record->ecc_present) {
        *out_result = X86_NXVM_BOARD_MEDIA_RESULT_UNSUPPORTED;
        return LIB_STATUS_OK;
    }
    return x86_nxvm_board_media_write_bytes(registry, id, sector * 512u,
        record->data, sizeof(record->data), out_result);
}

lib_status x86_nxvm_board_media_write_sectors(const x86_nxvm_board_media_registry *registry,
    x86_nxvm_board_media_id id, lib_u64 logical_sector, lib_u32 sector_count,
    const void *buffer, x86_nxvm_board_media_result *out_result)
{
    lib_u64 offset;
    lib_u32 byte_count;
    lib_status status = x86_nxvm_board_media_get_sector_range(registry, id,
        logical_sector, sector_count, &offset, &byte_count, out_result);

    if (status != LIB_STATUS_OK || *out_result != X86_NXVM_BOARD_MEDIA_RESULT_OK)
        return status;
    return x86_nxvm_board_media_write_bytes(registry, id, offset, buffer, byte_count,
        out_result);
}

lib_status x86_nxvm_board_media_format_sectors(const x86_nxvm_board_media_registry *registry,
    x86_nxvm_board_media_id id, lib_u64 logical_sector, lib_u32 sector_count,
    lib_u8 fill, x86_nxvm_board_media_result *out_result)
{
    const x86_nxvm_board_media_binding *binding;
    x86_nxvm_board_media_info info;
    lib_status status;
    lib_u64 offset;
    lib_u32 byte_count;

    status = x86_nxvm_board_media_get_sector_range(registry, id, logical_sector,
        sector_count, &offset, &byte_count, out_result);
    (void)offset;
    (void)byte_count;
    if (status != LIB_STATUS_OK || *out_result != X86_NXVM_BOARD_MEDIA_RESULT_OK)
        return status;
    status = x86_nxvm_board_media_query(registry, id, &info, out_result);
    if (status != LIB_STATUS_OK || *out_result != X86_NXVM_BOARD_MEDIA_RESULT_OK)
        return status;
    if ((info.capabilities & X86_NXVM_BOARD_MEDIA_CAPABILITY_FORMATTABLE) == 0u) {
        x86_nxvm_board_media_set_result(out_result, X86_NXVM_BOARD_MEDIA_RESULT_UNSUPPORTED);
        return LIB_STATUS_OK;
    }
    status = x86_nxvm_board_media_get_binding(registry, id, &binding, out_result);
    if (status != LIB_STATUS_OK) return status;
    if (binding->provider->format_sectors == LIB_NULL) {
        x86_nxvm_board_media_set_result(out_result, X86_NXVM_BOARD_MEDIA_RESULT_UNSUPPORTED);
        return LIB_STATUS_OK;
    }
    x86_nxvm_board_media_set_result(out_result, binding->provider->format_sectors(
        binding->context, logical_sector, sector_count, fill));
    return LIB_STATUS_OK;
}

lib_status x86_nxvm_board_media_format_track(const x86_nxvm_board_media_registry *registry,
    x86_nxvm_board_media_id id, const x86_nxvm_board_media_track *track,
    x86_nxvm_board_media_result *out_result)
{
    const x86_nxvm_board_media_binding *binding;
    x86_nxvm_board_media_info info;
    lib_bool seen[256] = {0};
    lib_status status;

    if (track == LIB_NULL || out_result == LIB_NULL ||
        track->sector_count == 0u || track->sector_count > sizeof(track->ids) ||
        (track->fill_payload != LIB_FALSE && track->fill_payload != LIB_TRUE))
        return LIB_STATUS_INVALID_ARGUMENT;
    for (lib_u32 index = 0u; index < track->sector_count; ++index) {
        const lib_u8 sector = track->ids[index];
        if (sector >= track->sector_count || seen[sector] ||
            (track->bad[index] != LIB_FALSE && track->bad[index] != LIB_TRUE))
            return LIB_STATUS_INVALID_ARGUMENT;
        seen[sector] = LIB_TRUE;
    }
    status = x86_nxvm_board_media_query(registry, id, &info, out_result);
    if (status != LIB_STATUS_OK || *out_result != X86_NXVM_BOARD_MEDIA_RESULT_OK)
        return status;
    if ((info.capabilities & X86_NXVM_BOARD_MEDIA_CAPABILITY_READ_ONLY) != 0u) {
        *out_result = X86_NXVM_BOARD_MEDIA_RESULT_READ_ONLY;
        return LIB_STATUS_OK;
    }
    if ((info.capabilities & X86_NXVM_BOARD_MEDIA_CAPABILITY_FORMATTABLE) == 0u) {
        *out_result = X86_NXVM_BOARD_MEDIA_RESULT_UNSUPPORTED;
        return LIB_STATUS_OK;
    }
    if (info.geometry.bytes_per_sector != 512u ||
        track->first_sector >= info.geometry.logical_sector_count ||
        track->sector_count > info.geometry.logical_sector_count - track->first_sector ||
        track->first_sector % track->sector_count != 0u) {
        *out_result = X86_NXVM_BOARD_MEDIA_RESULT_INVALID_RANGE;
        return LIB_STATUS_OK;
    }
    status = x86_nxvm_board_media_get_binding(registry, id, &binding, out_result);
    if (status != LIB_STATUS_OK || *out_result != X86_NXVM_BOARD_MEDIA_RESULT_OK)
        return status;
    if (binding->provider->format_track == LIB_NULL) {
        *out_result = X86_NXVM_BOARD_MEDIA_RESULT_UNSUPPORTED;
        return LIB_STATUS_OK;
    }
    *out_result = binding->provider->format_track(binding->context, track);
    return LIB_STATUS_OK;
}

lib_status x86_nxvm_board_media_flush(const x86_nxvm_board_media_registry *registry,
    x86_nxvm_board_media_id id, x86_nxvm_board_media_result *out_result)
{
    const x86_nxvm_board_media_binding *binding;
    x86_nxvm_board_media_info info;
    lib_status status;

    if (out_result == LIB_NULL) return LIB_STATUS_INVALID_ARGUMENT;
    status = x86_nxvm_board_media_query(registry, id, &info, out_result);

    if (status != LIB_STATUS_OK || *out_result != X86_NXVM_BOARD_MEDIA_RESULT_OK)
        return status;
    if ((info.capabilities & X86_NXVM_BOARD_MEDIA_CAPABILITY_FLUSHABLE) == 0u) {
        x86_nxvm_board_media_set_result(out_result, X86_NXVM_BOARD_MEDIA_RESULT_UNSUPPORTED);
        return LIB_STATUS_OK;
    }
    status = x86_nxvm_board_media_get_binding(registry, id, &binding, out_result);
    if (status != LIB_STATUS_OK) return status;
    if (binding->provider->flush == LIB_NULL) {
        x86_nxvm_board_media_set_result(out_result, X86_NXVM_BOARD_MEDIA_RESULT_UNSUPPORTED);
        return LIB_STATUS_OK;
    }
    x86_nxvm_board_media_set_result(out_result,
        binding->provider->flush(binding->context));
    return LIB_STATUS_OK;
}

lib_status x86_nxvm_board_media_get_address_mark(const x86_nxvm_board_media_registry *registry,
    x86_nxvm_board_media_id id, lib_u64 logical_sector,
    x86_nxvm_board_media_address_mark *out_mark, x86_nxvm_board_media_result *out_result)
{
    const x86_nxvm_board_media_binding *binding;
    x86_nxvm_board_media_info info;
    lib_status status;
    lib_u64 offset;
    lib_u32 byte_count;
    x86_nxvm_board_media_address_mark mark;

    if (out_mark == LIB_NULL) return LIB_STATUS_INVALID_ARGUMENT;
    status = x86_nxvm_board_media_get_sector_range(registry, id, logical_sector, 1u,
        &offset, &byte_count, out_result);
    (void)offset;
    (void)byte_count;
    if (status != LIB_STATUS_OK || *out_result != X86_NXVM_BOARD_MEDIA_RESULT_OK)
        return status;
    status = x86_nxvm_board_media_query(registry, id, &info, out_result);
    if (status != LIB_STATUS_OK || *out_result != X86_NXVM_BOARD_MEDIA_RESULT_OK)
        return status;
    if ((info.capabilities & X86_NXVM_BOARD_MEDIA_CAPABILITY_ADDRESS_MARKS) == 0u) {
        x86_nxvm_board_media_set_result(out_result, X86_NXVM_BOARD_MEDIA_RESULT_UNSUPPORTED);
        return LIB_STATUS_OK;
    }
    status = x86_nxvm_board_media_get_binding(registry, id, &binding, out_result);
    if (status != LIB_STATUS_OK) return status;
    if (binding->provider->get_address_mark == LIB_NULL) {
        x86_nxvm_board_media_set_result(out_result, X86_NXVM_BOARD_MEDIA_RESULT_UNSUPPORTED);
        return LIB_STATUS_OK;
    }
    x86_nxvm_board_media_set_result(out_result, binding->provider->get_address_mark(
        binding->context, logical_sector, &mark));
    if (*out_result == X86_NXVM_BOARD_MEDIA_RESULT_OK &&
        mark != X86_NXVM_BOARD_MEDIA_ADDRESS_MARK_DATA &&
        mark != X86_NXVM_BOARD_MEDIA_ADDRESS_MARK_DELETED_DATA) {
        x86_nxvm_board_media_set_result(out_result, X86_NXVM_BOARD_MEDIA_RESULT_PERMANENT);
        return LIB_STATUS_OK;
    }
    if (*out_result == X86_NXVM_BOARD_MEDIA_RESULT_OK) *out_mark = mark;
    return LIB_STATUS_OK;
}

lib_status x86_nxvm_board_media_set_address_mark(const x86_nxvm_board_media_registry *registry,
    x86_nxvm_board_media_id id, lib_u64 logical_sector,
    x86_nxvm_board_media_address_mark mark, x86_nxvm_board_media_result *out_result)
{
    const x86_nxvm_board_media_binding *binding;
    x86_nxvm_board_media_info info;
    lib_status status;
    lib_u64 offset;
    lib_u32 byte_count;

    if (mark != X86_NXVM_BOARD_MEDIA_ADDRESS_MARK_DATA &&
        mark != X86_NXVM_BOARD_MEDIA_ADDRESS_MARK_DELETED_DATA)
        return LIB_STATUS_INVALID_ARGUMENT;
    status = x86_nxvm_board_media_get_sector_range(registry, id, logical_sector, 1u,
        &offset, &byte_count, out_result);
    (void)offset;
    (void)byte_count;
    if (status != LIB_STATUS_OK || *out_result != X86_NXVM_BOARD_MEDIA_RESULT_OK)
        return status;
    status = x86_nxvm_board_media_query(registry, id, &info, out_result);
    if (status != LIB_STATUS_OK || *out_result != X86_NXVM_BOARD_MEDIA_RESULT_OK)
        return status;
    if ((info.capabilities & X86_NXVM_BOARD_MEDIA_CAPABILITY_ADDRESS_MARKS) == 0u) {
        x86_nxvm_board_media_set_result(out_result, X86_NXVM_BOARD_MEDIA_RESULT_UNSUPPORTED);
        return LIB_STATUS_OK;
    }
    status = x86_nxvm_board_media_get_binding(registry, id, &binding, out_result);
    if (status != LIB_STATUS_OK) return status;
    if (binding->provider->set_address_mark == LIB_NULL) {
        x86_nxvm_board_media_set_result(out_result, X86_NXVM_BOARD_MEDIA_RESULT_UNSUPPORTED);
        return LIB_STATUS_OK;
    }
    x86_nxvm_board_media_set_result(out_result, binding->provider->set_address_mark(
        binding->context, logical_sector, mark));
    return LIB_STATUS_OK;
}
