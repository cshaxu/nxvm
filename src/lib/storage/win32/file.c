#include "lib/types/file.h"
#include "lib/types/win32/scalar.h"
#include "lib/types/win32/sync.h"
#include "lib/types/types_interface.h"

#include "lib/types/win32/file.h"

#include "lib/storage/file.h"

static lib_status storage_file_platform_open(const char *path, lib_bool readwrite,
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

lib_status storage_file_platform_open_readonly(const char *path,
    lib_storage_file *file)
{ return storage_file_platform_open(path, LIB_FALSE, file); }

lib_status storage_file_platform_open_readwrite(const char *path,
    lib_storage_file *file)
{ return storage_file_platform_open(path, LIB_TRUE, file); }

lib_status storage_file_platform_seek_absolute(lib_storage_file *file, lib_i64 offset)
{ return lib_win32_fseeki64(file->stream, offset, LIB_SEEK_SET) == 0 ? LIB_STATUS_OK : LIB_STATUS_IO_ERROR; }

lib_status storage_file_platform_byte_count(lib_storage_file *file, lib_i64 *out_byte_count)
{
    lib_i64 offset = lib_win32_ftelli64(file->stream);
    lib_i64 length;

    if (offset < 0 || lib_win32_fseeki64(file->stream, 0, LIB_SEEK_END) != 0)
        return LIB_STATUS_IO_ERROR;
    length = lib_win32_ftelli64(file->stream);
    if (length < 0 || lib_win32_fseeki64(file->stream, offset, LIB_SEEK_SET) != 0)
        return LIB_STATUS_IO_ERROR;
    *out_byte_count = length;
    return LIB_STATUS_OK;
}
