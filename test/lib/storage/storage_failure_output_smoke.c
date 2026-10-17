#include "lib/types/test.h"
#include "lib/storage/file_interface.h"
#include "lib/storage/medium_interface.h"
int main(void)
{
    void *bytes = (void *)1;
    lib_size byte_count = 7u;
    lib_storage_file_writer *writer = (lib_storage_file_writer *)1;
    lib_storage_medium *medium = (lib_storage_medium *)1;

    lib_test_assert(lib_storage_file_read_owned(LIB_NULL, 1u, &bytes, &byte_count) ==
        LIB_STATUS_INVALID_ARGUMENT && bytes == LIB_NULL);
    lib_test_assert(lib_storage_file_writer_open(LIB_NULL,
        LIB_STORAGE_FILE_WRITER_TRUNCATE, &writer) == LIB_STATUS_INVALID_ARGUMENT &&
        writer == LIB_NULL);
    lib_test_assert(lib_storage_medium_open(LIB_NULL, LIB_STORAGE_MEDIUM_READONLY, &medium) ==
        LIB_STATUS_INVALID_ARGUMENT && medium == LIB_NULL);
    medium = (lib_storage_medium *)1;
    lib_test_assert(lib_storage_medium_create_overlay(LIB_NULL, 1u, &medium) ==
        LIB_STATUS_INVALID_ARGUMENT && medium == LIB_NULL);
    return 0;
}
