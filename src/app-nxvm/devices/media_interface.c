#include "lib/types/types_interface.h"
#include "app-nxvm/devices/media_interface.h"

typedef struct core_machine_media_binding {
    core_machine_media_id id;
    C_VOID *context;
    const core_machine_media_provider *provider;
} core_machine_media_binding;

struct core_machine_media_registry {
    core_machine_media_binding bindings[CORE_MACHINE_MEDIA_MAX_DEVICES];
    lib_u32 binding_count;
    type_bool frozen;
};

static C_VOID core_machine_media_set_result(core_machine_media_result *out_result,
    core_machine_media_result result)
{
    if (out_result != LIB_NULL) *out_result = result;
}

static const core_machine_media_binding *core_machine_media_find(
    const core_machine_media_registry *registry, core_machine_media_id id)
{
    lib_u32 index;

    if (registry == LIB_NULL || id == CORE_MACHINE_MEDIA_ID_INVALID) return LIB_NULL;
    for (index = 0u; index < registry->binding_count; ++index) {
        if (registry->bindings[index].id == id) return &registry->bindings[index];
    }
    return LIB_NULL;
}

static type_status core_machine_media_get_binding(
    const core_machine_media_registry *registry, core_machine_media_id id,
    const core_machine_media_binding **out_binding,
    core_machine_media_result *out_result)
{
    const core_machine_media_binding *binding;

    core_machine_media_set_result(out_result, CORE_MACHINE_MEDIA_RESULT_INVALID_RANGE);
    if (out_binding == LIB_NULL || out_result == LIB_NULL || registry == LIB_NULL ||
        !registry->frozen) return TYPE_STATUS_INVALID_STATE;
    binding = core_machine_media_find(registry, id);
    if (binding == LIB_NULL) return TYPE_STATUS_INVALID_ARGUMENT;
    *out_binding = binding;
    return TYPE_STATUS_OK;
}

static type_status core_machine_media_get_sector_range(
    const core_machine_media_registry *registry, core_machine_media_id id,
    lib_u64 logical_sector, lib_u32 sector_count, lib_u64 *out_offset,
    lib_u32 *out_byte_count, core_machine_media_result *out_result)
{
    core_machine_media_info info;
    type_status status;
    lib_u64 byte_count;

    if (out_offset == LIB_NULL || out_byte_count == LIB_NULL || out_result == LIB_NULL ||
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
    byte_count = (lib_u64)sector_count * info.geometry.bytes_per_sector;
    if (byte_count > UINT32_MAX || logical_sector > UINT64_MAX /
            info.geometry.bytes_per_sector) {
        core_machine_media_set_result(out_result, CORE_MACHINE_MEDIA_RESULT_INVALID_RANGE);
        return TYPE_STATUS_OK;
    }
    *out_offset = logical_sector * info.geometry.bytes_per_sector;
    *out_byte_count = (lib_u32)byte_count;
    return TYPE_STATUS_OK;
}

type_status core_machine_media_registry_create(core_machine_media_registry **out_registry)
{
    core_machine_media_registry *registry;

    if (out_registry == LIB_NULL) return TYPE_STATUS_INVALID_ARGUMENT;
    *out_registry = LIB_NULL;
    registry = (core_machine_media_registry *)lib_allocate_zero(1u, sizeof(*registry));
    if (registry == LIB_NULL) return TYPE_STATUS_NO_MEMORY;
    *out_registry = registry;
    return TYPE_STATUS_OK;
}

type_status core_machine_media_registry_bind(core_machine_media_registry *registry,
    core_machine_media_id id, C_VOID *context,
    const core_machine_media_provider *provider)
{
    core_machine_media_binding *binding;

    if (registry == LIB_NULL || provider == LIB_NULL || provider->query == LIB_NULL ||
        id == CORE_MACHINE_MEDIA_ID_INVALID) return TYPE_STATUS_INVALID_ARGUMENT;
    if (registry->frozen) return TYPE_STATUS_INVALID_STATE;
    if (core_machine_media_find(registry, id) != LIB_NULL ||
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
    if (registry == LIB_NULL) return TYPE_STATUS_INVALID_ARGUMENT;
    if (registry->frozen) return TYPE_STATUS_INVALID_STATE;
    registry->frozen = LIB_TRUE;
    return TYPE_STATUS_OK;
}

C_VOID core_machine_media_registry_destroy(core_machine_media_registry *registry)
{
    lib_release(registry);
}

type_status core_machine_media_query(const core_machine_media_registry *registry,
    core_machine_media_id id, core_machine_media_info *out_info,
    core_machine_media_result *out_result)
{
    const core_machine_media_binding *binding;
    type_status status;

    if (out_info == LIB_NULL || out_result == LIB_NULL) return TYPE_STATUS_INVALID_ARGUMENT;
    lib_memory_set(out_info, TYPE_ZERO_8, sizeof(*out_info));
    status = core_machine_media_get_binding(registry, id, &binding, out_result);
    if (status != TYPE_STATUS_OK) return status;
    if (binding->provider->query == LIB_NULL) return TYPE_STATUS_UNSUPPORTED;
    core_machine_media_set_result(out_result,
        binding->provider->query(binding->context, out_info));
    out_info->id = id;
    return TYPE_STATUS_OK;
}

type_status core_machine_media_read_bytes(const core_machine_media_registry *registry,
    core_machine_media_id id, lib_u64 offset, C_VOID *buffer, lib_u32 byte_count,
    core_machine_media_result *out_result)
{
    const core_machine_media_binding *binding;
    type_status status;

    if (buffer == LIB_NULL || byte_count == 0u || out_result == LIB_NULL)
        return TYPE_STATUS_INVALID_ARGUMENT;
    status = core_machine_media_get_binding(registry, id, &binding, out_result);
    if (status != TYPE_STATUS_OK) return status;
    if (binding->provider->read_bytes == LIB_NULL) return TYPE_STATUS_UNSUPPORTED;
    core_machine_media_set_result(out_result,
        binding->provider->read_bytes(binding->context, offset, buffer, byte_count));
    return TYPE_STATUS_OK;
}

type_status core_machine_media_write_bytes(const core_machine_media_registry *registry,
    core_machine_media_id id, lib_u64 offset, const C_VOID *buffer,
    lib_u32 byte_count, core_machine_media_result *out_result)
{
    const core_machine_media_binding *binding;
    type_status status;

    if (buffer == LIB_NULL || byte_count == 0u || out_result == LIB_NULL)
        return TYPE_STATUS_INVALID_ARGUMENT;
    status = core_machine_media_get_binding(registry, id, &binding, out_result);
    if (status != TYPE_STATUS_OK) return status;
    if (binding->provider->write_bytes == LIB_NULL) return TYPE_STATUS_UNSUPPORTED;
    core_machine_media_set_result(out_result,
        binding->provider->write_bytes(binding->context, offset, buffer, byte_count));
    return TYPE_STATUS_OK;
}

type_status core_machine_media_read_sectors(const core_machine_media_registry *registry,
    core_machine_media_id id, lib_u64 logical_sector, lib_u32 sector_count,
    C_VOID *buffer, core_machine_media_result *out_result)
{
    lib_u64 offset;
    lib_u32 byte_count;
    type_status status = core_machine_media_get_sector_range(registry, id,
        logical_sector, sector_count, &offset, &byte_count, out_result);

    if (status != TYPE_STATUS_OK || *out_result != CORE_MACHINE_MEDIA_RESULT_OK)
        return status;
    return core_machine_media_read_bytes(registry, id, offset, buffer, byte_count,
        out_result);
}

type_status core_machine_media_write_sectors(const core_machine_media_registry *registry,
    core_machine_media_id id, lib_u64 logical_sector, lib_u32 sector_count,
    const C_VOID *buffer, core_machine_media_result *out_result)
{
    lib_u64 offset;
    lib_u32 byte_count;
    type_status status = core_machine_media_get_sector_range(registry, id,
        logical_sector, sector_count, &offset, &byte_count, out_result);

    if (status != TYPE_STATUS_OK || *out_result != CORE_MACHINE_MEDIA_RESULT_OK)
        return status;
    return core_machine_media_write_bytes(registry, id, offset, buffer, byte_count,
        out_result);
}

type_status core_machine_media_format_sectors(const core_machine_media_registry *registry,
    core_machine_media_id id, lib_u64 logical_sector, lib_u32 sector_count,
    lib_u8 fill, core_machine_media_result *out_result)
{
    const core_machine_media_binding *binding;
    core_machine_media_info info;
    type_status status;
    lib_u64 offset;
    lib_u32 byte_count;

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
    if (binding->provider->format_sectors == LIB_NULL) {
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

    if (out_result == LIB_NULL) return TYPE_STATUS_INVALID_ARGUMENT;
    status = core_machine_media_query(registry, id, &info, out_result);

    if (status != TYPE_STATUS_OK || *out_result != CORE_MACHINE_MEDIA_RESULT_OK)
        return status;
    if ((info.capabilities & CORE_MACHINE_MEDIA_CAPABILITY_FLUSHABLE) == 0u) {
        core_machine_media_set_result(out_result, CORE_MACHINE_MEDIA_RESULT_UNSUPPORTED);
        return TYPE_STATUS_OK;
    }
    status = core_machine_media_get_binding(registry, id, &binding, out_result);
    if (status != TYPE_STATUS_OK) return status;
    if (binding->provider->flush == LIB_NULL) {
        core_machine_media_set_result(out_result, CORE_MACHINE_MEDIA_RESULT_UNSUPPORTED);
        return TYPE_STATUS_OK;
    }
    core_machine_media_set_result(out_result,
        binding->provider->flush(binding->context));
    return TYPE_STATUS_OK;
}

type_status core_machine_media_get_address_mark(const core_machine_media_registry *registry,
    core_machine_media_id id, lib_u64 logical_sector,
    core_machine_media_address_mark *out_mark, core_machine_media_result *out_result)
{
    const core_machine_media_binding *binding;
    core_machine_media_info info;
    type_status status;
    lib_u64 offset;
    lib_u32 byte_count;
    core_machine_media_address_mark mark;

    if (out_mark == LIB_NULL) return TYPE_STATUS_INVALID_ARGUMENT;
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
    if (binding->provider->get_address_mark == LIB_NULL) {
        core_machine_media_set_result(out_result, CORE_MACHINE_MEDIA_RESULT_UNSUPPORTED);
        return TYPE_STATUS_OK;
    }
    core_machine_media_set_result(out_result, binding->provider->get_address_mark(
        binding->context, logical_sector, &mark));
    if (*out_result == CORE_MACHINE_MEDIA_RESULT_OK &&
        mark != CORE_MACHINE_MEDIA_ADDRESS_MARK_DATA &&
        mark != CORE_MACHINE_MEDIA_ADDRESS_MARK_DELETED_DATA) {
        core_machine_media_set_result(out_result, CORE_MACHINE_MEDIA_RESULT_PERMANENT);
        return TYPE_STATUS_OK;
    }
    if (*out_result == CORE_MACHINE_MEDIA_RESULT_OK) *out_mark = mark;
    return TYPE_STATUS_OK;
}

type_status core_machine_media_set_address_mark(const core_machine_media_registry *registry,
    core_machine_media_id id, lib_u64 logical_sector,
    core_machine_media_address_mark mark, core_machine_media_result *out_result)
{
    const core_machine_media_binding *binding;
    core_machine_media_info info;
    type_status status;
    lib_u64 offset;
    lib_u32 byte_count;

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
    if (binding->provider->set_address_mark == LIB_NULL) {
        core_machine_media_set_result(out_result, CORE_MACHINE_MEDIA_RESULT_UNSUPPORTED);
        return TYPE_STATUS_OK;
    }
    core_machine_media_set_result(out_result, binding->provider->set_address_mark(
        binding->context, logical_sector, mark));
    return TYPE_STATUS_OK;
}
