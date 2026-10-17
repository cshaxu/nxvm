#include "lib/types/test.h"
#include "lib/types/types_interface.h"
#include "lib/types/win32/file.h"
#include "lib/types/win32/sync.h"
#include "lib/types/win32/text.h"

static lib_u32 stage, handle_closes, descriptor_closes, opens;
static lib_bool writable;
static lib_win32_handle open_file(const lib_win32_wchar *path, lib_win32_dword access,
    lib_win32_dword share, void *security, lib_win32_dword disposition,
    lib_win32_dword attributes, lib_win32_handle template_file)
{
    (void)security; (void)attributes; (void)template_file;
    ++opens;
    lib_test_assert(path[0] == L'f' && disposition == LIB_WIN32_OPEN_EXISTING);
    lib_test_assert(access == (LIB_WIN32_GENERIC_READ | (writable ? LIB_WIN32_GENERIC_WRITE : 0u)));
    lib_test_assert(share == (writable ? 0u : LIB_WIN32_FILE_SHARE_READ));
    return stage == 1u ? LIB_WIN32_INVALID_HANDLE_VALUE : (lib_win32_handle)(lib_uptr)7u;
}
static lib_i32 open_descriptor(lib_iptr handle, lib_i32 flags)
{
    lib_test_assert(handle == 7 && flags == (LIB_WIN32_O_BINARY |
        (writable ? LIB_WIN32_O_RDWR : LIB_WIN32_O_RDONLY)));
    return stage == 2u ? -1 : 8;
}
static lib_c_file *stream(lib_i32 fd, const char *mode)
{
    lib_test_assert(fd == 8 && lib_text_compare(mode, writable ? "rb+" : "rb") == 0);
    return stage == 3u ? LIB_NULL : lib_c_tmpfile();
}
static lib_win32_bool close_handle(lib_win32_handle handle)
{ lib_test_assert((lib_uptr)handle == 7u); ++handle_closes; return 1; }
static lib_i32 close_descriptor(lib_i32 fd)
{ lib_test_assert(fd == 8); ++descriptor_closes; return 0; }
#undef lib_win32_create_file_w
#undef lib_win32_open_osfhandle
#undef lib_win32_fdopen
#undef lib_win32_close_handle
#undef lib_win32_close
#define lib_win32_create_file_w open_file
#define lib_win32_open_osfhandle open_descriptor
#define lib_win32_fdopen stream
#define lib_win32_close_handle close_handle
#define lib_win32_close close_descriptor
#include "lib/storage/win32/file.c"
#include "lib/storage/file.c"

int main(void)
{
    lib_storage_file file = { 0 };
    lib_test_assert(storage_file_platform_open("\xed\xa0\x80", LIB_FALSE, &file) == LIB_STATUS_INVALID_ARGUMENT);
    lib_test_assert(storage_file_platform_writer_open("\xc0\xaf", LIB_FALSE, &file) == LIB_STATUS_INVALID_ARGUMENT);
    lib_test_assert(opens == 0u && file.stream == LIB_NULL);
    for (lib_u32 mode = 0u; mode < 2u; ++mode) {
        writable = mode != 0u;
        for (stage = 0u; stage < 4u; ++stage) {
            handle_closes = descriptor_closes = 0u;
            lib_test_assert(storage_file_platform_open("fixture", writable, &file) ==
                (stage ? LIB_STATUS_IO_ERROR : LIB_STATUS_OK));
            lib_test_assert((file.stream == LIB_NULL) == (stage != 0u));
            lib_test_assert(handle_closes == (stage == 2u ? 1u : 0u));
            lib_test_assert(descriptor_closes == (stage == 3u ? 1u : 0u));
            if (!stage) {
                lib_i64 length;
                lib_test_assert(lib_storage_file_write_exact(&file, "abc", 3u) == LIB_STATUS_OK);
                lib_test_assert(lib_storage_file_seek_absolute(&file, 1) == LIB_STATUS_OK);
                lib_test_assert(lib_storage_file_byte_count(&file, &length) == LIB_STATUS_OK && length == 3);
                lib_test_assert(storage_file_platform_tell(&file) == 1);
            }
            lib_test_assert(lib_storage_file_close(&file) == LIB_STATUS_OK && file.stream == LIB_NULL);
        }
    }
    return 0;
}
