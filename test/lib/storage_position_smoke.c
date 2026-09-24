#include "lib/types/test.h"
#include "lib/types/file.h"
#include "lib/storage/file.h"

static lib_u32 operation, fail_at;
static lib_i64 position, length;
static lib_status seek_position(const lib_storage_file *file, lib_i64 offset, lib_i32 origin)
{
    (void)file;
    if (++operation == fail_at) return LIB_STATUS_IO_ERROR;
    position = origin == LIB_SEEK_END ? length + offset : offset;
    return LIB_STATUS_OK;
}
static lib_i64 tell_position(const lib_storage_file *file)
{
    (void)file;
    return ++operation == fail_at ? -1 : position;
}
#define storage_file_platform_seek seek_position
#define storage_file_platform_tell tell_position
#include "lib/storage/file.c"

int main(void)
{
    lib_storage_file file = { 0 };
    lib_i64 count;
    /* Simulate >4 GiB on both host widths without creating a huge file. */
    length = (lib_i64)0x100000000ULL + 7;
    for (fail_at = 0; fail_at <= 4; ++fail_at) {
        position = 23;
        operation = 0;
        count = -99;
        lib_status status = lib_storage_file_byte_count(&file, &count);
        lib_test_assert(status == (fail_at ? LIB_STATUS_IO_ERROR : LIB_STATUS_OK));
        lib_test_assert(operation == (fail_at ? fail_at : 4));
        lib_test_assert(count == (fail_at ? -99 : length));
        if (!fail_at) lib_test_assert(position == 23);
    }
    lib_test_assert(lib_storage_file_byte_count(LIB_NULL, &count) == LIB_STATUS_INVALID_ARGUMENT);
    lib_test_assert(lib_storage_file_byte_count(&file, LIB_NULL) == LIB_STATUS_INVALID_ARGUMENT);
    return 0;
}
