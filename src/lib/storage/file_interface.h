#ifndef LIB_STORAGE_FILE_INTERFACE_H
#define LIB_STORAGE_FILE_INTERFACE_H

#include "lib/types/types_interface.h"

typedef struct lib_storage_file_writer lib_storage_file_writer;

typedef enum lib_storage_file_writer_mode {
    LIB_STORAGE_FILE_WRITER_TRUNCATE,
    LIB_STORAGE_FILE_WRITER_APPEND
} lib_storage_file_writer_mode;

lib_status lib_storage_file_read_owned(const char *path, lib_size maximum,
    void **out_bytes, lib_size *out_byte_count);
lib_status lib_storage_file_writer_open(const char *path,
    lib_storage_file_writer_mode mode,
    lib_storage_file_writer **out_writer);
lib_status lib_storage_file_writer_write(lib_storage_file_writer *writer,
    const void *bytes, lib_size byte_count);
lib_status lib_storage_file_writer_close(lib_storage_file_writer *writer);

#endif
