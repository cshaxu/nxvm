#include "lib/storage/file_interface.h"
#include "lib/types/file.h"
#include "cleanup.h"

#include <assert.h>
#include <stdio.h>

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
#include "lib/storage/file.c"
#include "lib/storage/medium.c"

int main(void)
{
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
    lib_storage_medium *medium = NULL;
    assert(lib_storage_medium_open(path, LIB_STORAGE_MEDIUM_READONLY, &medium) == LIB_STATUS_OK);
    unsigned prior = close_calls;
    reject_close = 1;
    assert(lib_storage_medium_destroy(&medium) == LIB_STATUS_IO_ERROR);
    assert(medium == NULL && close_calls == prior + 1);
    assert(lib_storage_medium_destroy(&medium) == LIB_STATUS_OK);
    assert(close_calls == prior + 1); /* Consumed close must never be retried. */
    assert(softpc_test_remove_image(path));
    return 0;
}
