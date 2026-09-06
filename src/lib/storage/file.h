#ifndef LIB_STORAGE_FILE_H
#define LIB_STORAGE_FILE_H

#include "lib/base/base.h"

typedef enum lib_storage_file_write_result {
    LIB_STORAGE_FILE_WRITE_OK,
    LIB_STORAGE_FILE_WRITE_EXISTS,
    LIB_STORAGE_FILE_WRITE_FAULT
} lib_storage_file_write_result;

lib_storage_file_write_result lib_storage_file_write_exclusive(const char *path,
    const void *bytes, size_t byte_count);
lib_status lib_storage_file_read_owned(const char *path, size_t maximum,
    void **out_bytes, size_t *out_byte_count);
int lib_storage_file_exists(const char *path);
int lib_storage_file_replace(const char *source, const char *destination);
int lib_storage_file_remove(const char *path);

#endif
