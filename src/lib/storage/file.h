#ifndef LIB_STORAGE_FILE_H
#define LIB_STORAGE_FILE_H

#include "lib/base/base.h"

typedef struct lib_storage_file_writer lib_storage_file_writer;

lib_status lib_storage_file_read_owned(const char *path, size_t maximum,
    void **out_bytes, size_t *out_byte_count);
lib_status lib_storage_file_writer_open(const char *path,
    lib_storage_file_writer **out_writer);
lib_status lib_storage_file_writer_write(lib_storage_file_writer *writer,
    const char *text);
lib_status lib_storage_file_writer_close(lib_storage_file_writer *writer);

#endif
