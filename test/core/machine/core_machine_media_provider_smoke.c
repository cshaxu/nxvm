#include "type.h"

#include "core/machine/media_interface.h"

typedef struct core_machine_media_fixture {
    type_unsigned_8 bytes[16];
    core_machine_media_address_mark marks[4];
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
        CORE_MACHINE_MEDIA_CAPABILITY_FORMATTABLE |
        CORE_MACHINE_MEDIA_CAPABILITY_ADDRESS_MARKS;
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

static core_machine_media_result core_machine_media_fixture_get_address_mark(
    C_VOID *context, type_unsigned_64 logical_sector,
    core_machine_media_address_mark *out_mark)
{
    core_machine_media_fixture *fixture = (core_machine_media_fixture *)context;

    if (fixture == STD_NULL || !fixture->present) return CORE_MACHINE_MEDIA_RESULT_ABSENT;
    if (out_mark == STD_NULL || logical_sector >= 4u)
        return CORE_MACHINE_MEDIA_RESULT_INVALID_RANGE;
    *out_mark = fixture->marks[logical_sector];
    return CORE_MACHINE_MEDIA_RESULT_OK;
}

static core_machine_media_result core_machine_media_fixture_set_address_mark(
    C_VOID *context, type_unsigned_64 logical_sector,
    core_machine_media_address_mark mark)
{
    core_machine_media_fixture *fixture = (core_machine_media_fixture *)context;

    if (fixture == STD_NULL || !fixture->present) return CORE_MACHINE_MEDIA_RESULT_ABSENT;
    if (fixture->read_only) return CORE_MACHINE_MEDIA_RESULT_READ_ONLY;
    if (logical_sector >= 4u || (mark != CORE_MACHINE_MEDIA_ADDRESS_MARK_DATA &&
        mark != CORE_MACHINE_MEDIA_ADDRESS_MARK_DELETED_DATA))
        return CORE_MACHINE_MEDIA_RESULT_INVALID_RANGE;
    fixture->marks[logical_sector] = mark;
    ++fixture->generation;
    return CORE_MACHINE_MEDIA_RESULT_OK;
}

static core_machine_media_result core_machine_media_fixture_get_invalid_address_mark(
    C_VOID *context, type_unsigned_64 logical_sector,
    core_machine_media_address_mark *out_mark)
{
    core_machine_media_result result = core_machine_media_fixture_get_address_mark(
        context, logical_sector, out_mark);

    if (result == CORE_MACHINE_MEDIA_RESULT_OK) {
        *out_mark = (core_machine_media_address_mark)2;
    }
    return result;
}

C_INT main(C_VOID)
{
    static const core_machine_media_provider provider = {
        core_machine_media_fixture_query,
        core_machine_media_fixture_read,
        core_machine_media_fixture_write,
        core_machine_media_fixture_format,
        core_machine_media_fixture_flush,
        core_machine_media_fixture_get_address_mark,
        core_machine_media_fixture_set_address_mark
    };
    static const core_machine_media_provider unsupported_provider = {
        core_machine_media_fixture_query,
        core_machine_media_fixture_read,
        core_machine_media_fixture_write,
        core_machine_media_fixture_format,
        core_machine_media_fixture_flush,
        STD_NULL,
        STD_NULL
    };
    static const core_machine_media_provider invalid_provider = {
        core_machine_media_fixture_query,
        core_machine_media_fixture_read,
        core_machine_media_fixture_write,
        core_machine_media_fixture_format,
        core_machine_media_fixture_flush,
        core_machine_media_fixture_get_invalid_address_mark,
        core_machine_media_fixture_set_address_mark
    };
    core_machine_media_registry *registry = STD_NULL;
    core_machine_media_fixture first = {{0}, {CORE_MACHINE_MEDIA_ADDRESS_MARK_DATA}, 7u, 0u, TYPE_TRUE, TYPE_FALSE,
        CORE_MACHINE_MEDIA_RESULT_OK};
    core_machine_media_fixture second = {{0}, {CORE_MACHINE_MEDIA_ADDRESS_MARK_DATA}, 3u, 0u, TYPE_FALSE, TYPE_FALSE,
        CORE_MACHINE_MEDIA_RESULT_OK};
    core_machine_media_fixture unsupported = {{0}, {CORE_MACHINE_MEDIA_ADDRESS_MARK_DATA}, 4u, 0u, TYPE_TRUE, TYPE_FALSE,
        CORE_MACHINE_MEDIA_RESULT_OK};
    core_machine_media_fixture invalid = {{0}, {CORE_MACHINE_MEDIA_ADDRESS_MARK_DATA}, 5u, 0u, TYPE_TRUE, TYPE_FALSE,
        CORE_MACHINE_MEDIA_RESULT_OK};
    core_machine_media_info info;
    core_machine_media_result result = CORE_MACHINE_MEDIA_RESULT_PERMANENT;
    core_machine_media_address_mark mark = CORE_MACHINE_MEDIA_ADDRESS_MARK_DATA;
    type_unsigned_8 bytes[4] = {1u, 2u, 3u, 4u};
    type_unsigned_8 readback[4] = {0};
    C_INT failed = 0;

    if (core_machine_media_registry_create(&registry) != TYPE_STATUS_OK ||
        core_machine_media_query(registry, 1u, &info, &result) !=
            TYPE_STATUS_INVALID_STATE ||
        core_machine_media_registry_bind(registry, 1u, &first, &provider) !=
            TYPE_STATUS_OK ||
        core_machine_media_registry_bind(registry, 2u, &second, &provider) !=
            TYPE_STATUS_OK ||
        core_machine_media_registry_bind(registry, 3u, &unsupported, &unsupported_provider) !=
            TYPE_STATUS_OK ||
        core_machine_media_registry_bind(registry, 4u, &invalid, &invalid_provider) !=
            TYPE_STATUS_OK ||
        core_machine_media_registry_bind(registry, 1u, &first, &provider) !=
            TYPE_STATUS_INVALID_ARGUMENT ||
        core_machine_media_registry_freeze(registry) != TYPE_STATUS_OK ||
        core_machine_media_registry_bind(registry, 3u, &first, &provider) !=
            TYPE_STATUS_INVALID_STATE) failed = 1;
    if (!failed && (core_machine_media_query(registry, 1u, &info, &result) !=
            TYPE_STATUS_OK || result != CORE_MACHINE_MEDIA_RESULT_OK ||
        info.generation != 7u || !info.present || info.geometry.bytes_per_sector != 4u ||
        core_machine_media_query(registry, 2u, &info, &result) != TYPE_STATUS_OK ||
        result != CORE_MACHINE_MEDIA_RESULT_ABSENT)) failed = 1;
    if (!failed && (core_machine_media_get_address_mark(registry, 1u, 1u,
            &mark, &result) != TYPE_STATUS_OK ||
        result != CORE_MACHINE_MEDIA_RESULT_OK ||
        mark != CORE_MACHINE_MEDIA_ADDRESS_MARK_DATA ||
        core_machine_media_set_address_mark(registry, 1u, 1u,
            CORE_MACHINE_MEDIA_ADDRESS_MARK_DELETED_DATA, &result) != TYPE_STATUS_OK ||
        result != CORE_MACHINE_MEDIA_RESULT_OK || first.generation != 8u ||
        core_machine_media_get_address_mark(registry, 1u, 1u,
            &mark, &result) != TYPE_STATUS_OK ||
        result != CORE_MACHINE_MEDIA_RESULT_OK ||
        mark != CORE_MACHINE_MEDIA_ADDRESS_MARK_DELETED_DATA ||
        core_machine_media_get_address_mark(registry, 3u, 0u,
            &mark, &result) != TYPE_STATUS_OK ||
        result != CORE_MACHINE_MEDIA_RESULT_UNSUPPORTED ||
        core_machine_media_get_address_mark(registry, 4u, 0u,
            &mark, &result) != TYPE_STATUS_OK ||
        result != CORE_MACHINE_MEDIA_RESULT_PERMANENT)) failed = 1;
    if (!failed && (core_machine_media_write_sectors(registry, 1u, 1u, 1u,
            bytes, &result) != TYPE_STATUS_OK || result != CORE_MACHINE_MEDIA_RESULT_OK ||
        core_machine_media_read_bytes(registry, 1u, 4u, readback, sizeof(readback),
            &result) != TYPE_STATUS_OK || result != CORE_MACHINE_MEDIA_RESULT_OK ||
        STD_MEMCMP(bytes, readback, sizeof(bytes)) != 0)) failed = 1;
    if (!failed && (core_machine_media_format_sectors(registry, 1u, 1u, 1u,
            0xa5u, &result) != TYPE_STATUS_OK || result != CORE_MACHINE_MEDIA_RESULT_OK ||
        first.generation != 9u || first.bytes[4] != 0xa5u ||
        core_machine_media_flush(registry, 1u, &result) != TYPE_STATUS_OK ||
        result != CORE_MACHINE_MEDIA_RESULT_OK || first.flush_count != 1u)) failed = 1;
    first.read_only = TYPE_TRUE;
    if (!failed && (core_machine_media_write_bytes(registry, 1u, 0u, bytes,
            sizeof(bytes), &result) != TYPE_STATUS_OK ||
        result != CORE_MACHINE_MEDIA_RESULT_READ_ONLY ||
        core_machine_media_read_sectors(registry, 1u, 4u, 1u, readback,
            &result) != TYPE_STATUS_OK ||
        result != CORE_MACHINE_MEDIA_RESULT_INVALID_RANGE)) failed = 1;
    first.read_only = TYPE_FALSE;
    first.forced_read_result = CORE_MACHINE_MEDIA_RESULT_TRANSIENT;
    if (!failed && (core_machine_media_read_bytes(registry, 1u, 0u, readback,
            sizeof(readback), &result) != TYPE_STATUS_OK ||
        result != CORE_MACHINE_MEDIA_RESULT_TRANSIENT)) failed = 1;
    first.forced_read_result = CORE_MACHINE_MEDIA_RESULT_PERMANENT;
    if (!failed && (core_machine_media_read_bytes(registry, 1u, 0u, readback,
            sizeof(readback), &result) != TYPE_STATUS_OK ||
        result != CORE_MACHINE_MEDIA_RESULT_PERMANENT)) failed = 1;
    core_machine_media_registry_destroy(registry);
    if (failed) return 1;
    STD_PRINTF("M5:T270:S2:MEDIA-PROVIDER:OK\n");
    STD_PRINTF("M5:T374:S9:MEDIA-ADDRESS-MARK:OK\n");
    return 0;
}
