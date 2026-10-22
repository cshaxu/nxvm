#ifndef LIB_STORAGE_FILE_INTERFACE_H
#define LIB_STORAGE_FILE_INTERFACE_H

#include "lib/types/types_interface.h"

/* Paths are NUL-terminated UTF-8 on every platform. Malformed UTF-8 returns
 * INVALID_ARGUMENT; no locale/code-page fallback is attempted. POSIX names
 * containing non-UTF-8 bytes are outside this contract. File contents are
 * arbitrary bytes, not text. Failed opens leave their output object null. */

typedef struct lib_storage_file_writer lib_storage_file_writer;
typedef struct lib_storage_file_reader lib_storage_file_reader;

typedef enum lib_storage_file_writer_mode {
    LIB_STORAGE_FILE_WRITER_TRUNCATE,
    LIB_STORAGE_FILE_WRITER_APPEND
} lib_storage_file_writer_mode;

lib_status lib_storage_file_read_owned(const char *path, lib_size maximum,
    void **out_bytes, lib_size *out_byte_count);
lib_status lib_storage_file_reader_open(const char *path,
    lib_storage_file_reader **out_reader);
lib_status lib_storage_file_reader_read(lib_storage_file_reader *reader,
    void *bytes, lib_size byte_count);
lib_status lib_storage_file_reader_close(lib_storage_file_reader *reader);
lib_status lib_storage_file_writer_open(const char *path,
    lib_storage_file_writer_mode mode,
    lib_storage_file_writer **out_writer);
lib_status lib_storage_file_writer_write(lib_storage_file_writer *writer,
    const void *bytes, lib_size byte_count);
/* Same write operation, additionally reporting the accepted prefix even on
 * IO_ERROR. Invalid arguments set out_written to zero when supplied. Accepted
 * bytes must not be replayed; this does not imply persistence or flushing. */
lib_status lib_storage_file_writer_write_some(lib_storage_file_writer *writer,
    const void *bytes, lib_size byte_count, lib_size *out_written);
/* Make buffered bytes visible to other readers, not a durable-media barrier.
 * Failure does not revoke bytes previously accepted by write. */
lib_status lib_storage_file_writer_flush(lib_storage_file_writer *writer);
lib_status lib_storage_file_writer_close(lib_storage_file_writer *writer);

#endif
