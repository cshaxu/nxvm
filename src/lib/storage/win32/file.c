#include "lib/types/file.h"
#include "lib/types/win32/scalar.h"
#include "lib/types/win32/sync.h"
#include "lib/types/types_interface.h"

#include "lib/types/win32/file.h"

#include "lib/storage/file.h"

lib_status storage_file_platform_open(const char *path, lib_bool readwrite,
    lib_storage_file *file)
{
    lib_win32_handle handle;
    int descriptor;
    lib_win32_dword access_flags = readwrite != LIB_FALSE ?
        LIB_WIN32_GENERIC_READ | LIB_WIN32_GENERIC_WRITE : LIB_WIN32_GENERIC_READ;
    lib_win32_dword share = readwrite != LIB_FALSE ? 0u : LIB_WIN32_FILE_SHARE_READ;

    handle = lib_win32_create_file_a(path, access_flags, share, LIB_NULL, LIB_WIN32_OPEN_EXISTING,
        LIB_WIN32_FILE_ATTRIBUTE_NORMAL, LIB_NULL);
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

lib_status storage_file_platform_seek(const lib_storage_file *file, lib_i64 offset,
    int origin)
{ return lib_win32_fseeki64(file->stream, offset, origin) == 0 ? LIB_STATUS_OK : LIB_STATUS_IO_ERROR; }

lib_i64 storage_file_platform_tell(const lib_storage_file *file)
{ return lib_win32_ftelli64(file->stream); }
