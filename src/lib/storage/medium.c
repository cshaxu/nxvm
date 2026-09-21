#include "lib/types/types_interface.h"

#include "lib/storage/file.h"
#include "lib/storage/medium_interface.h"

#define LIB_STORAGE_MEDIUM_PAGE_BYTES 4096u

typedef struct lib_storage_medium_page {
    lib_u8 bytes[LIB_STORAGE_MEDIUM_PAGE_BYTES];
} lib_storage_medium_page;

struct lib_storage_medium {
    lib_storage_file file;
    lib_storage_medium_page **pages;
    lib_size byte_count;
    lib_storage_medium_mode mode;
};

static lib_size lib_storage_medium_page_count(lib_size byte_count)
{
    return byte_count / LIB_STORAGE_MEDIUM_PAGE_BYTES +
        (byte_count % LIB_STORAGE_MEDIUM_PAGE_BYTES != 0u);
}

static lib_status lib_storage_medium_set_size(lib_storage_medium *medium,
    lib_size byte_count)
{
    lib_size count = lib_storage_medium_page_count(byte_count);

    if (medium->mode == LIB_STORAGE_MEDIUM_OVERLAY && count != 0u) {
        if (count > (lib_size)-1 / sizeof(*medium->pages))
            return LIB_STATUS_LIMIT_EXCEEDED;
        medium->pages = lib_allocate_zero(count, sizeof(*medium->pages));
        if (medium->pages == LIB_NULL) return LIB_STATUS_NO_MEMORY;
    }
    medium->byte_count = byte_count;
    return LIB_STATUS_OK;
}

static lib_status lib_storage_medium_create(lib_size byte_count,
    lib_storage_medium_mode mode, lib_storage_medium **out_medium)
{
    lib_storage_medium *medium;
    lib_status status;

    if (out_medium == LIB_NULL) return LIB_STATUS_INVALID_ARGUMENT;
    *out_medium = LIB_NULL;
    medium = lib_allocate_zero(1u, sizeof(*medium));
    if (medium == LIB_NULL) return LIB_STATUS_NO_MEMORY;
    medium->mode = mode;
    status = lib_storage_medium_set_size(medium, byte_count);
    if (status != LIB_STATUS_OK) {
        lib_release(medium);
        return status;
    }
    *out_medium = medium;
    return LIB_STATUS_OK;
}

lib_status lib_storage_medium_open(const char *path, lib_storage_medium_mode mode,
    lib_storage_medium **out_medium)
{
    lib_storage_medium *medium;
    lib_i64 length;
    lib_status status;

    if (out_medium == LIB_NULL) return LIB_STATUS_INVALID_ARGUMENT;
    *out_medium = LIB_NULL;
    if (path == LIB_NULL || mode < LIB_STORAGE_MEDIUM_DIRECT ||
        mode > LIB_STORAGE_MEDIUM_OVERLAY)
        return LIB_STATUS_INVALID_ARGUMENT;
    status = lib_storage_medium_create(0u, mode, &medium);
    if (status != LIB_STATUS_OK) return status;
    status = storage_file_platform_open(path, mode == LIB_STORAGE_MEDIUM_DIRECT,
        &medium->file);
    if (status == LIB_STATUS_OK)
        status = lib_storage_file_byte_count(&medium->file, &length);
    if (status == LIB_STATUS_OK && (length < 0 || (lib_u64)length > (lib_u64)(lib_size)-1))
        status = LIB_STATUS_LIMIT_EXCEEDED;
    if (status == LIB_STATUS_OK)
        status = lib_storage_medium_set_size(medium, (lib_size)length);
    if (status != LIB_STATUS_OK) {
        (void)lib_storage_medium_destroy(&medium);
        return status;
    }
    *out_medium = medium;
    return LIB_STATUS_OK;
}

lib_status lib_storage_medium_create_overlay(const void *bytes, lib_size byte_count,
    lib_storage_medium **out_medium)
{
    lib_status status;

    if (out_medium == LIB_NULL) return LIB_STATUS_INVALID_ARGUMENT;
    *out_medium = LIB_NULL;
    if (byte_count != 0u && bytes == LIB_NULL) return LIB_STATUS_INVALID_ARGUMENT;
    status = lib_storage_medium_create(byte_count,
        LIB_STORAGE_MEDIUM_OVERLAY, out_medium);
    if (status != LIB_STATUS_OK || byte_count == 0u) return status;
    status = lib_storage_medium_write_at(*out_medium, 0u, bytes, byte_count);
    if (status != LIB_STATUS_OK) lib_storage_medium_destroy(out_medium);
    return status;
}

lib_status lib_storage_medium_create_zero_overlay(lib_size byte_count,
    lib_storage_medium **out_medium)
{
    return lib_storage_medium_create(byte_count,
        LIB_STORAGE_MEDIUM_OVERLAY, out_medium);
}

lib_status lib_storage_medium_destroy(lib_storage_medium **medium)
{
    lib_storage_medium *value;
    lib_status status;

    if (medium == LIB_NULL) return LIB_STATUS_INVALID_ARGUMENT;
    value = *medium;
    *medium = LIB_NULL;
    if (value == LIB_NULL) return LIB_STATUS_OK;
    status = lib_storage_file_close(&value->file);
    if (value->pages != LIB_NULL) {
        lib_size count = lib_storage_medium_page_count(value->byte_count);
        for (lib_size index = 0; index < count; ++index)
            lib_release(value->pages[index]);
        lib_release(value->pages);
    }
    lib_release(value);
    return status;
}

lib_size lib_storage_medium_byte_count(const lib_storage_medium *medium)
{ return medium == LIB_NULL ? 0u : medium->byte_count; }

static lib_status lib_storage_medium_range(const lib_storage_medium *medium,
    lib_size offset, lib_size byte_count, const void *bytes)
{
    if (medium == LIB_NULL || (byte_count != 0u && bytes == LIB_NULL) ||
        offset > medium->byte_count || byte_count > medium->byte_count - offset)
        return LIB_STATUS_INVALID_ARGUMENT;
    return LIB_STATUS_OK;
}

static lib_status lib_storage_medium_read_base(const lib_storage_medium *medium,
    lib_size offset, void *bytes, lib_size byte_count)
{
    if (byte_count == 0u) return LIB_STATUS_OK;
    if (medium->file.stream == LIB_NULL) {
        lib_memory_set(bytes, 0, byte_count);
        return LIB_STATUS_OK;
    }
    {
        lib_status status = lib_storage_file_seek_absolute(&medium->file, (lib_i64)offset);
        if (status == LIB_STATUS_OK)
            status = lib_storage_file_read_exact(&medium->file, bytes, byte_count);
        return status;
    }
}

static lib_status lib_storage_medium_page_for_write(lib_storage_medium *medium,
    lib_size index, lib_storage_medium_page **out_page)
{
    lib_storage_medium_page *page = medium->pages[index];
    lib_size offset;
    lib_size count;

    if (page != LIB_NULL) {
        *out_page = page;
        return LIB_STATUS_OK;
    }
    /* Caller range validation already bounds the derived page index. */
    page = lib_allocate(sizeof(*page));
    if (page == LIB_NULL) return LIB_STATUS_NO_MEMORY;
    offset = index * LIB_STORAGE_MEDIUM_PAGE_BYTES;
    count = medium->byte_count - offset;
    if (count > LIB_STORAGE_MEDIUM_PAGE_BYTES) count = LIB_STORAGE_MEDIUM_PAGE_BYTES;
    {
        lib_status status = lib_storage_medium_read_base(medium, offset, page->bytes, count);
        if (status != LIB_STATUS_OK) {
            lib_release(page);
            return status;
        }
    }
    if (count < LIB_STORAGE_MEDIUM_PAGE_BYTES)
        lib_memory_set(page->bytes + count, 0, LIB_STORAGE_MEDIUM_PAGE_BYTES - count);
    medium->pages[index] = page;
    *out_page = page;
    return LIB_STATUS_OK;
}

lib_status lib_storage_medium_read_at(const lib_storage_medium *medium,
    lib_size offset, void *bytes, lib_size byte_count)
{
    lib_u8 *cursor = bytes;

    if (lib_storage_medium_range(medium, offset, byte_count, bytes) != LIB_STATUS_OK)
        return LIB_STATUS_INVALID_ARGUMENT;
    if (medium->mode != LIB_STORAGE_MEDIUM_OVERLAY)
        return lib_storage_medium_read_base(medium, offset, bytes, byte_count);
    while (byte_count != 0u) {
        lib_size page_index = offset / LIB_STORAGE_MEDIUM_PAGE_BYTES;
        lib_size page_offset = offset % LIB_STORAGE_MEDIUM_PAGE_BYTES;
        lib_size count = LIB_STORAGE_MEDIUM_PAGE_BYTES - page_offset;
        lib_storage_medium_page *page;

        if (count > byte_count) count = byte_count;
        page = medium->pages[page_index];
        if (page != LIB_NULL) lib_memory_copy(cursor, page->bytes + page_offset, count);
        else {
            lib_status status = lib_storage_medium_read_base(medium, offset, cursor, count);
            if (status != LIB_STATUS_OK) return status;
        }
        cursor += count;
        offset += count;
        byte_count -= count;
    }
    return LIB_STATUS_OK;
}

static lib_status lib_storage_medium_write(lib_storage_medium *medium,
    lib_size offset, const void *bytes, lib_size byte_count)
{
    const lib_u8 *cursor = bytes;

    if (lib_storage_medium_range(medium, offset, byte_count, bytes) != LIB_STATUS_OK)
        return LIB_STATUS_INVALID_ARGUMENT;
    if (medium->mode == LIB_STORAGE_MEDIUM_READONLY) return LIB_STATUS_INVALID_STATE;
    if (medium->mode == LIB_STORAGE_MEDIUM_DIRECT) {
        lib_status status = lib_storage_file_seek_absolute(&medium->file, (lib_i64)offset);
        if (status == LIB_STATUS_OK)
            status = lib_storage_file_write_exact(&medium->file, bytes, byte_count);
        return status;
    }
    while (byte_count != 0u) {
        lib_size page_index = offset / LIB_STORAGE_MEDIUM_PAGE_BYTES;
        lib_size page_offset = offset % LIB_STORAGE_MEDIUM_PAGE_BYTES;
        lib_size count = LIB_STORAGE_MEDIUM_PAGE_BYTES - page_offset;
        lib_storage_medium_page *page;

        if (count > byte_count) count = byte_count;
        {
            lib_status status = lib_storage_medium_page_for_write(medium, page_index,
                &page);
            if (status != LIB_STATUS_OK) return status;
        }
        lib_memory_copy(page->bytes + page_offset, cursor, count);
        cursor += count;
        offset += count;
        byte_count -= count;
    }
    return LIB_STATUS_OK;
}

lib_status lib_storage_medium_write_at(lib_storage_medium *medium,
    lib_size offset, const void *bytes, lib_size byte_count)
{
    lib_status status = lib_storage_medium_write(medium, offset, bytes, byte_count);
    if (status == LIB_STATUS_OK && medium->mode == LIB_STORAGE_MEDIUM_DIRECT)
        status = lib_storage_file_flush(&medium->file);
    return status;
}

lib_status lib_storage_medium_fill_at(lib_storage_medium *medium,
    lib_size offset, lib_size byte_count, lib_u8 value)
{
    lib_u8 buffer[512];
    lib_size count;
    lib_status status = LIB_STATUS_OK;

    if (medium == LIB_NULL || offset > medium->byte_count ||
        byte_count > medium->byte_count - offset) return LIB_STATUS_INVALID_ARGUMENT;
    if (byte_count == 0u) return LIB_STATUS_OK;
    lib_memory_set(buffer, value, sizeof(buffer));
    while (byte_count != 0u) {
        count = byte_count > sizeof(buffer) ? sizeof(buffer) : byte_count;
        status = lib_storage_medium_write(medium, offset, buffer, count);
        if (status != LIB_STATUS_OK) break;
        offset += count;
        byte_count -= count;
    }
    if (medium->mode == LIB_STORAGE_MEDIUM_DIRECT) {
        /* Flush partial writes too; preserve the first write error. */
        lib_status flush_status = lib_storage_file_flush(&medium->file);
        if (status == LIB_STATUS_OK) status = flush_status;
    }
    return status;
}

lib_status lib_storage_medium_replace(lib_storage_medium **lease,
    lib_storage_medium *replacement, lib_storage_medium **out_retired)
{
    if (lease == LIB_NULL || out_retired == LIB_NULL || lease == out_retired ||
        *out_retired != LIB_NULL || (replacement != LIB_NULL && *lease == replacement))
        return LIB_STATUS_INVALID_ARGUMENT;
    *out_retired = *lease;
    *lease = replacement;
    return LIB_STATUS_OK;
}
