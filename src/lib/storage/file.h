#ifndef LIB_STORAGE_FILE_H
#define LIB_STORAGE_FILE_H

#include "lib/types/types_interface.h"

#include "lib/types/file.h"

typedef struct lib_storage_file {
    lib_c_file *stream;
} lib_storage_file;

/* Selected platform implementations supply open/locking and large-file
 * positioning. Open requires an empty caller-owned file; failure leaves it
 * empty. Common close consumes the stream, never the enclosing storage. */
lib_status storage_file_platform_open_readonly(const char *path,
    lib_storage_file *file);
lib_status storage_file_platform_open_readwrite(const char *path,
    lib_storage_file *file);
lib_status storage_file_platform_seek_absolute(const lib_storage_file *file,
    lib_i64 offset);
lib_status storage_file_platform_byte_count(lib_storage_file *file,
    lib_i64 *out_byte_count);

lib_status lib_storage_file_read_exact(const lib_storage_file *file, void *bytes,
    lib_size byte_count);
lib_status lib_storage_file_write_exact(lib_storage_file *file,
    const void *bytes, lib_size byte_count);
lib_status lib_storage_file_flush(lib_storage_file *file);
lib_status lib_storage_file_seek_absolute(const lib_storage_file *file,
    lib_i64 offset);
lib_status lib_storage_file_byte_count(lib_storage_file *file,
    lib_i64 *out_byte_count);
lib_status lib_storage_file_close(lib_storage_file *file);

#endif
