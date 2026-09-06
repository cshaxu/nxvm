#include "lib/base/base.h"

#include "lib/storage/file.h"
#include "lib/storage/native.h"

struct lib_storage_file_reader { FILE *file; };
struct lib_storage_file_writer { FILE *file; };

lib_storage_file_write_result lib_storage_file_write_exclusive(const char *path,
    const void *bytes, size_t byte_count)
{
    FILE *file;
    int failed;

    if (path == LIB_NULL || (byte_count != 0u && bytes == LIB_NULL)) {
        return LIB_STORAGE_FILE_WRITE_FAULT;
    }
    file = lib_storage_native_open_exclusive_write(path);
    if (file == LIB_NULL) return LIB_STORAGE_FILE_WRITE_EXISTS;
    failed = (byte_count != 0u && fwrite(bytes, 1u, byte_count, file) != byte_count) ||
        fclose(file) != 0;
    if (failed) (void)lib_storage_file_remove(path);
    return failed ? LIB_STORAGE_FILE_WRITE_FAULT : LIB_STORAGE_FILE_WRITE_OK;
}

lib_status lib_storage_file_read_owned(const char *path, size_t maximum,
    void **out_bytes, size_t *out_byte_count)
{
    FILE *file;
    int64_t length;
    void *bytes = LIB_NULL;

    if (path == LIB_NULL || out_bytes == LIB_NULL || out_byte_count == LIB_NULL) {
        return LIB_STATUS_INVALID_ARGUMENT;
    }
    *out_bytes = LIB_NULL;
    *out_byte_count = 0u;
    file = fopen(path, "rb");
    if (file == LIB_NULL) return LIB_STATUS_IO_ERROR;
    if (lib_storage_native_seek_64(file, 0, SEEK_END) != 0 ||
        (length = lib_storage_native_tell_64(file)) < 0 || (lib_u64)length > maximum ||
        lib_storage_native_seek_64(file, 0, SEEK_SET) != 0 ||
        (bytes = malloc((size_t)length == 0u ? 1u : (size_t)length)) == LIB_NULL ||
        ((size_t)length != 0u && fread(bytes, 1u, (size_t)length, file) !=
            (size_t)length)) {
        (void)fclose(file);
        free(bytes);
        return LIB_STATUS_IO_ERROR;
    }
    if (fclose(file) != 0) {
        free(bytes);
        return LIB_STATUS_IO_ERROR;
    }
    *out_bytes = bytes;
    *out_byte_count = (size_t)length;
    return LIB_STATUS_OK;
}

int lib_storage_file_exists(const char *path)
{
    FILE *file = path == LIB_NULL ? LIB_NULL : fopen(path, "rb");

    if (file == LIB_NULL) return LIB_FALSE;
    return fclose(file) == 0 ? LIB_TRUE : LIB_FALSE;
}

int lib_storage_file_replace(const char *source, const char *destination)
{ return lib_storage_native_file_replace(source, destination); }

int lib_storage_file_remove(const char *path)
{ return lib_storage_native_file_remove(path); }

lib_status lib_storage_file_reader_open(const char *path,
    lib_storage_file_reader **out_reader)
{
    lib_storage_file_reader *reader;

    if (out_reader == LIB_NULL || path == LIB_NULL) return LIB_STATUS_INVALID_ARGUMENT;
    *out_reader = LIB_NULL;
    reader = malloc(sizeof(*reader));
    if (reader == LIB_NULL) return LIB_STATUS_NO_MEMORY;
    reader->file = fopen(path, "rb");
    if (reader->file == LIB_NULL) {
        free(reader);
        return LIB_STATUS_IO_ERROR;
    }
    *out_reader = reader;
    return LIB_STATUS_OK;
}

int lib_storage_file_reader_next(lib_storage_file_reader *reader, char *line,
    size_t capacity)
{
    return reader != LIB_NULL && line != LIB_NULL && capacity != 0u &&
        capacity <= 0x7fffffffu && fgets(line, (int)capacity, reader->file) != LIB_NULL;
}

void lib_storage_file_reader_close(lib_storage_file_reader *reader)
{
    if (reader == LIB_NULL) return;
    if (reader->file != LIB_NULL) (void)fclose(reader->file);
    free(reader);
}

lib_status lib_storage_file_writer_open(const char *path,
    lib_storage_file_writer **out_writer)
{
    lib_storage_file_writer *writer;

    if (out_writer == LIB_NULL || path == LIB_NULL) return LIB_STATUS_INVALID_ARGUMENT;
    *out_writer = LIB_NULL;
    writer = malloc(sizeof(*writer));
    if (writer == LIB_NULL) return LIB_STATUS_NO_MEMORY;
    writer->file = fopen(path, "w");
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
