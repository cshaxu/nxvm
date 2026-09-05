#ifndef LIB_STORAGE_FILE_H
#define LIB_STORAGE_FILE_H

#include "type.h"

typedef enum lib_storage_file_write_result {
    LIB_STORAGE_FILE_WRITE_OK,
    LIB_STORAGE_FILE_WRITE_EXISTS,
    LIB_STORAGE_FILE_WRITE_FAULT
} lib_storage_file_write_result;

lib_storage_file_write_result lib_storage_file_write_exclusive(const C_CHAR *path,
    const C_VOID *bytes, STD_SIZE_T byte_count);
type_status lib_storage_file_read_owned(const C_CHAR *path, STD_SIZE_T maximum,
    C_VOID **out_bytes, STD_SIZE_T *out_byte_count);
C_INT lib_storage_file_exists(const C_CHAR *path);
C_INT lib_storage_file_replace(const C_CHAR *source, const C_CHAR *destination);
C_INT lib_storage_file_remove(const C_CHAR *path);

#endif
