#include "lib/base/base_interface.h"

#include "lib/storage/file_interface.h"
#include "lib/storage/file_backend.h"

#include <stdio.h>
#include <stdlib.h>

struct lib_storage_file_writer { FILE *file; };

lib_status lib_storage_file_read_owned(const char *path, lib_size maximum,
    void **out_bytes, lib_size *out_byte_count)
{
    FILE *file;
    lib_i64 length;
    void *bytes = LIB_NULL;

    if (path == LIB_NULL || out_bytes == LIB_NULL || out_byte_count == LIB_NULL) {
        return LIB_STATUS_INVALID_ARGUMENT;
    }
    *out_bytes = LIB_NULL;
    *out_byte_count = 0u;
    file = fopen(path, "rb");
    if (file == LIB_NULL) return LIB_STATUS_IO_ERROR;
    if (lib_storage_file_backend_seek_64(file, 0, SEEK_END) != 0 ||
        (length = lib_storage_file_backend_tell_64(file)) < 0 || (lib_u64)length > maximum ||
        lib_storage_file_backend_seek_64(file, 0, SEEK_SET) != 0 ||
        (bytes = malloc((lib_size)length == 0u ? 1u : (lib_size)length)) == LIB_NULL ||
        ((lib_size)length != 0u && fread(bytes, 1u, (lib_size)length, file) !=
            (lib_size)length)) {
        (void)fclose(file);
        free(bytes);
        return LIB_STATUS_IO_ERROR;
    }
    if (fclose(file) != 0) {
        free(bytes);
        return LIB_STATUS_IO_ERROR;
    }
    *out_bytes = bytes;
    *out_byte_count = (lib_size)length;
    return LIB_STATUS_OK;
}

lib_status lib_storage_file_writer_open(const char *path,
    lib_storage_file_writer_mode mode,
    lib_storage_file_writer **out_writer)
{
    lib_storage_file_writer *writer;

    if (out_writer == LIB_NULL || path == LIB_NULL ||
        mode < LIB_STORAGE_FILE_WRITER_TRUNCATE ||
        mode > LIB_STORAGE_FILE_WRITER_APPEND) return LIB_STATUS_INVALID_ARGUMENT;
    *out_writer = LIB_NULL;
    writer = malloc(sizeof(*writer));
    if (writer == LIB_NULL) return LIB_STATUS_NO_MEMORY;
    writer->file = fopen(path, mode == LIB_STORAGE_FILE_WRITER_TRUNCATE ? "w" : "a");
    if (writer->file == LIB_NULL) {
        free(writer);
        return LIB_STATUS_IO_ERROR;
    }
    *out_writer = writer;
    return LIB_STATUS_OK;
}

lib_status lib_storage_file_writer_write(lib_storage_file_writer *writer,
    const char *text)
{
    return writer == LIB_NULL || text == LIB_NULL || fputs(text, writer->file) < 0 ?
        LIB_STATUS_IO_ERROR : LIB_STATUS_OK;
}

lib_status lib_storage_file_writer_close(lib_storage_file_writer *writer)
{
    int result;

    if (writer == LIB_NULL) return LIB_STATUS_INVALID_ARGUMENT;
    result = fclose(writer->file);
    free(writer);
    return result == 0 ? LIB_STATUS_OK : LIB_STATUS_IO_ERROR;
}
