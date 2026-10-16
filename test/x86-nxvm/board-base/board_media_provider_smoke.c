#include "lib/types/types_interface.h"
#include "lib/types/file.h"

#include "x86-nxvm/board-base/media_interface.h"

typedef struct x86_nxvm_test_media_fixture {
    lib_u8 bytes[2048];
    x86_nxvm_board_media_address_mark marks[4];
    lib_u64 generation;
    lib_u32 flush_count;
    lib_u8 present;
    lib_u8 read_only;
    x86_nxvm_board_media_result forced_read_result;
    lib_u32 bytes_per_sector;
    lib_u32 reads;
    lib_u32 writes;
} x86_nxvm_test_media_fixture;

static x86_nxvm_board_media_result x86_nxvm_test_media_fixture_query(void *context,
    x86_nxvm_board_media_info *out_info)
{
    x86_nxvm_test_media_fixture *fixture = (x86_nxvm_test_media_fixture *)context;

    if (fixture == LIB_NULL || out_info == LIB_NULL) return X86_NXVM_BOARD_MEDIA_RESULT_PERMANENT;
    out_info->generation = fixture->generation;
    out_info->capabilities = X86_NXVM_BOARD_MEDIA_CAPABILITY_REMOVABLE |
        X86_NXVM_BOARD_MEDIA_CAPABILITY_FLUSHABLE |
        X86_NXVM_BOARD_MEDIA_CAPABILITY_GEOMETRY_KNOWN |
        X86_NXVM_BOARD_MEDIA_CAPABILITY_CHANGE_DETECTABLE |
        X86_NXVM_BOARD_MEDIA_CAPABILITY_FORMATTABLE |
        X86_NXVM_BOARD_MEDIA_CAPABILITY_ADDRESS_MARKS;
    if (fixture->read_only) out_info->capabilities |= X86_NXVM_BOARD_MEDIA_CAPABILITY_READ_ONLY;
    out_info->present = fixture->present;
    out_info->geometry.logical_sector_count = 4u;
    out_info->geometry.bytes_per_sector = fixture->bytes_per_sector != 0u ?
        fixture->bytes_per_sector : 4u;
    out_info->geometry.cylinders = 1u;
    out_info->geometry.heads = 1u;
    out_info->geometry.sectors_per_track = 4u;
    return fixture->present ? X86_NXVM_BOARD_MEDIA_RESULT_OK : X86_NXVM_BOARD_MEDIA_RESULT_ABSENT;
}

static x86_nxvm_board_media_result x86_nxvm_test_media_fixture_read(void *context,
    lib_u64 offset, void *buffer, lib_u32 byte_count)
{
    x86_nxvm_test_media_fixture *fixture = (x86_nxvm_test_media_fixture *)context;

    if (fixture == LIB_NULL || !fixture->present) return X86_NXVM_BOARD_MEDIA_RESULT_ABSENT;
    ++fixture->reads;
    if (fixture->forced_read_result != X86_NXVM_BOARD_MEDIA_RESULT_OK) {
        if (buffer != LIB_NULL && byte_count != 0u) ((lib_u8 *)buffer)[0] = 0xedu;
        return fixture->forced_read_result;
    }
    if (offset > sizeof(fixture->bytes) || byte_count > sizeof(fixture->bytes) - offset)
        return X86_NXVM_BOARD_MEDIA_RESULT_INVALID_RANGE;
    lib_memory_copy(buffer, fixture->bytes + offset, byte_count);
    return X86_NXVM_BOARD_MEDIA_RESULT_OK;
}

static x86_nxvm_board_media_result x86_nxvm_test_media_fixture_write(void *context,
    lib_u64 offset, const void *buffer, lib_u32 byte_count)
{
    x86_nxvm_test_media_fixture *fixture = (x86_nxvm_test_media_fixture *)context;

    if (fixture == LIB_NULL || !fixture->present) return X86_NXVM_BOARD_MEDIA_RESULT_ABSENT;
    if (fixture->read_only) return X86_NXVM_BOARD_MEDIA_RESULT_READ_ONLY;
    ++fixture->writes;
    if (offset > sizeof(fixture->bytes) || byte_count > sizeof(fixture->bytes) - offset)
        return X86_NXVM_BOARD_MEDIA_RESULT_INVALID_RANGE;
    lib_memory_copy(fixture->bytes + offset, buffer, byte_count);
    return X86_NXVM_BOARD_MEDIA_RESULT_OK;
}

static x86_nxvm_board_media_result x86_nxvm_test_media_fixture_format(void *context,
    lib_u64 logical_sector, lib_u32 sector_count, lib_u8 fill)
{
    x86_nxvm_test_media_fixture *fixture = (x86_nxvm_test_media_fixture *)context;
    lib_u64 offset = logical_sector * 4u;
    lib_u32 bytes = sector_count * 4u;

    if (fixture == LIB_NULL || !fixture->present) return X86_NXVM_BOARD_MEDIA_RESULT_ABSENT;
    if (fixture->read_only) return X86_NXVM_BOARD_MEDIA_RESULT_READ_ONLY;
    if (logical_sector >= 4u || sector_count > 4u - logical_sector)
        return X86_NXVM_BOARD_MEDIA_RESULT_INVALID_RANGE;
    lib_memory_set(fixture->bytes + offset, fill, bytes);
    ++fixture->generation;
    return X86_NXVM_BOARD_MEDIA_RESULT_OK;
}

static x86_nxvm_board_media_result x86_nxvm_test_media_fixture_flush(void *context)
{
    x86_nxvm_test_media_fixture *fixture = (x86_nxvm_test_media_fixture *)context;

    if (fixture == LIB_NULL || !fixture->present) return X86_NXVM_BOARD_MEDIA_RESULT_ABSENT;
    ++fixture->flush_count;
    return X86_NXVM_BOARD_MEDIA_RESULT_OK;
}

static x86_nxvm_board_media_result x86_nxvm_test_media_fixture_get_address_mark(
    void *context, lib_u64 logical_sector,
    x86_nxvm_board_media_address_mark *out_mark)
{
    x86_nxvm_test_media_fixture *fixture = (x86_nxvm_test_media_fixture *)context;

    if (fixture == LIB_NULL || !fixture->present) return X86_NXVM_BOARD_MEDIA_RESULT_ABSENT;
    if (out_mark == LIB_NULL || logical_sector >= 4u)
        return X86_NXVM_BOARD_MEDIA_RESULT_INVALID_RANGE;
    *out_mark = fixture->marks[logical_sector];
    return X86_NXVM_BOARD_MEDIA_RESULT_OK;
}

static x86_nxvm_board_media_result x86_nxvm_test_media_fixture_set_address_mark(
    void *context, lib_u64 logical_sector,
    x86_nxvm_board_media_address_mark mark)
{
    x86_nxvm_test_media_fixture *fixture = (x86_nxvm_test_media_fixture *)context;

    if (fixture == LIB_NULL || !fixture->present) return X86_NXVM_BOARD_MEDIA_RESULT_ABSENT;
    if (fixture->read_only) return X86_NXVM_BOARD_MEDIA_RESULT_READ_ONLY;
    if (logical_sector >= 4u || (mark != X86_NXVM_BOARD_MEDIA_ADDRESS_MARK_DATA &&
        mark != X86_NXVM_BOARD_MEDIA_ADDRESS_MARK_DELETED_DATA))
        return X86_NXVM_BOARD_MEDIA_RESULT_INVALID_RANGE;
    fixture->marks[logical_sector] = mark;
    ++fixture->generation;
    return X86_NXVM_BOARD_MEDIA_RESULT_OK;
}

static x86_nxvm_board_media_result x86_nxvm_test_media_fixture_get_invalid_address_mark(
    void *context, lib_u64 logical_sector,
    x86_nxvm_board_media_address_mark *out_mark)
{
    x86_nxvm_board_media_result result = x86_nxvm_test_media_fixture_get_address_mark(
        context, logical_sector, out_mark);

    if (result == X86_NXVM_BOARD_MEDIA_RESULT_OK) {
        *out_mark = (x86_nxvm_board_media_address_mark)2;
    }
    return result;
}

static lib_i32 x86_nxvm_test_media_record_fallback(void)
{
    static const x86_nxvm_board_media_provider provider = {
        .query = x86_nxvm_test_media_fixture_query,
        .read_bytes = x86_nxvm_test_media_fixture_read,
        .write_bytes = x86_nxvm_test_media_fixture_write,
    };
    x86_nxvm_test_media_fixture fixture = {.present = LIB_TRUE, .bytes_per_sector = 512u};
    x86_nxvm_board_media_registry *registry = LIB_NULL;
    x86_nxvm_board_media_record record = {.data = {0xabu}, .ecc = {1u, 2u, 3u, 4u},
        .ecc_present = LIB_TRUE};
    const x86_nxvm_board_media_record saved = record;
    x86_nxvm_board_media_result result = X86_NXVM_BOARD_MEDIA_RESULT_PERMANENT;
    lib_i32 failed = 0;

    fixture.bytes[512] = 0x5au;
    if (x86_nxvm_board_media_registry_create(&registry) != LIB_STATUS_OK) return 1;
    if (x86_nxvm_board_media_registry_bind(registry, 1u, &fixture, &provider) != LIB_STATUS_OK ||
        x86_nxvm_board_media_registry_freeze(registry) != LIB_STATUS_OK) failed = 1;
    if (!failed && (x86_nxvm_board_media_write_record(registry, 1u, 1u, &record,
            &result) != LIB_STATUS_OK || result != X86_NXVM_BOARD_MEDIA_RESULT_UNSUPPORTED ||
        fixture.writes != 0u || fixture.bytes[512] != 0x5au)) failed = 1;
    if (!failed && (x86_nxvm_board_media_read_record(registry, 1u, 1u, &record,
            &result) != LIB_STATUS_OK || result != X86_NXVM_BOARD_MEDIA_RESULT_OK ||
        fixture.reads != 1u || record.data[0] != 0x5au || record.ecc_present ||
        record.ecc[0] != 0u || record.ecc[3] != 0u)) failed = 1;
    record.data[0] = 0x6bu;
    if (!failed && (x86_nxvm_board_media_write_record(registry, 1u, 1u, &record,
            &result) != LIB_STATUS_OK || result != X86_NXVM_BOARD_MEDIA_RESULT_OK ||
        fixture.writes != 1u || fixture.bytes[512] != 0x6bu || fixture.bytes[0] != 0u))
        failed = 1;
    record = saved;
    fixture.forced_read_result = X86_NXVM_BOARD_MEDIA_RESULT_TRANSIENT;
    if (!failed && (x86_nxvm_board_media_read_record(registry, 1u, 1u, &record,
            &result) != LIB_STATUS_OK || result != X86_NXVM_BOARD_MEDIA_RESULT_TRANSIENT ||
        fixture.reads != 2u || !record.ecc_present ||
        lib_memory_compare(record.data, saved.data, sizeof(record.data)) != 0 ||
        lib_memory_compare(record.ecc, saved.ecc, sizeof(record.ecc)) != 0))
        failed = 1;
    fixture.forced_read_result = X86_NXVM_BOARD_MEDIA_RESULT_OK;
    if (!failed && (x86_nxvm_board_media_read_record(registry, 1u, 4u, &record,
            &result) != LIB_STATUS_OK || result != X86_NXVM_BOARD_MEDIA_RESULT_INVALID_RANGE ||
        fixture.reads != 2u || !record.ecc_present ||
        lib_memory_compare(record.data, saved.data, sizeof(record.data)) != 0 ||
        lib_memory_compare(record.ecc, saved.ecc, sizeof(record.ecc)) != 0))
        failed = 1;
    fixture.read_only = LIB_TRUE;
    record.ecc_present = LIB_FALSE;
    if (!failed && (x86_nxvm_board_media_write_record(registry, 1u, 1u, &record,
            &result) != LIB_STATUS_OK || result != X86_NXVM_BOARD_MEDIA_RESULT_READ_ONLY ||
        fixture.writes != 1u || fixture.bytes[512] != 0x6bu)) failed = 1;
    fixture.present = LIB_FALSE;
    record = saved;
    if (!failed && (x86_nxvm_board_media_read_record(registry, 1u, 1u, &record,
            &result) != LIB_STATUS_OK || result != X86_NXVM_BOARD_MEDIA_RESULT_ABSENT ||
        fixture.reads != 2u || !record.ecc_present ||
        lib_memory_compare(record.data, saved.data, sizeof(record.data)) != 0 ||
        lib_memory_compare(record.ecc, saved.ecc, sizeof(record.ecc)) != 0))
        failed = 1;
    x86_nxvm_board_media_registry_destroy(registry);
    return failed;
}

lib_i32 main(void)
{
    static const x86_nxvm_board_media_provider provider = {
        .query = x86_nxvm_test_media_fixture_query,
        .read_bytes = x86_nxvm_test_media_fixture_read,
        .write_bytes = x86_nxvm_test_media_fixture_write,
        .format_sectors = x86_nxvm_test_media_fixture_format,
        .flush = x86_nxvm_test_media_fixture_flush,
        .get_address_mark = x86_nxvm_test_media_fixture_get_address_mark,
        .set_address_mark = x86_nxvm_test_media_fixture_set_address_mark,
    };
    static const x86_nxvm_board_media_provider unsupported_provider = {
        .query = x86_nxvm_test_media_fixture_query,
        .read_bytes = x86_nxvm_test_media_fixture_read,
        .write_bytes = x86_nxvm_test_media_fixture_write,
        .format_sectors = x86_nxvm_test_media_fixture_format,
        .flush = x86_nxvm_test_media_fixture_flush,
    };
    static const x86_nxvm_board_media_provider invalid_provider = {
        .query = x86_nxvm_test_media_fixture_query,
        .read_bytes = x86_nxvm_test_media_fixture_read,
        .write_bytes = x86_nxvm_test_media_fixture_write,
        .format_sectors = x86_nxvm_test_media_fixture_format,
        .flush = x86_nxvm_test_media_fixture_flush,
        .get_address_mark = x86_nxvm_test_media_fixture_get_invalid_address_mark,
        .set_address_mark = x86_nxvm_test_media_fixture_set_address_mark,
    };
    x86_nxvm_board_media_registry *registry = LIB_NULL;
    x86_nxvm_test_media_fixture first = {.generation = 7u, .present = LIB_TRUE};
    x86_nxvm_test_media_fixture second = {.generation = 3u};
    x86_nxvm_test_media_fixture unsupported = {.generation = 4u, .present = LIB_TRUE};
    x86_nxvm_test_media_fixture invalid = {.generation = 5u, .present = LIB_TRUE};
    x86_nxvm_board_media_info info;
    x86_nxvm_board_media_result result = X86_NXVM_BOARD_MEDIA_RESULT_PERMANENT;
    x86_nxvm_board_media_address_mark mark = X86_NXVM_BOARD_MEDIA_ADDRESS_MARK_DATA;
    lib_u8 bytes[4] = {1u, 2u, 3u, 4u};
    lib_u8 readback[4] = {0};
    x86_nxvm_board_media_record record = {.data = {0xabu}, .ecc_present = LIB_TRUE};
    lib_i32 failed = 0;

    if (x86_nxvm_board_media_registry_create(&registry) != LIB_STATUS_OK ||
        x86_nxvm_board_media_query(registry, 1u, &info, &result) !=
            LIB_STATUS_INVALID_STATE ||
        x86_nxvm_board_media_registry_bind(registry, 1u, &first, &provider) !=
            LIB_STATUS_OK ||
        x86_nxvm_board_media_registry_bind(registry, 2u, &second, &provider) !=
            LIB_STATUS_OK ||
        x86_nxvm_board_media_registry_bind(registry, 3u, &unsupported, &unsupported_provider) !=
            LIB_STATUS_OK ||
        x86_nxvm_board_media_registry_bind(registry, 4u, &invalid, &invalid_provider) !=
            LIB_STATUS_OK ||
        x86_nxvm_board_media_registry_bind(registry, 1u, &first, &provider) !=
            LIB_STATUS_INVALID_ARGUMENT ||
        x86_nxvm_board_media_registry_freeze(registry) != LIB_STATUS_OK ||
        x86_nxvm_board_media_registry_bind(registry, 3u, &first, &provider) !=
            LIB_STATUS_INVALID_STATE) failed = 1;
    if (!failed && (x86_nxvm_board_media_query(registry, 1u, &info, &result) !=
            LIB_STATUS_OK || result != X86_NXVM_BOARD_MEDIA_RESULT_OK ||
        info.generation != 7u || !info.present || info.geometry.bytes_per_sector != 4u ||
        x86_nxvm_board_media_query(registry, 2u, &info, &result) != LIB_STATUS_OK ||
        result != X86_NXVM_BOARD_MEDIA_RESULT_ABSENT)) failed = 1;
    if (!failed && (x86_nxvm_board_media_read_record(registry, 1u, 0u,
            &record, &result) != LIB_STATUS_OK ||
        result != X86_NXVM_BOARD_MEDIA_RESULT_INVALID_RANGE ||
        record.data[0] != 0xabu || !record.ecc_present ||
        x86_nxvm_board_media_write_record(registry, 1u, 0u,
            &record, &result) != LIB_STATUS_OK ||
        result != X86_NXVM_BOARD_MEDIA_RESULT_INVALID_RANGE || first.bytes[0] != 0u))
        failed = 1;
    if (!failed && (x86_nxvm_board_media_get_address_mark(registry, 1u, 1u,
            &mark, &result) != LIB_STATUS_OK ||
        result != X86_NXVM_BOARD_MEDIA_RESULT_OK ||
        mark != X86_NXVM_BOARD_MEDIA_ADDRESS_MARK_DATA ||
        x86_nxvm_board_media_set_address_mark(registry, 1u, 1u,
            X86_NXVM_BOARD_MEDIA_ADDRESS_MARK_DELETED_DATA, &result) != LIB_STATUS_OK ||
        result != X86_NXVM_BOARD_MEDIA_RESULT_OK || first.generation != 8u ||
        x86_nxvm_board_media_get_address_mark(registry, 1u, 1u,
            &mark, &result) != LIB_STATUS_OK ||
        result != X86_NXVM_BOARD_MEDIA_RESULT_OK ||
        mark != X86_NXVM_BOARD_MEDIA_ADDRESS_MARK_DELETED_DATA ||
        x86_nxvm_board_media_get_address_mark(registry, 3u, 0u,
            &mark, &result) != LIB_STATUS_OK ||
        result != X86_NXVM_BOARD_MEDIA_RESULT_UNSUPPORTED ||
        x86_nxvm_board_media_get_address_mark(registry, 4u, 0u,
            &mark, &result) != LIB_STATUS_OK ||
        result != X86_NXVM_BOARD_MEDIA_RESULT_PERMANENT)) failed = 1;
    if (!failed && (x86_nxvm_board_media_write_sectors(registry, 1u, 1u, 1u,
            bytes, &result) != LIB_STATUS_OK || result != X86_NXVM_BOARD_MEDIA_RESULT_OK ||
        x86_nxvm_board_media_read_bytes(registry, 1u, 4u, readback, sizeof(readback),
            &result) != LIB_STATUS_OK || result != X86_NXVM_BOARD_MEDIA_RESULT_OK ||
        lib_memory_compare(bytes, readback, sizeof(bytes)) != 0)) failed = 1;
    if (!failed && (x86_nxvm_board_media_format_sectors(registry, 1u, 1u, 1u,
            0xa5u, &result) != LIB_STATUS_OK || result != X86_NXVM_BOARD_MEDIA_RESULT_OK ||
        first.generation != 9u || first.bytes[4] != 0xa5u ||
        x86_nxvm_board_media_flush(registry, 1u, &result) != LIB_STATUS_OK ||
        result != X86_NXVM_BOARD_MEDIA_RESULT_OK || first.flush_count != 1u)) failed = 1;
    first.read_only = LIB_TRUE;
    if (!failed && (x86_nxvm_board_media_write_bytes(registry, 1u, 0u, bytes,
            sizeof(bytes), &result) != LIB_STATUS_OK ||
        result != X86_NXVM_BOARD_MEDIA_RESULT_READ_ONLY ||
        x86_nxvm_board_media_read_sectors(registry, 1u, 4u, 1u, readback,
            &result) != LIB_STATUS_OK ||
        result != X86_NXVM_BOARD_MEDIA_RESULT_INVALID_RANGE)) failed = 1;
    first.read_only = LIB_FALSE;
    first.forced_read_result = X86_NXVM_BOARD_MEDIA_RESULT_TRANSIENT;
    if (!failed && (x86_nxvm_board_media_read_bytes(registry, 1u, 0u, readback,
            sizeof(readback), &result) != LIB_STATUS_OK ||
        result != X86_NXVM_BOARD_MEDIA_RESULT_TRANSIENT)) failed = 1;
    first.forced_read_result = X86_NXVM_BOARD_MEDIA_RESULT_PERMANENT;
    if (!failed && (x86_nxvm_board_media_read_bytes(registry, 1u, 0u, readback,
            sizeof(readback), &result) != LIB_STATUS_OK ||
        result != X86_NXVM_BOARD_MEDIA_RESULT_PERMANENT)) failed = 1;
    x86_nxvm_board_media_registry_destroy(registry);
    failed |= x86_nxvm_test_media_record_fallback();
    if (failed) return 1;
    lib_c_printf("MEDIA-PROVIDER:OK\n");
    lib_c_printf("MEDIA-ADDRESS-MARK:OK\n");
    return 0;
}
