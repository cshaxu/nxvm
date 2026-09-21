#include "lib/storage/file_interface.h"
#include "lib/types/file.h"
#include "cleanup.h"

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

static unsigned allocations, live_allocations;
static unsigned allocation_attempts, fail_allocation_at;
static size_t last_allocation;
static int reject_allocation;
static void *allocate_bytes(size_t count)
{
    ++allocation_attempts;
    last_allocation = count;
    void *bytes = reject_allocation || allocation_attempts == fail_allocation_at ? NULL : malloc(count);
    if (bytes != NULL) { ++allocations; ++live_allocations; }
    return bytes;
}
static void *allocate_zero(size_t count, size_t size)
{
    assert(size == 0u || count <= (size_t)-1 / size);
    void *bytes = allocate_bytes(count * size);
    if (bytes != NULL) lib_memory_set(bytes, 0, count * size);
    return bytes;
}
static void release_bytes(void *bytes)
{
    if (bytes != NULL) { assert(live_allocations != 0u); --live_allocations; }
    free(bytes);
}
#undef lib_allocate
#undef lib_allocate_zero
#undef lib_release
#define lib_allocate allocate_bytes
#define lib_allocate_zero allocate_zero
#define lib_release release_bytes

static int reject_close;
static unsigned close_calls;
static int close_stream(FILE *stream)
{
    int result = fclose(stream);
    ++close_calls;
    return reject_close ? EOF : result;
}
#undef lib_c_fclose
#define lib_c_fclose close_stream
static int reject_read;
static size_t read_stream(void *bytes, size_t size, size_t count, FILE *stream)
{ return reject_read ? 0u : fread(bytes, size, count, stream); }
#undef lib_c_fread
#define lib_c_fread read_stream
#include "lib/storage/file.c"
#include "lib/storage/medium.c"

static void overlay_index(const char *path)
{
    lib_storage_medium *medium = NULL;
    unsigned char source[8201], actual[8201];
    for (size_t i = 0; i < sizeof(source); ++i) source[i] = (unsigned char)(i * 17u + 3u);
    assert(lib_storage_medium_page_count((lib_size)-1) ==
        (lib_size)-1 / 4096u + 1u);
    /* Reject the huge index before allocating it; no multi-GiB test memory. */
    fail_allocation_at = allocation_attempts + 2u;
    assert(lib_storage_medium_create_zero_overlay((lib_size)-1, &medium) == LIB_STATUS_NO_MEMORY);
    assert(medium == NULL && live_allocations == 0u);
    assert(last_allocation == lib_storage_medium_page_count((lib_size)-1) * sizeof(void *));
    fail_allocation_at = 0u;
    assert(lib_storage_medium_create_zero_overlay(0u, &medium) == LIB_STATUS_OK);
    assert(medium->pages == NULL && live_allocations == 1u);
    assert(lib_storage_medium_read_at(medium, 0, NULL, 0) == LIB_STATUS_OK);
    assert(lib_storage_medium_write_at(medium, 0, NULL, 0) == LIB_STATUS_OK);
    assert(lib_storage_medium_destroy(&medium) == LIB_STATUS_OK);
    for (unsigned failure = 1; failure <= 5; ++failure) {
        /* Container, index and three pages: every failed create cleans up. */
        fail_allocation_at = allocation_attempts + failure;
        assert(lib_storage_medium_create_overlay(source, sizeof(source), &medium) == LIB_STATUS_NO_MEMORY);
        assert(medium == NULL && live_allocations == 0u);
    }
    fail_allocation_at = 0u;
    assert(lib_storage_medium_create_overlay(source, sizeof(source), &medium) == LIB_STATUS_OK);
    assert(lib_storage_medium_read_at(medium, 0, actual, sizeof(actual)) == LIB_STATUS_OK);
    assert(memcmp(actual, source, sizeof(actual)) == 0);
    unsigned before = allocations;
    assert(lib_storage_medium_write_at(medium, 4094u, source, 12u) == LIB_STATUS_OK);
    assert(allocations == before); /* Existing pages are reused. */
    assert(lib_storage_medium_read_at(medium, 4094u, actual, 12u) == LIB_STATUS_OK);
    assert(memcmp(actual, source, 12u) == 0);
    assert(lib_storage_medium_read_at(medium, sizeof(source), actual, 1u) == LIB_STATUS_INVALID_ARGUMENT);
    assert(lib_storage_medium_destroy(&medium) == LIB_STATUS_OK);

    assert(lib_storage_medium_create_zero_overlay(sizeof(source), &medium) == LIB_STATUS_OK);
    fail_allocation_at = allocation_attempts + 2u;
    assert(lib_storage_medium_write_at(medium, 4095u, source, 2u) == LIB_STATUS_NO_MEMORY);
    fail_allocation_at = 0u;
    assert(medium->pages[0] != NULL && medium->pages[1] == NULL);
    assert(lib_storage_medium_read_at(medium, 4095u, actual, 2u) == LIB_STATUS_OK);
    assert(actual[0] == source[0] && actual[1] == 0u); /* Existing partial-write contract. */
    assert(lib_storage_medium_write_at(medium, sizeof(source)-1u, source+1, 1u) == LIB_STATUS_OK);
    assert(medium->pages[1] == NULL && medium->pages[2] != NULL);
    assert(lib_storage_medium_read_at(medium, sizeof(source)-1u, actual, 1u) == LIB_STATUS_OK);
    assert(actual[0] == source[1]);
    assert(lib_storage_medium_destroy(&medium) == LIB_STATUS_OK);

    unsigned closed = close_calls;
    fail_allocation_at = allocation_attempts + 2u;
    assert(lib_storage_medium_open(path, LIB_STORAGE_MEDIUM_OVERLAY, &medium) == LIB_STATUS_NO_MEMORY);
    assert(medium == NULL && live_allocations == 0u && close_calls == closed + 1u);
    fail_allocation_at = 0u;
    assert(lib_storage_medium_open(path, LIB_STORAGE_MEDIUM_OVERLAY, &medium) == LIB_STATUS_OK);
    reject_read = 1;
    assert(lib_storage_medium_write_at(medium, 0u, source, 1u) == LIB_STATUS_IO_ERROR);
    assert(medium->pages[0] == NULL && live_allocations == 2u);
    reject_read = 0;
    assert(lib_storage_medium_write_at(medium, 0u, source, 1u) == LIB_STATUS_OK);
    assert(lib_storage_medium_destroy(&medium) == LIB_STATUS_OK && live_allocations == 0u);
}

int main(void)
{
    lib_storage_file positioned = { tmpfile() };
    lib_i64 measured = -1;
    assert(positioned.stream != NULL);
    assert(lib_storage_file_byte_count(&positioned, &measured) == LIB_STATUS_OK && measured == 0);
    assert(lib_storage_file_seek_absolute(&positioned, 4096) == LIB_STATUS_OK);
    assert(lib_storage_file_write_exact(&positioned, "Z", 1u) == LIB_STATUS_OK);
    assert(lib_storage_file_seek_absolute(&positioned, 7) == LIB_STATUS_OK);
    assert(lib_storage_file_byte_count(&positioned, &measured) == LIB_STATUS_OK && measured == 4097);
    assert(storage_file_platform_tell(&positioned) == 7);
    assert(lib_storage_file_close(&positioned) == LIB_STATUS_OK);
    static const unsigned char payload[] = { 'A', 0u, 'B', '\n' };
    unsigned char actual[sizeof(payload)] = { 0u };
    const char *path = "softpc-storage-writer-binary-smoke.bin";
    lib_storage_file_writer *writer = LIB_NULL;
    FILE *file;

    assert(lib_storage_file_writer_open(path, LIB_STORAGE_FILE_WRITER_TRUNCATE,
        &writer) == LIB_STATUS_OK);
    assert(lib_storage_file_writer_write(writer, payload, sizeof(payload)) ==
        LIB_STATUS_OK);
    assert(lib_storage_file_writer_close(writer) == LIB_STATUS_OK);
    file = fopen(path, "rb");
    assert(file != NULL);
    assert(fread(actual, 1u, sizeof(actual), file) == sizeof(actual));
    assert(fgetc(file) == EOF);
    assert(fclose(file) == 0);
    assert(lib_memory_compare(actual, payload, sizeof(payload)) == 0);
    assert(allocations == 1u && live_allocations == 0u);
    {
        lib_storage_file_reader *reader = LIB_NULL;
        lib_u8 prefix[2] = { 0u };
        lib_u8 suffix[2] = { 0u };

        assert(lib_storage_file_reader_open(path, &reader) == LIB_STATUS_OK);
        assert(lib_storage_file_reader_read(reader, prefix, sizeof(prefix)) ==
            LIB_STATUS_OK);
        assert(lib_storage_file_reader_read(reader, suffix, sizeof(suffix)) ==
            LIB_STATUS_OK);
        assert(lib_storage_file_reader_read(reader, suffix, 1u) ==
            LIB_STATUS_IO_ERROR);
        assert(lib_storage_file_reader_close(reader) == LIB_STATUS_OK);
        assert(lib_memory_compare(prefix, payload, sizeof(prefix)) == 0);
        assert(lib_memory_compare(suffix, payload + sizeof(prefix),
            sizeof(suffix)) == 0);
    }
    void *owned = NULL;
    lib_size length = 0u;
    unsigned before = allocations;
    assert(lib_storage_file_read_owned(path, sizeof(payload), &owned, &length) == LIB_STATUS_OK);
    assert(length == sizeof(payload) && allocations == before + 1u);
    assert(lib_memory_compare(owned, payload, length) == 0);
    lib_release(owned);
    assert(lib_storage_file_read_owned(path, 1u, &owned, &length) == LIB_STATUS_LIMIT_EXCEEDED);
    assert(owned == NULL && length == 0u && live_allocations == 0u);
    reject_allocation = 1;
    assert(lib_storage_file_read_owned(path, sizeof(payload), &owned, &length) == LIB_STATUS_NO_MEMORY);
    assert(owned == NULL && live_allocations == 0u);
    lib_storage_medium *medium = NULL;
    assert(lib_storage_medium_open(path, LIB_STORAGE_MEDIUM_DIRECT, &medium) == LIB_STATUS_NO_MEMORY);
    assert(medium == NULL);
    reject_allocation = 0;
    for (int mode = LIB_STORAGE_MEDIUM_DIRECT; mode <= LIB_STORAGE_MEDIUM_OVERLAY; ++mode) {
        before = allocations;
        assert(lib_storage_medium_open(path, (lib_storage_medium_mode)mode, &medium) == LIB_STATUS_OK);
        unsigned expected = mode == LIB_STORAGE_MEDIUM_OVERLAY ? 2u : 1u;
        assert(allocations == before + expected && live_allocations == expected);
        assert((medium->pages != NULL) == (mode == LIB_STORAGE_MEDIUM_OVERLAY));
        assert(lib_storage_medium_read_at(medium, 0u, actual, sizeof(actual)) == LIB_STATUS_OK);
        assert(lib_memory_compare(actual, payload, sizeof(payload)) == 0);
        unsigned char changed = 'Z';
        assert(lib_storage_medium_write_at(medium, 0u, &changed, 1u) ==
            (mode == LIB_STORAGE_MEDIUM_READONLY ? LIB_STATUS_INVALID_STATE : LIB_STATUS_OK));
        if (mode == LIB_STORAGE_MEDIUM_DIRECT)
            assert(lib_storage_medium_write_at(medium, 0u, payload, 1u) == LIB_STATUS_OK);
        assert(lib_storage_medium_destroy(&medium) == LIB_STATUS_OK);
        assert(live_allocations == 0u);
    }
    assert(lib_storage_file_read_owned(path, sizeof(payload), &owned, &length) == LIB_STATUS_OK);
    assert(lib_memory_compare(owned, payload, length) == 0); /* Overlay did not modify file. */
    lib_release(owned);
    assert(lib_storage_medium_create_zero_overlay(sizeof(payload), &medium) == LIB_STATUS_OK);
    assert(lib_storage_medium_read_at(medium, 0u, actual, sizeof(actual)) == LIB_STATUS_OK);
    for (unsigned i = 0u; i < sizeof(actual); ++i) assert(actual[i] == 0u);
    assert(lib_storage_medium_destroy(&medium) == LIB_STATUS_OK);
    assert(lib_storage_medium_open(path, LIB_STORAGE_MEDIUM_READONLY, &medium) == LIB_STATUS_OK);
    unsigned prior = close_calls;
    reject_close = 1;
    assert(lib_storage_medium_destroy(&medium) == LIB_STATUS_IO_ERROR);
    assert(medium == NULL && close_calls == prior + 1);
    assert(lib_storage_medium_destroy(&medium) == LIB_STATUS_OK);
    assert(close_calls == prior + 1); /* Consumed close must never be retried. */
    assert(lib_storage_file_read_owned(path, sizeof(payload), &owned, &length) == LIB_STATUS_IO_ERROR);
    assert(owned == NULL && live_allocations == 0u);
    reject_close = 0;
    overlay_index(path);
    assert(softpc_test_remove_image(path));
    for (int mode = LIB_STORAGE_MEDIUM_DIRECT; mode <= LIB_STORAGE_MEDIUM_OVERLAY; ++mode) {
        assert(lib_storage_medium_open(path, (lib_storage_medium_mode)mode, &medium) == LIB_STATUS_IO_ERROR);
        assert(medium == NULL && live_allocations == 0u);
    }
    return 0;
}
