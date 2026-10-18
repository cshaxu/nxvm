#include "lib/types/test.h"
#include "lib/types/file.h"
#include "lib/storage/file_interface.h"
#include "lib/storage/medium_interface.h"
#ifdef _WIN32
#include "lib/types/win32/file.h"
#endif

int main(void)
{
    const char *path = "storage-\xe6\xb5\x8b-\xf0\x9f\x8e\xb5.bin";
    const lib_u8 payload[] = { 0u, 0xffu, '\r', '\n', 0x80u };
    lib_u8 actual[sizeof(payload) + 1u];
    lib_storage_file_writer *writer = LIB_NULL;
    lib_storage_file_reader *reader = LIB_NULL;
    lib_storage_medium *medium = LIB_NULL;
    void *bytes = LIB_NULL;
    lib_size count = 99u;
    lib_test_assert(lib_storage_file_writer_open(path, LIB_STORAGE_FILE_WRITER_TRUNCATE, &writer) == LIB_STATUS_OK);
    lib_test_assert(lib_storage_file_writer_write(writer, payload, sizeof(payload)) == LIB_STATUS_OK);
    lib_test_assert(lib_storage_file_writer_close(writer) == LIB_STATUS_OK);
    lib_test_assert(lib_storage_file_writer_open(path, LIB_STORAGE_FILE_WRITER_APPEND, &writer) == LIB_STATUS_OK);
    lib_test_assert(lib_storage_file_writer_write(writer, "Z", 1u) == LIB_STATUS_OK);
    lib_test_assert(lib_storage_file_writer_close(writer) == LIB_STATUS_OK);
    lib_test_assert(lib_storage_file_reader_open(path, &reader) == LIB_STATUS_OK);
    lib_test_assert(lib_storage_file_reader_read(reader, actual, sizeof(actual)) == LIB_STATUS_OK);
    lib_test_assert(lib_storage_file_reader_close(reader) == LIB_STATUS_OK);
    lib_test_assert(lib_memory_compare(actual, payload, sizeof(payload)) == 0 && actual[sizeof(payload)] == 'Z');
    lib_test_assert(lib_storage_file_read_owned(path, sizeof(actual), &bytes, &count) == LIB_STATUS_OK);
    lib_test_assert(count == sizeof(actual) && lib_memory_compare(actual, bytes, count) == 0);
    lib_release(bytes);
    for (lib_i32 mode = LIB_STORAGE_MEDIUM_DIRECT; mode <= LIB_STORAGE_MEDIUM_OVERLAY; ++mode) {
        lib_test_assert(lib_storage_medium_open(path, (lib_storage_medium_mode)mode, &medium) == LIB_STATUS_OK);
        lib_test_assert(lib_storage_medium_read_at(medium, 0u, actual, sizeof(payload)) == LIB_STATUS_OK);
        lib_test_assert(lib_memory_compare(actual, payload, sizeof(payload)) == 0);
        lib_test_assert(lib_storage_medium_destroy(&medium) == LIB_STATUS_OK);
    }
    /* Native-name oracle: a symmetric mis-encoding in read/write cannot pass. */
#ifdef _WIN32
    lib_c_file *native = lib_win32_wfopen(L"storage-\u6d4b-\U0001f3b5.bin", L"rb");
    lib_test_assert(native != LIB_NULL);
    lib_test_assert(lib_c_fclose(native) == 0);
    lib_test_assert(lib_win32_wremove(L"storage-\u6d4b-\U0001f3b5.bin") == 0);
#else
    lib_test_assert(lib_c_remove(path) == 0);
#endif
    /* Decoder enumeration belongs to Base; these exercise each Storage entry. */
    static const char *invalid[] = { "\x80", "\xc2" };
    for (lib_size i = 0u; i < sizeof(invalid) / sizeof(invalid[0]); ++i) {
        lib_test_assert(lib_storage_file_writer_open(invalid[i], LIB_STORAGE_FILE_WRITER_TRUNCATE, &writer) == LIB_STATUS_INVALID_ARGUMENT);
        lib_test_assert(writer == LIB_NULL);
        lib_test_assert(lib_storage_file_reader_open(invalid[i], &reader) == LIB_STATUS_INVALID_ARGUMENT);
        lib_test_assert(reader == LIB_NULL);
        lib_test_assert(lib_storage_file_read_owned(invalid[i], 100u, &bytes, &count) == LIB_STATUS_INVALID_ARGUMENT);
        lib_test_assert(bytes == LIB_NULL && count == 0u);
        lib_test_assert(lib_storage_medium_open(invalid[i], LIB_STORAGE_MEDIUM_DIRECT, &medium) == LIB_STATUS_INVALID_ARGUMENT);
        lib_test_assert(medium == LIB_NULL);
    }
    return 0;
}
