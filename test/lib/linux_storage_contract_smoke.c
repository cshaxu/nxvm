#include "lib/types/test.h"
#include "lib/types/file.h"
/* Controlled Linux platform calls; executes the real file implementation. */
#include "lib/types/types_interface.h"
#include "lib/types/file.h"

#define LIB_TYPES_LINUX_FILE_H
typedef lib_i64 lib_linux_off_t;
#define LIB_LINUX_LOCK_SH 1
#define LIB_LINUX_LOCK_EX 2
#define LIB_LINUX_LOCK_NB 4
#define lib_linux_fseeko fseek
#define lib_linux_ftello ftell
static lib_bool reject_lock, reject_open;
static lib_i32 expected_lock;
static lib_u32 closes;
static lib_c_file *open_stream(const char *path, const char *mode)
{
    lib_test_assert(path != LIB_NULL);
    lib_test_assert(mode[0] == 'r' && mode[1] == 'b');
    lib_test_assert(mode[2] == (expected_lock == LIB_LINUX_LOCK_EX ? '+' : '\0'));
    return reject_open ? LIB_NULL : tmpfile();
}
static lib_i32 close_stream(lib_c_file *stream) { ++closes; return lib_c_fclose(stream); }
static lib_i32 descriptor(lib_c_file *stream) { lib_test_assert(stream != LIB_NULL); return 7; }
static lib_i32 lock_stream(lib_i32 fd, lib_i32 operation)
{
    lib_test_assert(fd == 7 && operation == (expected_lock | LIB_LINUX_LOCK_NB));
    return reject_lock ? -1 : 0;
}
#undef lib_c_fopen
#undef lib_c_fclose
#define lib_c_fopen open_stream
#define lib_c_fclose close_stream
#define lib_linux_fileno descriptor
#define lib_linux_flock lock_stream
#include "lib/storage/linux/file.c"
#include "lib/storage/file.c"

int main(void)
{
    for (lib_i32 write = 0; write != 2; ++write) {
        for (lib_u32 failure = 0u; failure != 2u; ++failure) {
            reject_lock = failure != 0u;
            lib_storage_file file = { 0 };
            lib_u32 before = closes;
            expected_lock = write ? LIB_LINUX_LOCK_EX : LIB_LINUX_LOCK_SH;
            lib_status status = storage_file_platform_open("fixture", write, &file);
            lib_test_assert(status == (reject_lock ? LIB_STATUS_IO_ERROR : LIB_STATUS_OK));
            lib_test_assert((file.stream == LIB_NULL) == (reject_lock != 0));
            if (!reject_lock) {
                lib_i64 length = -1;
                lib_test_assert(lib_storage_file_byte_count(&file, &length) == LIB_STATUS_OK && length == 0);
                lib_test_assert(lib_storage_file_write_exact(&file, "abc", 3u) == LIB_STATUS_OK);
                lib_test_assert(lib_storage_file_seek_absolute(&file, 1) == LIB_STATUS_OK);
                lib_test_assert(lib_storage_file_byte_count(&file, &length) == LIB_STATUS_OK && length == 3);
                lib_test_assert(storage_file_platform_tell(&file) == 1);
            }
            lib_test_assert(lib_storage_file_close(&file) == LIB_STATUS_OK);
            lib_test_assert(closes == before + 1u && file.stream == LIB_NULL);
            reject_open = LIB_TRUE;
            lib_test_assert(storage_file_platform_open("fixture", write, &file) == LIB_STATUS_IO_ERROR);
            lib_test_assert(file.stream == LIB_NULL && closes == before + 1u);
            reject_open = LIB_FALSE;
        }
    }
    return 0;
}
