#include "lib/types/file.h"
#include "lib/types/win32/scalar.h"
#include "lib/types/win32/sync.h"
#include "lib/types/types_interface.h"

#include "lib/types/win32/file.h"
#include "lib/types/win32/text.h"

#include "lib/storage/file.h"

static lib_status storage_file_path(const char *path, lib_win32_wchar **out_path)
{
    lib_i32 count;
    lib_win32_wchar *wide;
    *out_path = LIB_NULL;
    count = lib_win32_multi_byte_to_wide_char(LIB_WIN32_CP_UTF8,
        LIB_WIN32_MB_ERR_INVALID_CHARS, path, -1, LIB_NULL, 0);
    if (count == 0) return LIB_STATUS_INVALID_ARGUMENT;
    if ((lib_size)count > LIB_SIZE_MAX / sizeof(*wide)) return LIB_STATUS_LIMIT_EXCEEDED;
    wide = lib_allocate((lib_size)count * sizeof(*wide));
    if (wide == LIB_NULL) return LIB_STATUS_NO_MEMORY;
    if (lib_win32_multi_byte_to_wide_char(LIB_WIN32_CP_UTF8,
        LIB_WIN32_MB_ERR_INVALID_CHARS, path, -1, wide, count) != count) {
        lib_release(wide);
        return LIB_STATUS_INVALID_ARGUMENT;
    }
    *out_path = wide;
    return LIB_STATUS_OK;
}

lib_status storage_file_platform_open(const char *path, lib_bool readwrite,
    lib_storage_file *file)
{
    lib_win32_handle handle;
    lib_i32 descriptor;
    lib_win32_wchar *wide;
    lib_status status;
    lib_win32_dword access_flags = readwrite != LIB_FALSE ?
        LIB_WIN32_GENERIC_READ | LIB_WIN32_GENERIC_WRITE : LIB_WIN32_GENERIC_READ;
    lib_win32_dword share = readwrite != LIB_FALSE ? 0u : LIB_WIN32_FILE_SHARE_READ;

    file->stream = LIB_NULL;
    status = storage_file_path(path, &wide);
    if (status != LIB_STATUS_OK) return status;
    handle = lib_win32_create_file_w(wide, access_flags, share, LIB_NULL, LIB_WIN32_OPEN_EXISTING,
        LIB_WIN32_FILE_ATTRIBUTE_NORMAL, LIB_NULL);
    lib_release(wide);
    if (handle == LIB_WIN32_INVALID_HANDLE_VALUE) return LIB_STATUS_IO_ERROR;
    descriptor = lib_win32_open_osfhandle((lib_iptr)handle,
        readwrite != LIB_FALSE ? LIB_WIN32_O_RDWR | LIB_WIN32_O_BINARY : LIB_WIN32_O_RDONLY | LIB_WIN32_O_BINARY);
    if (descriptor == -1) {
        (void)lib_win32_close_handle(handle);
        return LIB_STATUS_IO_ERROR;
    }
    file->stream = lib_win32_fdopen(descriptor,
        readwrite != LIB_FALSE ? "rb+" : "rb");
    if (file->stream == LIB_NULL) {
        (void)lib_win32_close(descriptor);
        return LIB_STATUS_IO_ERROR;
    }
    return LIB_STATUS_OK;
}

lib_status storage_file_platform_writer_open(const char *path, lib_bool append,
    lib_storage_file *file)
{
    lib_win32_wchar *wide;
    lib_status status;
    file->stream = LIB_NULL;
    status = storage_file_path(path, &wide);
    if (status != LIB_STATUS_OK) return status;
    file->stream = lib_win32_wfopen(wide, append ? L"ab" : L"wb");
    lib_release(wide);
    return file->stream != LIB_NULL ? LIB_STATUS_OK : LIB_STATUS_IO_ERROR;
}

lib_status storage_file_platform_seek(const lib_storage_file *file, lib_i64 offset,
    lib_i32 origin)
{ return lib_win32_fseeki64(file->stream, offset, origin) == 0 ? LIB_STATUS_OK : LIB_STATUS_IO_ERROR; }

lib_i64 storage_file_platform_tell(const lib_storage_file *file)
{ return lib_win32_ftelli64(file->stream); }
