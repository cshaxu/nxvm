#include "core/platform/backing_resource_interface.h"

static C_VOID core_platform_backing_resource_set_result(
    core_platform_backing_resource_result *out_result,
    core_platform_backing_resource_result result)
{
    if (out_result != STD_NULL) *out_result = result;
}

static type_status core_platform_backing_resource_check(
    const core_platform_backing_resource *resource,
    core_platform_backing_resource_result *out_result)
{
    if (out_result == STD_NULL) return TYPE_STATUS_INVALID_ARGUMENT;
    if (resource == STD_NULL || resource->provider == STD_NULL)
        return TYPE_STATUS_INVALID_ARGUMENT;
    if (resource->closed) {
        core_platform_backing_resource_set_result(out_result,
            CORE_PLATFORM_BACKING_RESOURCE_CLOSED);
        return TYPE_STATUS_INVALID_STATE;
    }
    return TYPE_STATUS_OK;
}

C_VOID core_platform_backing_resource_initialize(
    core_platform_backing_resource *resource, C_VOID *context,
    const core_platform_backing_resource_provider *provider)
{
    if (resource == STD_NULL) return;
    resource->context = context;
    resource->provider = provider;
    resource->closed = TYPE_FALSE;
}

type_status core_platform_backing_resource_size(
    const core_platform_backing_resource *resource, uint64_t *out_size,
    core_platform_backing_resource_result *out_result)
{
    type_status status;

    if (out_size == STD_NULL) return TYPE_STATUS_INVALID_ARGUMENT;
    *out_size = 0u;
    status = core_platform_backing_resource_check(resource, out_result);
    if (status != TYPE_STATUS_OK) return status;
    if (resource->provider->size == STD_NULL) {
        core_platform_backing_resource_set_result(out_result,
            CORE_PLATFORM_BACKING_RESOURCE_UNSUPPORTED);
        return TYPE_STATUS_OK;
    }
    core_platform_backing_resource_set_result(out_result,
        resource->provider->size(resource->context, out_size));
    return TYPE_STATUS_OK;
}

static type_status core_platform_backing_resource_transfer(
    const core_platform_backing_resource *resource, uint64_t offset,
    C_VOID *buffer, uint32_t requested, uint32_t *out_transferred,
    core_platform_backing_resource_result *out_result, type_bool write)
{
    type_status status;
    core_platform_backing_resource_result result;

    if (buffer == STD_NULL || requested == 0u || out_transferred == STD_NULL)
        return TYPE_STATUS_INVALID_ARGUMENT;
    *out_transferred = 0u;
    status = core_platform_backing_resource_check(resource, out_result);
    if (status != TYPE_STATUS_OK) return status;
    if ((!write && resource->provider->read == STD_NULL) ||
        (write && resource->provider->write == STD_NULL)) {
        core_platform_backing_resource_set_result(out_result,
            CORE_PLATFORM_BACKING_RESOURCE_UNSUPPORTED);
        return TYPE_STATUS_OK;
    }
    result = write ? resource->provider->write(resource->context, offset, buffer,
        requested, out_transferred) : resource->provider->read(resource->context,
        offset, buffer, requested, out_transferred);
    if (*out_transferred > requested) return TYPE_STATUS_FAULT;
    core_platform_backing_resource_set_result(out_result, result);
    return TYPE_STATUS_OK;
}

type_status core_platform_backing_resource_read(
    const core_platform_backing_resource *resource, uint64_t offset,
    C_VOID *buffer, uint32_t requested, uint32_t *out_transferred,
    core_platform_backing_resource_result *out_result)
{
    return core_platform_backing_resource_transfer(resource, offset, buffer,
        requested, out_transferred, out_result, TYPE_FALSE);
}

type_status core_platform_backing_resource_write(
    const core_platform_backing_resource *resource, uint64_t offset,
    const C_VOID *buffer, uint32_t requested, uint32_t *out_transferred,
    core_platform_backing_resource_result *out_result)
{
    return core_platform_backing_resource_transfer(resource, offset, (C_VOID *)buffer,
        requested, out_transferred, out_result, TYPE_TRUE);
}

type_status core_platform_backing_resource_flush(
    const core_platform_backing_resource *resource,
    core_platform_backing_resource_result *out_result)
{
    type_status status = core_platform_backing_resource_check(resource, out_result);

    if (status != TYPE_STATUS_OK) return status;
    if (resource->provider->flush == STD_NULL) {
        core_platform_backing_resource_set_result(out_result,
            CORE_PLATFORM_BACKING_RESOURCE_UNSUPPORTED);
        return TYPE_STATUS_OK;
    }
    core_platform_backing_resource_set_result(out_result,
        resource->provider->flush(resource->context));
    return TYPE_STATUS_OK;
}

type_status core_platform_backing_resource_close(core_platform_backing_resource *resource,
    core_platform_backing_resource_result *out_result)
{
    core_platform_backing_resource_result result;

    if (out_result == STD_NULL || resource == STD_NULL || resource->provider == STD_NULL)
        return TYPE_STATUS_INVALID_ARGUMENT;
    if (resource->closed) {
        core_platform_backing_resource_set_result(out_result,
            CORE_PLATFORM_BACKING_RESOURCE_CLOSED);
        return TYPE_STATUS_INVALID_STATE;
    }
    resource->closed = TYPE_TRUE;
    result = resource->provider->close == STD_NULL ?
        CORE_PLATFORM_BACKING_RESOURCE_OK : resource->provider->close(resource->context);
    core_platform_backing_resource_set_result(out_result, result);
    return TYPE_STATUS_OK;
}
