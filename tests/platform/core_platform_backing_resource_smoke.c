#include "type.h"

#include "core/platform/backing_resource_interface.h"

typedef struct core_platform_backing_resource_fixture {
    type_unsigned_8 bytes[8];
    type_unsigned_32 close_count;
    type_unsigned_32 flush_count;
    core_platform_backing_resource_result forced_flush_result;
} core_platform_backing_resource_fixture;

static core_platform_backing_resource_result core_platform_backing_resource_fixture_size(
    C_VOID *context, type_unsigned_64 *out_size)
{
    core_platform_backing_resource_fixture *fixture = context;

    *out_size = sizeof(fixture->bytes);
    return CORE_PLATFORM_BACKING_RESOURCE_OK;
}

static core_platform_backing_resource_result core_platform_backing_resource_fixture_read(
    C_VOID *context, type_unsigned_64 offset, C_VOID *buffer, type_unsigned_32 requested,
    type_unsigned_32 *out_transferred)
{
    core_platform_backing_resource_fixture *fixture = context;

    if (offset >= sizeof(fixture->bytes)) return CORE_PLATFORM_BACKING_RESOURCE_INVALID_RANGE;
    *out_transferred = requested > 2u ? 2u : requested;
    STD_MEMCPY(buffer, fixture->bytes + offset, *out_transferred);
    return *out_transferred == requested ? CORE_PLATFORM_BACKING_RESOURCE_OK :
        CORE_PLATFORM_BACKING_RESOURCE_SHORT;
}

static core_platform_backing_resource_result core_platform_backing_resource_fixture_write(
    C_VOID *context, type_unsigned_64 offset, const C_VOID *buffer, type_unsigned_32 requested,
    type_unsigned_32 *out_transferred)
{
    core_platform_backing_resource_fixture *fixture = context;

    if (offset > sizeof(fixture->bytes) || requested > sizeof(fixture->bytes) - offset)
        return CORE_PLATFORM_BACKING_RESOURCE_INVALID_RANGE;
    STD_MEMCPY(fixture->bytes + offset, buffer, requested);
    *out_transferred = requested;
    return CORE_PLATFORM_BACKING_RESOURCE_OK;
}

static core_platform_backing_resource_result core_platform_backing_resource_fixture_flush(
    C_VOID *context)
{
    core_platform_backing_resource_fixture *fixture = context;

    ++fixture->flush_count;
    return fixture->forced_flush_result;
}

static core_platform_backing_resource_result core_platform_backing_resource_fixture_close(
    C_VOID *context)
{
    ++((core_platform_backing_resource_fixture *)context)->close_count;
    return CORE_PLATFORM_BACKING_RESOURCE_OK;
}

C_INT main(C_VOID)
{
    static const core_platform_backing_resource_provider provider = {
        core_platform_backing_resource_fixture_size,
        core_platform_backing_resource_fixture_read,
        core_platform_backing_resource_fixture_write,
        core_platform_backing_resource_fixture_flush,
        core_platform_backing_resource_fixture_close
    };
    core_platform_backing_resource_fixture fixture = {{0xa5u, 0x5au}, 0u, 0u,
        CORE_PLATFORM_BACKING_RESOURCE_OK};
    core_platform_backing_resource resource;
    core_platform_backing_resource_result result;
    type_unsigned_8 write[4] = {1u, 2u, 3u, 4u};
    type_unsigned_8 read[4] = {0};
    type_unsigned_64 size = 0u;
    type_unsigned_32 transferred = 0u;
    C_INT failed = 0;

    core_platform_backing_resource_initialize(&resource, &fixture, &provider);
    if (core_platform_backing_resource_size(&resource, &size, &result) != TYPE_STATUS_OK ||
        result != CORE_PLATFORM_BACKING_RESOURCE_OK || size != sizeof(fixture.bytes) ||
        core_platform_backing_resource_read(&resource, 0u, read, sizeof(read),
            &transferred, &result) != TYPE_STATUS_OK ||
        result != CORE_PLATFORM_BACKING_RESOURCE_SHORT || transferred != 2u ||
        read[0] != 0xa5u || read[1] != 0x5au) failed = 1;
    if (!failed && (core_platform_backing_resource_write(&resource, 2u, write,
            sizeof(write), &transferred, &result) != TYPE_STATUS_OK ||
        result != CORE_PLATFORM_BACKING_RESOURCE_OK || transferred != sizeof(write) ||
        fixture.bytes[2] != 1u || fixture.bytes[5] != 4u ||
        core_platform_backing_resource_flush(&resource, &result) != TYPE_STATUS_OK ||
        result != CORE_PLATFORM_BACKING_RESOURCE_OK || fixture.flush_count != 1u)) failed = 1;
    fixture.forced_flush_result = CORE_PLATFORM_BACKING_RESOURCE_TRANSIENT;
    if (!failed && (core_platform_backing_resource_flush(&resource, &result) != TYPE_STATUS_OK ||
        result != CORE_PLATFORM_BACKING_RESOURCE_TRANSIENT || fixture.flush_count != 2u)) failed = 1;
    if (!failed && (core_platform_backing_resource_close(&resource, &result) != TYPE_STATUS_OK ||
        result != CORE_PLATFORM_BACKING_RESOURCE_OK || fixture.close_count != 1u ||
        core_platform_backing_resource_read(&resource, 0u, read, 1u, &transferred,
            &result) != TYPE_STATUS_INVALID_STATE ||
        result != CORE_PLATFORM_BACKING_RESOURCE_CLOSED ||
        core_platform_backing_resource_close(&resource, &result) != TYPE_STATUS_INVALID_STATE ||
        result != CORE_PLATFORM_BACKING_RESOURCE_CLOSED || fixture.close_count != 1u)) failed = 1;
    if (failed) return 1;
    STD_PRINTF("M5:T271:S2:BACKING-RESOURCE:OK\n");
    return 0;
}
