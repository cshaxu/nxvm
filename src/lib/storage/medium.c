#include "lib/base/base.h"

#include "lib/storage/medium.h"
#include "lib/storage/internal/native.h"

#define LIB_STORAGE_MEDIUM_PAGE_BYTES 4096u

typedef struct lib_storage_medium_page {
    size_t index;
    struct lib_storage_medium_page *next;
    lib_u8 bytes[LIB_STORAGE_MEDIUM_PAGE_BYTES];
} lib_storage_medium_page;

struct lib_storage_medium {
    FILE *file;
    lib_storage_medium_page *pages;
    size_t byte_count;
    lib_storage_medium_mode mode;
};

static lib_status lib_storage_medium_create(FILE *file, size_t byte_count,
    lib_storage_medium_mode mode, lib_storage_medium **out_medium)
{
    lib_storage_medium *medium;

    if (out_medium == LIB_NULL) return LIB_STATUS_INVALID_ARGUMENT;
    *out_medium = LIB_NULL;
    medium = calloc(1u, sizeof(*medium));
    if (medium == LIB_NULL) return LIB_STATUS_NO_MEMORY;
    medium->file = file;
    medium->byte_count = byte_count;
    medium->mode = mode;
    *out_medium = medium;
    return LIB_STATUS_OK;
}

lib_status lib_storage_medium_open(const char *path, lib_storage_medium_mode mode,
    lib_storage_medium **out_medium)
{
    FILE *file;
    int64_t length;
    lib_status status;

    if (path == LIB_NULL || out_medium == LIB_NULL || mode < LIB_STORAGE_MEDIUM_DIRECT ||
        mode > LIB_STORAGE_MEDIUM_OVERLAY)
        return LIB_STATUS_INVALID_ARGUMENT;
    *out_medium = LIB_NULL;
    file = lib_storage_native_open(path, mode);
    if (file == LIB_NULL || lib_storage_native_seek_64(file, 0, SEEK_END) != 0 ||
        (length = lib_storage_native_tell_64(file)) < 0 ||
        (uint64_t)length > SIZE_MAX || lib_storage_native_seek_64(file, 0, SEEK_SET) != 0) {
        if (file != LIB_NULL) (void)fclose(file);
        return LIB_STATUS_IO_ERROR;
    }
    status = lib_storage_medium_create(file, (size_t)length, mode, out_medium);
    if (status != LIB_STATUS_OK) (void)fclose(file);
    return status;
}

lib_status lib_storage_medium_create_overlay(const void *bytes, size_t byte_count,
    lib_storage_medium **out_medium)
{
    lib_status status;

    if (byte_count != 0u && bytes == LIB_NULL) return LIB_STATUS_INVALID_ARGUMENT;
    status = lib_storage_medium_create(LIB_NULL, byte_count,
        LIB_STORAGE_MEDIUM_OVERLAY, out_medium);
    if (status != LIB_STATUS_OK || byte_count == 0u) return status;
    status = lib_storage_medium_write_at(*out_medium, 0u, bytes, byte_count);
    if (status != LIB_STATUS_OK) lib_storage_medium_destroy(out_medium);
    return status;
}

lib_status lib_storage_medium_create_zero_overlay(size_t byte_count,
    lib_storage_medium **out_medium)
{
    return lib_storage_medium_create(LIB_NULL, byte_count,
        LIB_STORAGE_MEDIUM_OVERLAY, out_medium);
}

static void lib_storage_medium_pages_destroy(lib_storage_medium_page *page)
{
    while (page != LIB_NULL) {
        lib_storage_medium_page *next = page->next;
        free(page);
        page = next;
    }
}

void lib_storage_medium_destroy(lib_storage_medium **medium)
{
    lib_storage_medium *value;

    if (medium == LIB_NULL) return;
    value = *medium;
    *medium = LIB_NULL;
    if (value == LIB_NULL) return;
    if (value->file != LIB_NULL) (void)fclose(value->file);
    lib_storage_medium_pages_destroy(value->pages);
    free(value);
}

size_t lib_storage_medium_byte_count(const lib_storage_medium *medium)
{ return medium == LIB_NULL ? 0u : medium->byte_count; }

static lib_status lib_storage_medium_range(const lib_storage_medium *medium,
    size_t offset, size_t byte_count, const void *bytes)
{
    if (medium == LIB_NULL || (byte_count != 0u && bytes == LIB_NULL) ||
        offset > medium->byte_count || byte_count > medium->byte_count - offset)
        return LIB_STATUS_INVALID_ARGUMENT;
    return LIB_STATUS_OK;
}

static lib_storage_medium_page *lib_storage_medium_find_page(
    const lib_storage_medium *medium, size_t index)
{
    lib_storage_medium_page *page = medium->pages;
    while (page != LIB_NULL && page->index != index) page = page->next;
    return page;
}

static lib_status lib_storage_medium_read_base(const lib_storage_medium *medium,
    size_t offset, void *bytes, size_t byte_count)
{
    if (byte_count == 0u) return LIB_STATUS_OK;
    if (medium->file == LIB_NULL) {
        memset(bytes, 0, byte_count);
        return LIB_STATUS_OK;
    }
    return lib_storage_native_seek_64(medium->file, (int64_t)offset, SEEK_SET) != 0 ||
        fread(bytes, 1u, byte_count, medium->file) != byte_count ?
        LIB_STATUS_IO_ERROR : LIB_STATUS_OK;
}

static lib_status lib_storage_medium_page_for_write(lib_storage_medium *medium,
    size_t index, lib_storage_medium_page **out_page)
{
    lib_storage_medium_page *page = lib_storage_medium_find_page(medium, index);
    size_t offset;
    size_t count;

    if (page != LIB_NULL) {
        *out_page = page;
        return LIB_STATUS_OK;
    }
    page = malloc(sizeof(*page));
    if (page == LIB_NULL) return LIB_STATUS_NO_MEMORY;
    if (index > medium->byte_count / LIB_STORAGE_MEDIUM_PAGE_BYTES) {
        return LIB_STATUS_INVALID_ARGUMENT;
    }
    offset = index * LIB_STORAGE_MEDIUM_PAGE_BYTES;
    count = medium->byte_count - offset;
    if (count > LIB_STORAGE_MEDIUM_PAGE_BYTES) count = LIB_STORAGE_MEDIUM_PAGE_BYTES;
    if (lib_storage_medium_read_base(medium, offset, page->bytes, count) != LIB_STATUS_OK) {
        free(page);
        return LIB_STATUS_IO_ERROR;
    }
    if (count < LIB_STORAGE_MEDIUM_PAGE_BYTES)
        memset(page->bytes + count, 0, LIB_STORAGE_MEDIUM_PAGE_BYTES - count);
    page->index = index;
    page->next = medium->pages;
    medium->pages = page;
    *out_page = page;
    return LIB_STATUS_OK;
}

lib_status lib_storage_medium_read_at(const lib_storage_medium *medium,
    size_t offset, void *bytes, size_t byte_count)
{
    lib_u8 *cursor = bytes;

    if (lib_storage_medium_range(medium, offset, byte_count, bytes) != LIB_STATUS_OK)
        return LIB_STATUS_INVALID_ARGUMENT;
    if (medium->mode != LIB_STORAGE_MEDIUM_OVERLAY)
        return lib_storage_medium_read_base(medium, offset, bytes, byte_count);
    while (byte_count != 0u) {
        size_t page_index = offset / LIB_STORAGE_MEDIUM_PAGE_BYTES;
        size_t page_offset = offset % LIB_STORAGE_MEDIUM_PAGE_BYTES;
        size_t count = LIB_STORAGE_MEDIUM_PAGE_BYTES - page_offset;
        lib_storage_medium_page *page;

        if (count > byte_count) count = byte_count;
        page = lib_storage_medium_find_page(medium, page_index);
        if (page != LIB_NULL) memcpy(cursor, page->bytes + page_offset, count);
        else if (lib_storage_medium_read_base(medium, offset, cursor, count) != LIB_STATUS_OK)
            return LIB_STATUS_IO_ERROR;
        cursor += count;
        offset += count;
        byte_count -= count;
    }
    return LIB_STATUS_OK;
}

lib_status lib_storage_medium_write_at(lib_storage_medium *medium,
    size_t offset, const void *bytes, size_t byte_count)
{
    const lib_u8 *cursor = bytes;

    if (lib_storage_medium_range(medium, offset, byte_count, bytes) != LIB_STATUS_OK)
        return LIB_STATUS_INVALID_ARGUMENT;
    if (medium->mode == LIB_STORAGE_MEDIUM_READONLY) return LIB_STATUS_INVALID_STATE;
    if (medium->mode == LIB_STORAGE_MEDIUM_DIRECT) {
        return lib_storage_native_seek_64(medium->file, (int64_t)offset, SEEK_SET) != 0 ||
            fwrite(bytes, 1u, byte_count, medium->file) != byte_count ||
            fflush(medium->file) != 0 ? LIB_STATUS_IO_ERROR : LIB_STATUS_OK;
    }
    while (byte_count != 0u) {
        size_t page_index = offset / LIB_STORAGE_MEDIUM_PAGE_BYTES;
        size_t page_offset = offset % LIB_STORAGE_MEDIUM_PAGE_BYTES;
        size_t count = LIB_STORAGE_MEDIUM_PAGE_BYTES - page_offset;
        lib_storage_medium_page *page;

        if (count > byte_count) count = byte_count;
        {
            lib_status status = lib_storage_medium_page_for_write(medium, page_index,
                &page);
            if (status != LIB_STATUS_OK) return status;
        }
        memcpy(page->bytes + page_offset, cursor, count);
        cursor += count;
        offset += count;
        byte_count -= count;
    }
    return LIB_STATUS_OK;
}

lib_status lib_storage_medium_fill_at(lib_storage_medium *medium,
    size_t offset, size_t byte_count, lib_u8 value)
{
    lib_u8 buffer[512];
    size_t count;

    if (medium == LIB_NULL || offset > medium->byte_count ||
        byte_count > medium->byte_count - offset) return LIB_STATUS_INVALID_ARGUMENT;
    memset(buffer, value, sizeof(buffer));
    while (byte_count != 0u) {
        count = byte_count > sizeof(buffer) ? sizeof(buffer) : byte_count;
        if (lib_storage_medium_write_at(medium, offset, buffer, count) != LIB_STATUS_OK)
            return LIB_STATUS_IO_ERROR;
        offset += count;
        byte_count -= count;
    }
    return LIB_STATUS_OK;
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
