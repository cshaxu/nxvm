#include "type.h"

#include "core/machine/media_interface.h"

typedef struct core_machine_media_fixture {
    type_unsigned_8 bytes[16];
    type_unsigned_64 generation;
    type_unsigned_32 flush_count;
    type_bool present;
    type_bool read_only;
    core_machine_media_result forced_read_result;
} core_machine_media_fixture;

static core_machine_media_result core_machine_media_fixture_query(C_VOID *context,
    core_machine_media_info *out_info)
{
    core_machine_media_fixture *fixture = (core_machine_media_fixture *)context;

    if (fixture == STD_NULL || out_info == STD_NULL) return CORE_MACHINE_MEDIA_RESULT_PERMANENT;
    out_info->generation = fixture->generation;
    out_info->capabilities = CORE_MACHINE_MEDIA_CAPABILITY_REMOVABLE |
        CORE_MACHINE_MEDIA_CAPABILITY_FLUSHABLE |
        CORE_MACHINE_MEDIA_CAPABILITY_GEOMETRY_KNOWN |
        CORE_MACHINE_MEDIA_CAPABILITY_CHANGE_DETECTABLE |
        CORE_MACHINE_MEDIA_CAPABILITY_FORMATTABLE;
    if (fixture->read_only) out_info->capabilities |= CORE_MACHINE_MEDIA_CAPABILITY_READ_ONLY;
    out_info->present = fixture->present;
    out_info->geometry.logical_sector_count = 4u;
    out_info->geometry.bytes_per_sector = 4u;
    out_info->geometry.cylinders = 1u;
    out_info->geometry.heads = 1u;
    out_info->geometry.sectors_per_track = 4u;
    return fixture->present ? CORE_MACHINE_MEDIA_RESULT_OK : CORE_MACHINE_MEDIA_RESULT_ABSENT;
}

static core_machine_media_result core_machine_media_fixture_read(C_VOID *context,
    type_unsigned_64 offset, C_VOID *buffer, type_unsigned_32 byte_count)
{
    core_machine_media_fixture *fixture = (core_machine_media_fixture *)context;

    if (fixture == STD_NULL || !fixture->present) return CORE_MACHINE_MEDIA_RESULT_ABSENT;
    if (fixture->forced_read_result != CORE_MACHINE_MEDIA_RESULT_OK)
        return fixture->forced_read_result;
    if (offset > sizeof(fixture->bytes) || byte_count > sizeof(fixture->bytes) - offset)
        return CORE_MACHINE_MEDIA_RESULT_INVALID_RANGE;
    STD_MEMCPY(buffer, fixture->bytes + offset, byte_count);
    return CORE_MACHINE_MEDIA_RESULT_OK;
}

static core_machine_media_result core_machine_media_fixture_write(C_VOID *context,
    type_unsigned_64 offset, const C_VOID *buffer, type_unsigned_32 byte_count)
{
    core_machine_media_fixture *fixture = (core_machine_media_fixture *)context;

    if (fixture == STD_NULL || !fixture->present) return CORE_MACHINE_MEDIA_RESULT_ABSENT;
    if (fixture->read_only) return CORE_MACHINE_MEDIA_RESULT_READ_ONLY;
    if (offset > sizeof(fixture->bytes) || byte_count > sizeof(fixture->bytes) - offset)
        return CORE_MACHINE_MEDIA_RESULT_INVALID_RANGE;
    STD_MEMCPY(fixture->bytes + offset, buffer, byte_count);
    return CORE_MACHINE_MEDIA_RESULT_OK;
}

static core_machine_media_result core_machine_media_fixture_format(C_VOID *context,
    type_unsigned_64 logical_sector, type_unsigned_32 sector_count, type_unsigned_8 fill)
{
    core_machine_media_fixture *fixture = (core_machine_media_fixture *)context;
    type_unsigned_64 offset = logical_sector * 4u;
    type_unsigned_32 bytes = sector_count * 4u;

    if (fixture == STD_NULL || !fixture->present) return CORE_MACHINE_MEDIA_RESULT_ABSENT;
    if (fixture->read_only) return CORE_MACHINE_MEDIA_RESULT_READ_ONLY;
    if (logical_sector >= 4u || sector_count > 4u - logical_sector)
        return CORE_MACHINE_MEDIA_RESULT_INVALID_RANGE;
    STD_MEMSET(fixture->bytes + offset, fill, bytes);
    ++fixture->generation;
    return CORE_MACHINE_MEDIA_RESULT_OK;
}

static core_machine_media_result core_machine_media_fixture_flush(C_VOID *context)
{
    core_machine_media_fixture *fixture = (core_machine_media_fixture *)context;

    if (fixture == STD_NULL || !fixture->present) return CORE_MACHINE_MEDIA_RESULT_ABSENT;
    ++fixture->flush_count;
    return CORE_MACHINE_MEDIA_RESULT_OK;
}

C_INT main(C_VOID)
{
    static const core_machine_media_provider provider = {
        core_machine_media_fixture_query,
        core_machine_media_fixture_read,
        core_machine_media_fixture_write,
        core_machine_media_fixture_format,
        core_machine_media_fixture_flush
    };
    core_machine_media_registry registry;
    core_machine_media_fixture first = {{0}, 7u, 0u, TYPE_TRUE, TYPE_FALSE,
        CORE_MACHINE_MEDIA_RESULT_OK};
    core_machine_media_fixture second = {{0}, 3u, 0u, TYPE_FALSE, TYPE_FALSE,
        CORE_MACHINE_MEDIA_RESULT_OK};
    core_machine_media_info info;
    core_machine_media_result result = CORE_MACHINE_MEDIA_RESULT_PERMANENT;
    type_unsigned_8 bytes[4] = {1u, 2u, 3u, 4u};
    type_unsigned_8 readback[4] = {0};
    C_INT failed = 0;

    core_machine_media_registry_initialize(&registry);
    if (core_machine_media_query(&registry, 1u, &info, &result) !=
            TYPE_STATUS_INVALID_STATE ||
        core_machine_media_registry_bind(&registry, 1u, &first, &provider) !=
            TYPE_STATUS_OK ||
        core_machine_media_registry_bind(&registry, 2u, &second, &provider) !=
            TYPE_STATUS_OK ||
        core_machine_media_registry_bind(&registry, 1u, &first, &provider) !=
            TYPE_STATUS_INVALID_ARGUMENT ||
        core_machine_media_registry_freeze(&registry) != TYPE_STATUS_OK ||
        core_machine_media_registry_bind(&registry, 3u, &first, &provider) !=
            TYPE_STATUS_INVALID_STATE) failed = 1;
    if (!failed && (core_machine_media_query(&registry, 1u, &info, &result) !=
            TYPE_STATUS_OK || result != CORE_MACHINE_MEDIA_RESULT_OK ||
        info.generation != 7u || !info.present || info.geometry.bytes_per_sector != 4u ||
        core_machine_media_query(&registry, 2u, &info, &result) != TYPE_STATUS_OK ||
        result != CORE_MACHINE_MEDIA_RESULT_ABSENT)) failed = 1;
    if (!failed && (core_machine_media_write_sectors(&registry, 1u, 1u, 1u,
            bytes, &result) != TYPE_STATUS_OK || result != CORE_MACHINE_MEDIA_RESULT_OK ||
        core_machine_media_read_bytes(&registry, 1u, 4u, readback, sizeof(readback),
            &result) != TYPE_STATUS_OK || result != CORE_MACHINE_MEDIA_RESULT_OK ||
        STD_MEMCMP(bytes, readback, sizeof(bytes)) != 0)) failed = 1;
    if (!failed && (core_machine_media_format_sectors(&registry, 1u, 1u, 1u,
            0xa5u, &result) != TYPE_STATUS_OK || result != CORE_MACHINE_MEDIA_RESULT_OK ||
        first.generation != 8u || first.bytes[4] != 0xa5u ||
        core_machine_media_flush(&registry, 1u, &result) != TYPE_STATUS_OK ||
        result != CORE_MACHINE_MEDIA_RESULT_OK || first.flush_count != 1u)) failed = 1;
    first.read_only = TYPE_TRUE;
    if (!failed && (core_machine_media_write_bytes(&registry, 1u, 0u, bytes,
            sizeof(bytes), &result) != TYPE_STATUS_OK ||
        result != CORE_MACHINE_MEDIA_RESULT_READ_ONLY ||
        core_machine_media_read_sectors(&registry, 1u, 4u, 1u, readback,
            &result) != TYPE_STATUS_OK ||
        result != CORE_MACHINE_MEDIA_RESULT_INVALID_RANGE)) failed = 1;
    first.read_only = TYPE_FALSE;
    first.forced_read_result = CORE_MACHINE_MEDIA_RESULT_TRANSIENT;
    if (!failed && (core_machine_media_read_bytes(&registry, 1u, 0u, readback,
            sizeof(readback), &result) != TYPE_STATUS_OK ||
        result != CORE_MACHINE_MEDIA_RESULT_TRANSIENT)) failed = 1;
    first.forced_read_result = CORE_MACHINE_MEDIA_RESULT_PERMANENT;
    if (!failed && (core_machine_media_read_bytes(&registry, 1u, 0u, readback,
            sizeof(readback), &result) != TYPE_STATUS_OK ||
        result != CORE_MACHINE_MEDIA_RESULT_PERMANENT)) failed = 1;
    core_machine_media_registry_finalize(&registry);
    if (failed) return 1;
    STD_PRINTF("M5:T270:S2:MEDIA-PROVIDER:OK\n");
    return 0;
}
