#include "lib/storage/file_interface.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main(void)
{
    static const char path[] = "storage-file-smoke.tmp";
    static const unsigned char binary[] = { 0x00u, 0xffu, 0x41u };
    lib_storage_file_writer *writer = LIB_NULL;
    void *bytes = LIB_NULL;
    size_t byte_count = 0u;
    int result = 1;

    if (lib_storage_file_writer_open(LIB_NULL, LIB_STORAGE_FILE_WRITER_TRUNCATE,
            &writer) !=
            LIB_STATUS_INVALID_ARGUMENT ||
        lib_storage_file_writer_open(path, LIB_STORAGE_FILE_WRITER_APPEND,
            &writer) != LIB_STATUS_OK ||
        lib_storage_file_writer_write(writer, "first", 5u) != LIB_STATUS_OK ||
        lib_storage_file_writer_write(writer, "second", 6u) != LIB_STATUS_OK ||
        lib_storage_file_writer_close(writer) != LIB_STATUS_OK) goto done;
    writer = LIB_NULL;
    if (lib_storage_file_writer_open(path, LIB_STORAGE_FILE_WRITER_APPEND,
            &writer) != LIB_STATUS_OK ||
        lib_storage_file_writer_write(writer, "third", 5u) != LIB_STATUS_OK ||
        lib_storage_file_writer_close(writer) != LIB_STATUS_OK) goto done;
    writer = LIB_NULL;
    if (lib_storage_file_read_owned(path, 32u, &bytes, &byte_count) !=
            LIB_STATUS_OK || byte_count != 16u ||
        memcmp(bytes, "firstsecondthird", byte_count) != 0) goto done;
    free(bytes);
    bytes = LIB_NULL;
    if (lib_storage_file_writer_open(path, LIB_STORAGE_FILE_WRITER_TRUNCATE,
            &writer) != LIB_STATUS_OK ||
        lib_storage_file_writer_write(writer, binary, sizeof(binary)) != LIB_STATUS_OK ||
        lib_storage_file_writer_close(writer) != LIB_STATUS_OK) goto done;
    writer = LIB_NULL;
    if (lib_storage_file_read_owned(path, 32u, &bytes, &byte_count) !=
            LIB_STATUS_OK || byte_count != sizeof(binary) ||
        memcmp(bytes, binary, byte_count) != 0) goto done;
    result = 0;
done:
    free(bytes);
    if (writer != LIB_NULL) (void)lib_storage_file_writer_close(writer);
    (void)remove(path);
    if (result == 0) puts("M5:T523:S6:STORAGE-OWNER:OK");
    return result;
}
