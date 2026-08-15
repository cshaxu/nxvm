#include "core/machine/media_interface.h"

static C_VOID core_machine_media_set_result(core_machine_media_result *out_result,
    core_machine_media_result result)
{
    if (out_result != STD_NULL) *out_result = result;
}

static const core_machine_media_binding *core_machine_media_find(
    const core_machine_media_registry *registry, core_machine_media_id id)
{
    type_unsigned_32 index;

    if (registry == STD_NULL || id == CORE_MACHINE_MEDIA_ID_INVALID) return STD_NULL;
    for (index = 0u; index < registry->binding_count; ++index) {
        if (registry->bindings[index].id == id) return &registry->bindings[index];
    }
    return STD_NULL;
}

static type_status core_machine_media_get_binding(
    const core_machine_media_registry *registry, core_machine_media_id id,
    const core_machine_media_binding **out_binding,
    core_machine_media_result *out_result)
{
    const core_machine_media_binding *binding;

    core_machine_media_set_result(out_result, CORE_MACHINE_MEDIA_RESULT_INVALID_RANGE);
    if (out_binding == STD_NULL || out_result == STD_NULL || registry == STD_NULL ||
        !registry->frozen) return TYPE_STATUS_INVALID_STATE;
    binding = core_machine_media_find(registry, id);
    if (binding == STD_NULL) return TYPE_STATUS_INVALID_ARGUMENT;
    *out_binding = binding;
    return TYPE_STATUS_OK;
}

static type_status core_machine_media_get_sector_range(
    const core_machine_media_registry *registry, core_machine_media_id id,
    type_unsigned_64 logical_sector, type_unsigned_32 sector_count, type_unsigned_64 *out_offset,
    type_unsigned_32 *out_byte_count, core_machine_media_result *out_result)
{
    core_machine_media_info info;
    type_status status;
    type_unsigned_64 byte_count;

    if (out_offset == STD_NULL || out_byte_count == STD_NULL || out_result == STD_NULL ||
        sector_count == 0u)
        return TYPE_STATUS_INVALID_ARGUMENT;
    status = core_machine_media_query(registry, id, &info, out_result);
    if (status != TYPE_STATUS_OK || *out_result != CORE_MACHINE_MEDIA_RESULT_OK)
        return status;
    if ((info.capabilities & CORE_MACHINE_MEDIA_CAPABILITY_GEOMETRY_KNOWN) == 0u ||
        info.geometry.bytes_per_sector == 0u ||
        logical_sector >= info.geometry.logical_sector_count ||
        sector_count > info.geometry.logical_sector_count - logical_sector) {
        core_machine_media_set_result(out_result, CORE_MACHINE_MEDIA_RESULT_INVALID_RANGE);
        return TYPE_STATUS_OK;
    }
    byte_count = (type_unsigned_64)sector_count * info.geometry.bytes_per_sector;
    if (byte_count > UINT32_MAX || logical_sector > UINT64_MAX /
            info.geometry.bytes_per_sector) {
        core_machine_media_set_result(out_result, CORE_MACHINE_MEDIA_RESULT_INVALID_RANGE);
        return TYPE_STATUS_OK;
    }
    *out_offset = logical_sector * info.geometry.bytes_per_sector;
    *out_byte_count = (type_unsigned_32)byte_count;
    return TYPE_STATUS_OK;
}

C_VOID core_machine_media_registry_initialize(core_machine_media_registry *registry)
{
    if (registry != STD_NULL) STD_MEMSET(registry, TYPE_ZERO_8, sizeof(*registry));
}

type_status core_machine_media_registry_bind(core_machine_media_registry *registry,
    core_machine_media_id id, C_VOID *context,
    const core_machine_media_provider *provider)
{
    core_machine_media_binding *binding;

    if (registry == STD_NULL || provider == STD_NULL || provider->query == STD_NULL ||
        id == CORE_MACHINE_MEDIA_ID_INVALID) return TYPE_STATUS_INVALID_ARGUMENT;
    if (registry->frozen) return TYPE_STATUS_INVALID_STATE;
    if (core_machine_media_find(registry, id) != STD_NULL ||
        registry->binding_count >= CORE_MACHINE_MEDIA_MAX_DEVICES)
        return TYPE_STATUS_INVALID_ARGUMENT;
    binding = &registry->bindings[registry->binding_count++];
    binding->id = id;
    binding->context = context;
    binding->provider = provider;
    return TYPE_STATUS_OK;
}

type_status core_machine_media_registry_freeze(core_machine_media_registry *registry)
{
    if (registry == STD_NULL) return TYPE_STATUS_INVALID_ARGUMENT;
    if (registry->frozen) return TYPE_STATUS_INVALID_STATE;
    registry->frozen = TYPE_TRUE;
    return TYPE_STATUS_OK;
}

C_VOID core_machine_media_registry_finalize(core_machine_media_registry *registry)
{
    if (registry != STD_NULL) STD_MEMSET(registry, TYPE_ZERO_8, sizeof(*registry));
}

type_status core_machine_media_query(const core_machine_media_registry *registry,
    core_machine_media_id id, core_machine_media_info *out_info,
    core_machine_media_result *out_result)
{
    const core_machine_media_binding *binding;
    type_status status;

    if (out_info == STD_NULL || out_result == STD_NULL) return TYPE_STATUS_INVALID_ARGUMENT;
    STD_MEMSET(out_info, TYPE_ZERO_8, sizeof(*out_info));
    status = core_machine_media_get_binding(registry, id, &binding, out_result);
    if (status != TYPE_STATUS_OK) return status;
    if (binding->provider->query == STD_NULL) return TYPE_STATUS_UNSUPPORTED;
    core_machine_media_set_result(out_result,
        binding->provider->query(binding->context, out_info));
    out_info->id = id;
    return TYPE_STATUS_OK;
}

type_status core_machine_media_read_bytes(const core_machine_media_registry *registry,
    core_machine_media_id id, type_unsigned_64 offset, C_VOID *buffer, type_unsigned_32 byte_count,
    core_machine_media_result *out_result)
{
    const core_machine_media_binding *binding;
    type_status status;

    if (buffer == STD_NULL || byte_count == 0u || out_result == STD_NULL)
        return TYPE_STATUS_INVALID_ARGUMENT;
    status = core_machine_media_get_binding(registry, id, &binding, out_result);
    if (status != TYPE_STATUS_OK) return status;
    if (binding->provider->read_bytes == STD_NULL) return TYPE_STATUS_UNSUPPORTED;
    core_machine_media_set_result(out_result,
        binding->provider->read_bytes(binding->context, offset, buffer, byte_count));
    return TYPE_STATUS_OK;
}

type_status core_machine_media_write_bytes(const core_machine_media_registry *registry,
    core_machine_media_id id, type_unsigned_64 offset, const C_VOID *buffer,
    type_unsigned_32 byte_count, core_machine_media_result *out_result)
{
    const core_machine_media_binding *binding;
    type_status status;

    if (buffer == STD_NULL || byte_count == 0u || out_result == STD_NULL)
        return TYPE_STATUS_INVALID_ARGUMENT;
    status = core_machine_media_get_binding(registry, id, &binding, out_result);
    if (status != TYPE_STATUS_OK) return status;
    if (binding->provider->write_bytes == STD_NULL) return TYPE_STATUS_UNSUPPORTED;
    core_machine_media_set_result(out_result,
        binding->provider->write_bytes(binding->context, offset, buffer, byte_count));
    return TYPE_STATUS_OK;
}

type_status core_machine_media_read_sectors(const core_machine_media_registry *registry,
    core_machine_media_id id, type_unsigned_64 logical_sector, type_unsigned_32 sector_count,
    C_VOID *buffer, core_machine_media_result *out_result)
{
    type_unsigned_64 offset;
    type_unsigned_32 byte_count;
    type_status status = core_machine_media_get_sector_range(registry, id,
        logical_sector, sector_count, &offset, &byte_count, out_result);

    if (status != TYPE_STATUS_OK || *out_result != CORE_MACHINE_MEDIA_RESULT_OK)
        return status;
    return core_machine_media_read_bytes(registry, id, offset, buffer, byte_count,
        out_result);
}

type_status core_machine_media_write_sectors(const core_machine_media_registry *registry,
    core_machine_media_id id, type_unsigned_64 logical_sector, type_unsigned_32 sector_count,
    const C_VOID *buffer, core_machine_media_result *out_result)
{
    type_unsigned_64 offset;
    type_unsigned_32 byte_count;
    type_status status = core_machine_media_get_sector_range(registry, id,
        logical_sector, sector_count, &offset, &byte_count, out_result);

    if (status != TYPE_STATUS_OK || *out_result != CORE_MACHINE_MEDIA_RESULT_OK)
        return status;
    return core_machine_media_write_bytes(registry, id, offset, buffer, byte_count,
        out_result);
}

type_status core_machine_media_format_sectors(const core_machine_media_registry *registry,
    core_machine_media_id id, type_unsigned_64 logical_sector, type_unsigned_32 sector_count,
    type_unsigned_8 fill, core_machine_media_result *out_result)
{
    const core_machine_media_binding *binding;
    core_machine_media_info info;
    type_status status;
    type_unsigned_64 offset;
    type_unsigned_32 byte_count;

    status = core_machine_media_get_sector_range(registry, id, logical_sector,
        sector_count, &offset, &byte_count, out_result);
    (C_VOID)offset;
    (C_VOID)byte_count;
    if (status != TYPE_STATUS_OK || *out_result != CORE_MACHINE_MEDIA_RESULT_OK)
        return status;
    status = core_machine_media_query(registry, id, &info, out_result);
    if (status != TYPE_STATUS_OK || *out_result != CORE_MACHINE_MEDIA_RESULT_OK)
        return status;
    if ((info.capabilities & CORE_MACHINE_MEDIA_CAPABILITY_FORMATTABLE) == 0u) {
        core_machine_media_set_result(out_result, CORE_MACHINE_MEDIA_RESULT_UNSUPPORTED);
        return TYPE_STATUS_OK;
    }
    status = core_machine_media_get_binding(registry, id, &binding, out_result);
    if (status != TYPE_STATUS_OK) return status;
    if (binding->provider->format_sectors == STD_NULL) {
        core_machine_media_set_result(out_result, CORE_MACHINE_MEDIA_RESULT_UNSUPPORTED);
        return TYPE_STATUS_OK;
    }
    core_machine_media_set_result(out_result, binding->provider->format_sectors(
        binding->context, logical_sector, sector_count, fill));
    return TYPE_STATUS_OK;
}

type_status core_machine_media_flush(const core_machine_media_registry *registry,
    core_machine_media_id id, core_machine_media_result *out_result)
{
    const core_machine_media_binding *binding;
    core_machine_media_info info;
    type_status status;

    if (out_result == STD_NULL) return TYPE_STATUS_INVALID_ARGUMENT;
    status = core_machine_media_query(registry, id, &info, out_result);

    if (status != TYPE_STATUS_OK || *out_result != CORE_MACHINE_MEDIA_RESULT_OK)
        return status;
    if ((info.capabilities & CORE_MACHINE_MEDIA_CAPABILITY_FLUSHABLE) == 0u) {
        core_machine_media_set_result(out_result, CORE_MACHINE_MEDIA_RESULT_UNSUPPORTED);
        return TYPE_STATUS_OK;
    }
    status = core_machine_media_get_binding(registry, id, &binding, out_result);
    if (status != TYPE_STATUS_OK) return status;
    if (binding->provider->flush == STD_NULL) {
        core_machine_media_set_result(out_result, CORE_MACHINE_MEDIA_RESULT_UNSUPPORTED);
        return TYPE_STATUS_OK;
    }
    core_machine_media_set_result(out_result,
        binding->provider->flush(binding->context));
    return TYPE_STATUS_OK;
}

type_status core_machine_media_get_address_mark(const core_machine_media_registry *registry,
    core_machine_media_id id, type_unsigned_64 logical_sector,
    core_machine_media_address_mark *out_mark, core_machine_media_result *out_result)
{
    const core_machine_media_binding *binding;
    core_machine_media_info info;
    type_status status;
    type_unsigned_64 offset;
    type_unsigned_32 byte_count;

    if (out_mark == STD_NULL) return TYPE_STATUS_INVALID_ARGUMENT;
    status = core_machine_media_get_sector_range(registry, id, logical_sector, 1u,
        &offset, &byte_count, out_result);
    (C_VOID)offset;
    (C_VOID)byte_count;
    if (status != TYPE_STATUS_OK || *out_result != CORE_MACHINE_MEDIA_RESULT_OK)
        return status;
    status = core_machine_media_query(registry, id, &info, out_result);
    if (status != TYPE_STATUS_OK || *out_result != CORE_MACHINE_MEDIA_RESULT_OK)
        return status;
    if ((info.capabilities & CORE_MACHINE_MEDIA_CAPABILITY_ADDRESS_MARKS) == 0u) {
        core_machine_media_set_result(out_result, CORE_MACHINE_MEDIA_RESULT_UNSUPPORTED);
        return TYPE_STATUS_OK;
    }
    status = core_machine_media_get_binding(registry, id, &binding, out_result);
    if (status != TYPE_STATUS_OK) return status;
    if (binding->provider->get_address_mark == STD_NULL) {
        core_machine_media_set_result(out_result, CORE_MACHINE_MEDIA_RESULT_UNSUPPORTED);
        return TYPE_STATUS_OK;
    }
    core_machine_media_set_result(out_result, binding->provider->get_address_mark(
        binding->context, logical_sector, out_mark));
    return TYPE_STATUS_OK;
}

type_status core_machine_media_set_address_mark(const core_machine_media_registry *registry,
    core_machine_media_id id, type_unsigned_64 logical_sector,
    core_machine_media_address_mark mark, core_machine_media_result *out_result)
{
    const core_machine_media_binding *binding;
    core_machine_media_info info;
    type_status status;
    type_unsigned_64 offset;
    type_unsigned_32 byte_count;

    if (mark != CORE_MACHINE_MEDIA_ADDRESS_MARK_DATA &&
        mark != CORE_MACHINE_MEDIA_ADDRESS_MARK_DELETED_DATA)
        return TYPE_STATUS_INVALID_ARGUMENT;
    status = core_machine_media_get_sector_range(registry, id, logical_sector, 1u,
        &offset, &byte_count, out_result);
    (C_VOID)offset;
    (C_VOID)byte_count;
    if (status != TYPE_STATUS_OK || *out_result != CORE_MACHINE_MEDIA_RESULT_OK)
        return status;
    status = core_machine_media_query(registry, id, &info, out_result);
    if (status != TYPE_STATUS_OK || *out_result != CORE_MACHINE_MEDIA_RESULT_OK)
        return status;
    if ((info.capabilities & CORE_MACHINE_MEDIA_CAPABILITY_ADDRESS_MARKS) == 0u) {
        core_machine_media_set_result(out_result, CORE_MACHINE_MEDIA_RESULT_UNSUPPORTED);
        return TYPE_STATUS_OK;
    }
    status = core_machine_media_get_binding(registry, id, &binding, out_result);
    if (status != TYPE_STATUS_OK) return status;
    if (binding->provider->set_address_mark == STD_NULL) {
        core_machine_media_set_result(out_result, CORE_MACHINE_MEDIA_RESULT_UNSUPPORTED);
        return TYPE_STATUS_OK;
    }
    core_machine_media_set_result(out_result, binding->provider->set_address_mark(
        binding->context, logical_sector, mark));
    return TYPE_STATUS_OK;
}
