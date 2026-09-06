#include "lib/storage/file.h"

int main(void)
{
    static const char path[] = "storage-file-smoke.tmp";
    lib_storage_file_writer *writer = LIB_NULL;
    lib_storage_file_reader *reader = LIB_NULL;
    char line[32];
    void *bytes = LIB_NULL;
    size_t byte_count = 0u;
    int result = 1;

    if (lib_storage_file_writer_open(LIB_NULL, &writer) !=
            LIB_STATUS_INVALID_ARGUMENT ||
        lib_storage_file_writer_open(path, &writer) != LIB_STATUS_OK ||
        lib_storage_file_writer_write(writer, "first\n") != LIB_STATUS_OK ||
        lib_storage_file_writer_write(writer, "second\n") != LIB_STATUS_OK ||
        lib_storage_file_writer_close(writer) != LIB_STATUS_OK) goto done;
    writer = LIB_NULL;
    if (lib_storage_file_reader_open(path, &reader) != LIB_STATUS_OK ||
        !lib_storage_file_reader_next(reader, line, sizeof(line)) ||
        strncmp(line, "first", 5u) != 0 ||
        !lib_storage_file_reader_next(reader, line, sizeof(line)) ||
        strncmp(line, "second", 6u) != 0 ||
        lib_storage_file_reader_next(reader, line, sizeof(line))) goto done;
    lib_storage_file_reader_close(reader);
    reader = LIB_NULL;
    if (lib_storage_file_read_owned(path, 32u, &bytes, &byte_count) !=
            LIB_STATUS_OK || byte_count == 0u) goto done;
    result = 0;
done:
    free(bytes);
    lib_storage_file_reader_close(reader);
    if (writer != LIB_NULL) (void)lib_storage_file_writer_close(writer);
    (void)lib_storage_file_remove(path);
    if (result == 0) puts("M5:T523:S6:STORAGE-OWNER:OK");
    return result;
}
