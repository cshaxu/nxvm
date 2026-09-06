#include "lib/base/base.h"

#include "lib/storage/file.h"
#include "lib/storage/native.h"

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
