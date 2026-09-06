#ifndef LIB_STORAGE_FILE_H
#define LIB_STORAGE_FILE_H

#include "lib/base/base.h"

typedef enum lib_storage_file_write_result {
    LIB_STORAGE_FILE_WRITE_OK,
    LIB_STORAGE_FILE_WRITE_EXISTS,
    LIB_STORAGE_FILE_WRITE_FAULT
} lib_storage_file_write_result;

typedef struct lib_storage_file_reader lib_storage_file_reader;
typedef struct lib_storage_file_writer lib_storage_file_writer;

lib_storage_file_write_result lib_storage_file_write_exclusive(const char *path,
    const void *bytes, size_t byte_count);
lib_status lib_storage_file_read_owned(const char *path, size_t maximum,
    void **out_bytes, size_t *out_byte_count);
int lib_storage_file_exists(const char *path);
int lib_storage_file_replace(const char *source, const char *destination);
int lib_storage_file_remove(const char *path);
lib_status lib_storage_file_reader_open(const char *path,
    lib_storage_file_reader **out_reader);
int lib_storage_file_reader_next(lib_storage_file_reader *reader, char *line,
    size_t capacity);
void lib_storage_file_reader_close(lib_storage_file_reader *reader);
lib_status lib_storage_file_writer_open(const char *path,
    lib_storage_file_writer **out_writer);
lib_status lib_storage_file_writer_write(lib_storage_file_writer *writer,
    const char *text);
lib_status lib_storage_file_writer_close(lib_storage_file_writer *writer);

#endif
