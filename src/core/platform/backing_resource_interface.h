/* Opaque, synchronous host backing resource for composition-owned media. */
#ifndef CORE_PLATFORM_BACKING_RESOURCE_INTERFACE_H
#define CORE_PLATFORM_BACKING_RESOURCE_INTERFACE_H

#include "type.h"

typedef enum core_platform_backing_resource_result {
    CORE_PLATFORM_BACKING_RESOURCE_OK = 0,
    CORE_PLATFORM_BACKING_RESOURCE_SHORT,
    CORE_PLATFORM_BACKING_RESOURCE_READ_ONLY,
    CORE_PLATFORM_BACKING_RESOURCE_INVALID_RANGE,
    CORE_PLATFORM_BACKING_RESOURCE_TRANSIENT,
    CORE_PLATFORM_BACKING_RESOURCE_PERMANENT,
    CORE_PLATFORM_BACKING_RESOURCE_UNSUPPORTED,
    CORE_PLATFORM_BACKING_RESOURCE_CLOSED
} core_platform_backing_resource_result;

typedef core_platform_backing_resource_result (*core_platform_backing_resource_size_provider)(
    C_VOID *context, type_unsigned_64 *out_size);
typedef core_platform_backing_resource_result (*core_platform_backing_resource_read_provider)(
    C_VOID *context, type_unsigned_64 offset, C_VOID *buffer, type_unsigned_32 requested,
    type_unsigned_32 *out_transferred);
typedef core_platform_backing_resource_result (*core_platform_backing_resource_write_provider)(
    C_VOID *context, type_unsigned_64 offset, const C_VOID *buffer, type_unsigned_32 requested,
    type_unsigned_32 *out_transferred);
typedef core_platform_backing_resource_result (*core_platform_backing_resource_flush_provider)(
    C_VOID *context);
typedef core_platform_backing_resource_result (*core_platform_backing_resource_close_provider)(
    C_VOID *context);

typedef struct core_platform_backing_resource_provider {
    core_platform_backing_resource_size_provider size;
    core_platform_backing_resource_read_provider read;
    core_platform_backing_resource_write_provider write;
    core_platform_backing_resource_flush_provider flush;
    core_platform_backing_resource_close_provider close;
} core_platform_backing_resource_provider;

typedef struct core_platform_backing_resource {
    C_VOID *context;
    const core_platform_backing_resource_provider *provider;
    type_bool closed;
} core_platform_backing_resource;

C_VOID core_platform_backing_resource_initialize(
    core_platform_backing_resource *resource, C_VOID *context,
    const core_platform_backing_resource_provider *provider);
type_status core_platform_backing_resource_size(
    const core_platform_backing_resource *resource, type_unsigned_64 *out_size,
    core_platform_backing_resource_result *out_result);
type_status core_platform_backing_resource_read(
    const core_platform_backing_resource *resource, type_unsigned_64 offset,
    C_VOID *buffer, type_unsigned_32 requested, type_unsigned_32 *out_transferred,
    core_platform_backing_resource_result *out_result);
type_status core_platform_backing_resource_write(
    const core_platform_backing_resource *resource, type_unsigned_64 offset,
    const C_VOID *buffer, type_unsigned_32 requested, type_unsigned_32 *out_transferred,
    core_platform_backing_resource_result *out_result);
type_status core_platform_backing_resource_flush(
    const core_platform_backing_resource *resource,
    core_platform_backing_resource_result *out_result);
type_status core_platform_backing_resource_close(core_platform_backing_resource *resource,
    core_platform_backing_resource_result *out_result);

#endif
