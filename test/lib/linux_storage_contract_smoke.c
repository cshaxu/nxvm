/* Controlled Linux platform calls; executes the real file implementation. */
#include "lib/types/types_interface.h"
#include "lib/types/file.h"
#include <assert.h>
#include <stdio.h>

#define LIB_TYPES_LINUX_FILE_H
typedef long lib_linux_off_t;
typedef struct { int l_type, l_whence; } lib_linux_file_lock;
#define LIB_LINUX_F_RDLCK 1
#define LIB_LINUX_F_WRLCK 2
#define LIB_LINUX_F_SETLK 3
#define lib_linux_fseeko fseek
#define lib_linux_ftello ftell
static int reject_lock, reject_open, expected_lock;
static unsigned closes;
static FILE *open_stream(const char *path, const char *mode)
{
    assert(path != NULL);
    assert(mode[0] == 'r' && mode[1] == 'b');
    assert(mode[2] == (expected_lock == LIB_LINUX_F_WRLCK ? '+' : '\0'));
    return reject_open ? NULL : tmpfile();
}
static int close_stream(FILE *stream) { ++closes; return fclose(stream); }
static int descriptor(FILE *stream) { assert(stream != NULL); return 7; }
static int lock_stream(int fd, int operation, const lib_linux_file_lock *lock)
{
    assert(fd == 7 && operation == LIB_LINUX_F_SETLK);
    assert(lock->l_type == expected_lock && lock->l_whence == LIB_SEEK_SET);
    return reject_lock ? -1 : 0;
}
#undef lib_c_fopen
#undef lib_c_fclose
#define lib_c_fopen open_stream
#define lib_c_fclose close_stream
#define lib_linux_fileno descriptor
#define lib_linux_fcntl lock_stream
#include "lib/storage/linux/file.c"
#include "lib/storage/file.c"

int main(void)
{
    for (int write = 0; write != 2; ++write) {
        for (reject_lock = 0; reject_lock != 2; ++reject_lock) {
            lib_storage_file file = { 0 };
            unsigned before = closes;
            expected_lock = write ? LIB_LINUX_F_WRLCK : LIB_LINUX_F_RDLCK;
            lib_status status = storage_file_platform_open("fixture", write, &file);
            assert(status == (reject_lock ? LIB_STATUS_IO_ERROR : LIB_STATUS_OK));
            assert((file.stream == NULL) == (reject_lock != 0));
            if (!reject_lock) {
                lib_i64 length = -1;
                assert(lib_storage_file_byte_count(&file, &length) == LIB_STATUS_OK && length == 0);
                assert(lib_storage_file_write_exact(&file, "abc", 3u) == LIB_STATUS_OK);
                assert(lib_storage_file_seek_absolute(&file, 1) == LIB_STATUS_OK);
                assert(lib_storage_file_byte_count(&file, &length) == LIB_STATUS_OK && length == 3);
                assert(storage_file_platform_tell(&file) == 1);
            }
            assert(lib_storage_file_close(&file) == LIB_STATUS_OK);
            assert(closes == before + 1u && file.stream == NULL);
            reject_open = 1;
            assert(storage_file_platform_open("fixture", write, &file) == LIB_STATUS_IO_ERROR);
            assert(file.stream == NULL && closes == before + 1u);
            reject_open = 0;
        }
    }
    return 0;
}
