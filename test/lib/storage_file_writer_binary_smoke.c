#include "lib/types/test.h"
#include "lib/types/file.h"
#include "lib/storage/file_interface.h"
#include "lib/types/file.h"
#include "cleanup.h"


static lib_u32 allocations, live_allocations;
static lib_u32 allocation_attempts, fail_allocation_at;
static lib_size last_allocation;
static lib_i32 reject_allocation;
static void *allocate_bytes(lib_size count)
{
    ++allocation_attempts;
    last_allocation = count;
    void *bytes = reject_allocation || allocation_attempts == fail_allocation_at ? LIB_NULL : lib_allocate(count);
    if (bytes != LIB_NULL) { ++allocations; ++live_allocations; }
    return bytes;
}
static void *allocate_zero(lib_size count, lib_size size)
{
    lib_test_assert(size == 0u || count <= (lib_size)-1 / size);
    void *bytes = allocate_bytes(count * size);
    if (bytes != LIB_NULL) lib_memory_set(bytes, 0, count * size);
    return bytes;
}
static void release_bytes(void *bytes)
{
    if (bytes != LIB_NULL) { lib_test_assert(live_allocations != 0u); --live_allocations; }
    lib_release(bytes);
}
#undef lib_allocate
#undef lib_allocate_zero
#undef lib_release
#define lib_allocate allocate_bytes
#define lib_allocate_zero allocate_zero
#define lib_release release_bytes

static lib_i32 reject_close;
static lib_u32 close_calls;
static lib_i32 close_stream(lib_c_file *stream)
{
    lib_i32 result = lib_c_fclose(stream);
    ++close_calls;
    return reject_close ? LIB_C_EOF : result;
}
#undef lib_c_fclose
#define lib_c_fclose close_stream
static lib_i32 reject_read;
static lib_size read_stream(void *bytes, lib_size size, lib_size count, lib_c_file *stream)
{ return reject_read ? 0u : lib_c_fread(bytes, size, count, stream); }
#undef lib_c_fread
#define lib_c_fread read_stream
static lib_u32 write_calls, flush_calls, fail_write_at;
static lib_i32 reject_flush;
static lib_size write_stream(const void *bytes, lib_size size, lib_size count, lib_c_file *stream)
{
    if (++write_calls == fail_write_at) count /= 2u;
    return lib_c_fwrite(bytes, size, count, stream);
}
static lib_i32 flush_stream(lib_c_file *stream)
{
    ++flush_calls;
    return reject_flush ? LIB_C_EOF : lib_c_fflush(stream);
}
#undef lib_c_fwrite
#undef lib_c_fflush
#define lib_c_fwrite write_stream
#define lib_c_fflush flush_stream
#include "lib/storage/file.c"
#include "lib/storage/medium.c"

static void medium_fill(void)
{
    lib_storage_medium medium = { .file = { tmpfile() },
        .byte_count = 1024u * 1024u + 7u, .mode = LIB_STORAGE_MEDIUM_DIRECT };
    lib_u8 actual[513];
    lib_test_assert(medium.file.stream != LIB_NULL);
    write_calls = flush_calls = 0;
    lib_test_assert(lib_storage_medium_fill_at(&medium, 0, medium.byte_count, 0x5a) == LIB_STATUS_OK);
    lib_test_assert(write_calls == 2049u && flush_calls == 1u);
    rewind(medium.file.stream);
    for (lib_size left = medium.byte_count; left != 0u;) {
        lib_size count = left > sizeof(actual) ? sizeof(actual) : left;
        lib_test_assert(lib_c_fread(actual, 1u, count, medium.file.stream) == count);
        for (lib_size i = 0; i < count; ++i) lib_test_assert(actual[i] == 0x5a);
        left -= count;
    }
    write_calls = flush_calls = 0;
    lib_test_assert(lib_storage_medium_fill_at(&medium, 3u, 1024u * 1024u, 0x6b) == LIB_STATUS_OK);
    lib_test_assert(write_calls == 2048u && flush_calls == 1u);
    lib_test_assert(lib_storage_medium_read_at(&medium, 2u, actual, 2u) == LIB_STATUS_OK);
    lib_test_assert(actual[0] == 0x5a && actual[1] == 0x6b);
    lib_test_assert(lib_storage_medium_read_at(&medium, medium.byte_count - 5u, actual, 5u) == LIB_STATUS_OK);
    lib_test_assert(actual[0] == 0x6b);
    for (lib_u32 i = 1; i < 5; ++i) lib_test_assert(actual[i] == 0x5a);
    write_calls = flush_calls = 0;
    lib_test_assert(lib_storage_medium_fill_at(&medium, medium.byte_count, 0, 0) == LIB_STATUS_OK);
    lib_test_assert(lib_storage_medium_fill_at(&medium, medium.byte_count, 1, 0) == LIB_STATUS_INVALID_ARGUMENT);
    lib_test_assert(lib_storage_medium_fill_at(&medium, (lib_size)-1, 1, 0) == LIB_STATUS_INVALID_ARGUMENT);
    lib_test_assert(lib_storage_medium_fill_at(LIB_NULL, 0, 0, 0) == LIB_STATUS_INVALID_ARGUMENT);
    lib_test_assert(write_calls == 0u && flush_calls == 0u);
    lib_test_assert(lib_storage_medium_write_at(&medium, 0, "Z", 1) == LIB_STATUS_OK);
    lib_test_assert(write_calls == 1u && flush_calls == 1u);
    for (lib_u32 failure = 1; failure <= 2; ++failure) {
        for (reject_flush = 0; reject_flush <= 1; ++reject_flush) {
            write_calls = flush_calls = 0;
            fail_write_at = failure;
            lib_test_assert(lib_storage_medium_fill_at(&medium, 0, 1025u, 0x7c) == LIB_STATUS_IO_ERROR);
            lib_test_assert(write_calls == failure && flush_calls == 1u);
        }
    }
    fail_write_at = 0;
    reject_flush = 1;
    write_calls = flush_calls = 0;
    lib_test_assert(lib_storage_medium_fill_at(&medium, 0, 1025u, 0x8d) == LIB_STATUS_IO_ERROR);
    lib_test_assert(write_calls == 3u && flush_calls == 1u);
    reject_flush = 0;
    write_calls = flush_calls = 0;
    fail_write_at = 1;
    lib_test_assert(lib_storage_medium_write_at(&medium, 0, "XYZ", 3) == LIB_STATUS_IO_ERROR);
    lib_test_assert(write_calls == 1u && flush_calls == 0u); /* Ordinary write unchanged. */
    fail_write_at = 0;
    lib_test_assert(lib_storage_file_close(&medium.file) == LIB_STATUS_OK);
}

static void overlay_index(const char *path)
{
    lib_storage_medium *medium = LIB_NULL;
    lib_u8 source[8201], actual[8201];
    for (lib_size i = 0; i < sizeof(source); ++i) source[i] = (lib_u8)(i * 17u + 3u);
    lib_test_assert(lib_storage_medium_page_count((lib_size)-1) ==
        (lib_size)-1 / 4096u + 1u);
    /* Reject the huge index before allocating it; no multi-GiB test memory. */
    fail_allocation_at = allocation_attempts + 2u;
    lib_test_assert(lib_storage_medium_create_zero_overlay((lib_size)-1, &medium) == LIB_STATUS_NO_MEMORY);
    lib_test_assert(medium == LIB_NULL && live_allocations == 0u);
    lib_test_assert(last_allocation == lib_storage_medium_page_count((lib_size)-1) * sizeof(void *));
    fail_allocation_at = 0u;
    lib_test_assert(lib_storage_medium_create_zero_overlay(0u, &medium) == LIB_STATUS_OK);
    lib_test_assert(medium->pages == LIB_NULL && live_allocations == 1u);
    lib_test_assert(lib_storage_medium_read_at(medium, 0, LIB_NULL, 0) == LIB_STATUS_OK);
    lib_test_assert(lib_storage_medium_write_at(medium, 0, LIB_NULL, 0) == LIB_STATUS_OK);
    lib_test_assert(lib_storage_medium_destroy(&medium) == LIB_STATUS_OK);
    for (lib_u32 failure = 1; failure <= 5; ++failure) {
        /* Container, index and three pages: every failed create cleans up. */
        fail_allocation_at = allocation_attempts + failure;
        lib_test_assert(lib_storage_medium_create_overlay(source, sizeof(source), &medium) == LIB_STATUS_NO_MEMORY);
        lib_test_assert(medium == LIB_NULL && live_allocations == 0u);
    }
    fail_allocation_at = 0u;
    lib_test_assert(lib_storage_medium_create_overlay(source, sizeof(source), &medium) == LIB_STATUS_OK);
    lib_test_assert(lib_storage_medium_read_at(medium, 0, actual, sizeof(actual)) == LIB_STATUS_OK);
    lib_test_assert(lib_memory_compare(actual, source, sizeof(actual)) == 0);
    lib_u32 before = allocations;
    lib_test_assert(lib_storage_medium_write_at(medium, 4094u, source, 12u) == LIB_STATUS_OK);
    lib_test_assert(allocations == before); /* Existing pages are reused. */
    lib_test_assert(lib_storage_medium_read_at(medium, 4094u, actual, 12u) == LIB_STATUS_OK);
    lib_test_assert(lib_memory_compare(actual, source, 12u) == 0);
    lib_test_assert(lib_storage_medium_read_at(medium, sizeof(source), actual, 1u) == LIB_STATUS_INVALID_ARGUMENT);
    flush_calls = 0;
    lib_test_assert(lib_storage_medium_fill_at(medium, 4094u, sizeof(source) - 4094u, 0x3a) == LIB_STATUS_OK);
    lib_test_assert(flush_calls == 0u);
    lib_test_assert(lib_storage_medium_read_at(medium, 0u, actual, sizeof(actual)) == LIB_STATUS_OK);
    lib_test_assert(lib_memory_compare(actual, source, 4094u) == 0);
    for (lib_size i = 4094u; i < sizeof(actual); ++i) lib_test_assert(actual[i] == 0x3a);
    lib_test_assert(lib_storage_medium_destroy(&medium) == LIB_STATUS_OK);

    lib_test_assert(lib_storage_medium_create_zero_overlay(sizeof(source), &medium) == LIB_STATUS_OK);
    fail_allocation_at = allocation_attempts + 2u;
    lib_test_assert(lib_storage_medium_write_at(medium, 4095u, source, 2u) == LIB_STATUS_NO_MEMORY);
    fail_allocation_at = 0u;
    lib_test_assert(medium->pages[0] != LIB_NULL && medium->pages[1] == LIB_NULL);
    lib_test_assert(lib_storage_medium_read_at(medium, 4095u, actual, 2u) == LIB_STATUS_OK);
    lib_test_assert(actual[0] == source[0] && actual[1] == 0u); /* Existing partial-write contract. */
    lib_test_assert(lib_storage_medium_write_at(medium, sizeof(source)-1u, source+1, 1u) == LIB_STATUS_OK);
    lib_test_assert(medium->pages[1] == LIB_NULL && medium->pages[2] != LIB_NULL);
    lib_test_assert(lib_storage_medium_read_at(medium, sizeof(source)-1u, actual, 1u) == LIB_STATUS_OK);
    lib_test_assert(actual[0] == source[1]);
    lib_test_assert(lib_storage_medium_destroy(&medium) == LIB_STATUS_OK);

    lib_u32 closed = close_calls;
    fail_allocation_at = allocation_attempts + 2u;
    lib_test_assert(lib_storage_medium_open(path, LIB_STORAGE_MEDIUM_OVERLAY, &medium) == LIB_STATUS_NO_MEMORY);
    lib_test_assert(medium == LIB_NULL && live_allocations == 0u && close_calls == closed + 1u);
    fail_allocation_at = 0u;
    lib_test_assert(lib_storage_medium_open(path, LIB_STORAGE_MEDIUM_OVERLAY, &medium) == LIB_STATUS_OK);
    reject_read = 1;
    lib_test_assert(lib_storage_medium_write_at(medium, 0u, source, 1u) == LIB_STATUS_IO_ERROR);
    lib_test_assert(medium->pages[0] == LIB_NULL && live_allocations == 2u);
    reject_read = 0;
    lib_test_assert(lib_storage_medium_write_at(medium, 0u, source, 1u) == LIB_STATUS_OK);
    lib_test_assert(lib_storage_medium_destroy(&medium) == LIB_STATUS_OK && live_allocations == 0u);
}

int main(void)
{
    lib_storage_file positioned = { tmpfile() };
    lib_i64 measured = -1;
    lib_test_assert(positioned.stream != LIB_NULL);
    lib_test_assert(lib_storage_file_byte_count(&positioned, &measured) == LIB_STATUS_OK && measured == 0);
    lib_test_assert(lib_storage_file_seek_absolute(&positioned, 4096) == LIB_STATUS_OK);
    lib_test_assert(lib_storage_file_write_exact(&positioned, "Z", 1u) == LIB_STATUS_OK);
    lib_test_assert(lib_storage_file_seek_absolute(&positioned, 7) == LIB_STATUS_OK);
    lib_test_assert(lib_storage_file_byte_count(&positioned, &measured) == LIB_STATUS_OK && measured == 4097);
    lib_test_assert(storage_file_platform_tell(&positioned) == 7);
    lib_test_assert(lib_storage_file_close(&positioned) == LIB_STATUS_OK);
    static const lib_u8 payload[] = { 'A', 0u, 'B', '\n' };
    lib_u8 actual[sizeof(payload)] = { 0u };
    const char *path = "shared-storage-writer-binary-smoke.bin";
    lib_storage_file_writer *writer = LIB_NULL;
    lib_c_file *file;

    lib_test_assert(lib_storage_file_writer_open(path, LIB_STORAGE_FILE_WRITER_TRUNCATE,
        &writer) == LIB_STATUS_OK);
    lib_test_assert(lib_storage_file_writer_write(writer, payload, sizeof(payload)) ==
        LIB_STATUS_OK);
    lib_test_assert(lib_storage_file_writer_close(writer) == LIB_STATUS_OK);
    file = lib_c_fopen(path, "rb");
    lib_test_assert(file != LIB_NULL);
    lib_test_assert(lib_c_fread(actual, 1u, sizeof(actual), file) == sizeof(actual));
    lib_test_assert(fgetc(file) == LIB_C_EOF);
    lib_test_assert(lib_c_fclose(file) == 0);
    lib_test_assert(lib_memory_compare(actual, payload, sizeof(payload)) == 0);
    lib_test_assert(allocations == 1u && live_allocations == 0u);
    {
        lib_storage_file_reader *reader = LIB_NULL;
        lib_u8 prefix[2] = { 0u };
        lib_u8 suffix[2] = { 0u };

        lib_test_assert(lib_storage_file_reader_open(path, &reader) == LIB_STATUS_OK);
        lib_test_assert(lib_storage_file_reader_read(reader, prefix, sizeof(prefix)) ==
            LIB_STATUS_OK);
        lib_test_assert(lib_storage_file_reader_read(reader, suffix, sizeof(suffix)) ==
            LIB_STATUS_OK);
        lib_test_assert(lib_storage_file_reader_read(reader, suffix, 1u) ==
            LIB_STATUS_IO_ERROR);
        lib_test_assert(lib_storage_file_reader_close(reader) == LIB_STATUS_OK);
        lib_test_assert(lib_memory_compare(prefix, payload, sizeof(prefix)) == 0);
        lib_test_assert(lib_memory_compare(suffix, payload + sizeof(prefix),
            sizeof(suffix)) == 0);
    }
    void *owned = LIB_NULL;
    lib_size length = 0u;
    lib_u32 before = allocations;
    lib_test_assert(lib_storage_file_read_owned(path, sizeof(payload), &owned, &length) == LIB_STATUS_OK);
    lib_test_assert(length == sizeof(payload) && allocations == before + 1u);
    lib_test_assert(lib_memory_compare(owned, payload, length) == 0);
    lib_release(owned);
    lib_test_assert(lib_storage_file_read_owned(path, 1u, &owned, &length) == LIB_STATUS_LIMIT_EXCEEDED);
    lib_test_assert(owned == LIB_NULL && length == 0u && live_allocations == 0u);
    reject_allocation = 1;
    lib_test_assert(lib_storage_file_read_owned(path, sizeof(payload), &owned, &length) == LIB_STATUS_NO_MEMORY);
    lib_test_assert(owned == LIB_NULL && live_allocations == 0u);
    lib_storage_medium *medium = LIB_NULL;
    lib_test_assert(lib_storage_medium_open(path, LIB_STORAGE_MEDIUM_DIRECT, &medium) == LIB_STATUS_NO_MEMORY);
    lib_test_assert(medium == LIB_NULL);
    reject_allocation = 0;
    for (lib_i32 mode = LIB_STORAGE_MEDIUM_DIRECT; mode <= LIB_STORAGE_MEDIUM_OVERLAY; ++mode) {
        before = allocations;
        lib_test_assert(lib_storage_medium_open(path, (lib_storage_medium_mode)mode, &medium) == LIB_STATUS_OK);
        lib_u32 expected = mode == LIB_STORAGE_MEDIUM_OVERLAY ? 2u : 1u;
        lib_test_assert(allocations == before + expected && live_allocations == expected);
        lib_test_assert((medium->pages != LIB_NULL) == (mode == LIB_STORAGE_MEDIUM_OVERLAY));
        lib_test_assert(lib_storage_medium_read_at(medium, 0u, actual, sizeof(actual)) == LIB_STATUS_OK);
        lib_test_assert(lib_memory_compare(actual, payload, sizeof(payload)) == 0);
        write_calls = flush_calls = 0;
        lib_test_assert(lib_storage_medium_fill_at(medium, sizeof(payload), 0u, 0u) == LIB_STATUS_OK);
        lib_test_assert(lib_storage_medium_fill_at(medium, 1u, 2u, 'F') ==
            (mode == LIB_STORAGE_MEDIUM_READONLY ? LIB_STATUS_INVALID_STATE : LIB_STATUS_OK));
        lib_test_assert(flush_calls == (mode == LIB_STORAGE_MEDIUM_DIRECT ? 1u : 0u));
        lib_test_assert(lib_storage_medium_read_at(medium, 0u, actual, sizeof(actual)) == LIB_STATUS_OK);
        lib_test_assert(actual[0] == payload[0] && actual[3] == payload[3]);
        if (mode == LIB_STORAGE_MEDIUM_READONLY) lib_test_assert(lib_memory_compare(actual, payload, sizeof(payload)) == 0);
        else lib_test_assert(actual[1] == 'F' && actual[2] == 'F');
        if (mode == LIB_STORAGE_MEDIUM_DIRECT)
            lib_test_assert(lib_storage_medium_write_at(medium, 0u, payload, sizeof(payload)) == LIB_STATUS_OK);
        lib_u8 changed = 'Z';
        lib_test_assert(lib_storage_medium_write_at(medium, 0u, &changed, 1u) ==
            (mode == LIB_STORAGE_MEDIUM_READONLY ? LIB_STATUS_INVALID_STATE : LIB_STATUS_OK));
        if (mode == LIB_STORAGE_MEDIUM_DIRECT)
            lib_test_assert(lib_storage_medium_write_at(medium, 0u, payload, 1u) == LIB_STATUS_OK);
        lib_test_assert(lib_storage_medium_destroy(&medium) == LIB_STATUS_OK);
        lib_test_assert(live_allocations == 0u);
    }
    lib_test_assert(lib_storage_file_read_owned(path, sizeof(payload), &owned, &length) == LIB_STATUS_OK);
    lib_test_assert(lib_memory_compare(owned, payload, length) == 0); /* Overlay did not modify file. */
    lib_release(owned);
    lib_test_assert(lib_storage_medium_create_zero_overlay(sizeof(payload), &medium) == LIB_STATUS_OK);
    lib_test_assert(lib_storage_medium_read_at(medium, 0u, actual, sizeof(actual)) == LIB_STATUS_OK);
    for (lib_u32 i = 0u; i < sizeof(actual); ++i) lib_test_assert(actual[i] == 0u);
    lib_test_assert(lib_storage_medium_destroy(&medium) == LIB_STATUS_OK);
    lib_test_assert(lib_storage_medium_open(path, LIB_STORAGE_MEDIUM_READONLY, &medium) == LIB_STATUS_OK);
    lib_u32 prior = close_calls;
    reject_close = 1;
    lib_test_assert(lib_storage_medium_destroy(&medium) == LIB_STATUS_IO_ERROR);
    lib_test_assert(medium == LIB_NULL && close_calls == prior + 1);
    lib_test_assert(lib_storage_medium_destroy(&medium) == LIB_STATUS_OK);
    lib_test_assert(close_calls == prior + 1); /* Consumed close must never be retried. */
    lib_test_assert(lib_storage_file_read_owned(path, sizeof(payload), &owned, &length) == LIB_STATUS_IO_ERROR);
    lib_test_assert(owned == LIB_NULL && live_allocations == 0u);
    reject_close = 0;
    medium_fill();
    overlay_index(path);
    lib_test_assert(shared_test_remove_file(path));
    for (lib_i32 mode = LIB_STORAGE_MEDIUM_DIRECT; mode <= LIB_STORAGE_MEDIUM_OVERLAY; ++mode) {
        lib_test_assert(lib_storage_medium_open(path, (lib_storage_medium_mode)mode, &medium) == LIB_STATUS_IO_ERROR);
        lib_test_assert(medium == LIB_NULL && live_allocations == 0u);
    }
    return 0;
}
