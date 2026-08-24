#ifndef CORE_PLATFORM_FILE_H
#define CORE_PLATFORM_FILE_H

#include "type.h"

typedef struct core_platform_file_reader core_platform_file_reader;
typedef struct core_platform_file_writer core_platform_file_writer;

typedef enum core_platform_file_write_result {
    CORE_PLATFORM_FILE_WRITE_OK,
    CORE_PLATFORM_FILE_WRITE_EXISTS,
    CORE_PLATFORM_FILE_WRITE_FAULT
} core_platform_file_write_result;

core_platform_file_write_result core_platform_file_write_exclusive(const C_CHAR *path, const C_VOID *bytes,
    STD_SIZE_T count);
C_INT core_platform_file_read_all(const C_CHAR *path, STD_SIZE_T maximum,
    C_VOID **out_bytes, STD_SIZE_T *out_count);
C_INT core_platform_file_exists(const C_CHAR *path);
C_INT core_platform_file_replace(const C_CHAR *source, const C_CHAR *destination);
C_INT core_platform_file_remove(const C_CHAR *path);
type_status core_platform_file_reader_open(const C_CHAR *path,
    core_platform_file_reader **out_reader);
C_INT core_platform_file_reader_next(core_platform_file_reader *reader,
    C_CHAR *line, STD_SIZE_T capacity);
C_VOID core_platform_file_reader_close(core_platform_file_reader *reader);
type_status core_platform_file_writer_open(const C_CHAR *path,
    core_platform_file_writer **out_writer);
type_status core_platform_file_writer_write(core_platform_file_writer *writer,
    const C_CHAR *text);
type_status core_platform_file_writer_close(core_platform_file_writer *writer);

#endif
