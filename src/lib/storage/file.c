#include "type.h"

#include "lib/storage/file.h"
#include "lib/storage/native.h"

lib_storage_file_write_result lib_storage_file_write_exclusive(const C_CHAR *path,
    const C_VOID *bytes, STD_SIZE_T byte_count)
{
    STD_FILE *file;
    C_INT failed;

    if (path == STD_NULL || (byte_count != 0u && bytes == STD_NULL)) {
        return LIB_STORAGE_FILE_WRITE_FAULT;
    }
    file = STD_FOPEN_EXCLUSIVE_WRITE(path);
    if (file == STD_NULL) return LIB_STORAGE_FILE_WRITE_EXISTS;
    failed = (byte_count != 0u && STD_FWRITE(bytes, 1u, byte_count, file) != byte_count) ||
        STD_FCLOSE(file) != 0;
    if (failed) (C_VOID)lib_storage_file_remove(path);
    return failed ? LIB_STORAGE_FILE_WRITE_FAULT : LIB_STORAGE_FILE_WRITE_OK;
}

type_status lib_storage_file_read_owned(const C_CHAR *path, STD_SIZE_T maximum,
    C_VOID **out_bytes, STD_SIZE_T *out_byte_count)
{
    STD_FILE *file;
    type_signed_64 length;
    C_VOID *bytes = STD_NULL;

    if (path == STD_NULL || out_bytes == STD_NULL || out_byte_count == STD_NULL) {
        return TYPE_STATUS_INVALID_ARGUMENT;
    }
    *out_bytes = STD_NULL;
    *out_byte_count = 0u;
    file = STD_FOPEN(path, "rb");
    if (file == STD_NULL) return TYPE_STATUS_FAULT;
    if (STD_FSEEK_64(file, 0, STD_SEEK_END) != 0 ||
        (length = STD_FTELL_64(file)) < 0 || (type_unsigned_64)length > maximum ||
        STD_FSEEK_64(file, 0, STD_SEEK_SET) != 0 ||
        (bytes = STD_MALLOC((STD_SIZE_T)length == 0u ? 1u : (STD_SIZE_T)length)) == STD_NULL ||
        ((STD_SIZE_T)length != 0u && STD_FREAD(bytes, 1u, (STD_SIZE_T)length, file) !=
            (STD_SIZE_T)length)) {
        (C_VOID)STD_FCLOSE(file);
        STD_FREE(bytes);
        return TYPE_STATUS_FAULT;
    }
    if (STD_FCLOSE(file) != 0) {
        STD_FREE(bytes);
        return TYPE_STATUS_FAULT;
    }
    *out_bytes = bytes;
    *out_byte_count = (STD_SIZE_T)length;
    return TYPE_STATUS_OK;
}

C_INT lib_storage_file_exists(const C_CHAR *path)
{
    STD_FILE *file = path == STD_NULL ? STD_NULL : STD_FOPEN(path, "rb");

    if (file == STD_NULL) return TYPE_FALSE;
    return STD_FCLOSE(file) == 0 ? TYPE_TRUE : TYPE_FALSE;
}

C_INT lib_storage_file_replace(const C_CHAR *source, const C_CHAR *destination)
{ return lib_storage_native_file_replace(source, destination); }

C_INT lib_storage_file_remove(const C_CHAR *path)
{ return lib_storage_native_file_remove(path); }
