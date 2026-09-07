#include "lib/base/base.h"

#include <fcntl.h>
#include <io.h>
#include <windows.h>

#include "lib/storage/internal/native.h"

FILE *lib_storage_native_open(const char *path, lib_storage_medium_mode mode)
{
    HANDLE handle;
    DWORD access = mode == LIB_STORAGE_MEDIUM_DIRECT ?
        GENERIC_READ | GENERIC_WRITE : GENERIC_READ;
    DWORD share = mode == LIB_STORAGE_MEDIUM_DIRECT ? 0u : FILE_SHARE_READ;
    int descriptor;
    FILE *file;

    handle = CreateFileA(path, access, share, LIB_NULL, OPEN_EXISTING,
        FILE_ATTRIBUTE_NORMAL, LIB_NULL);
    if (handle == INVALID_HANDLE_VALUE) return LIB_NULL;
    descriptor = _open_osfhandle((intptr_t)handle, mode == LIB_STORAGE_MEDIUM_DIRECT ?
        _O_RDWR | _O_BINARY : _O_RDONLY | _O_BINARY);
    if (descriptor == -1) {
        (void)CloseHandle(handle);
        return LIB_NULL;
    }
    file = _fdopen(descriptor, mode == LIB_STORAGE_MEDIUM_DIRECT ? "rb+" : "rb");
    if (file == LIB_NULL) (void)_close(descriptor);
    return file;
}

int lib_storage_native_seek_64(FILE *file, lib_i64 offset, int origin)
{ return _fseeki64(file, offset, origin); }

lib_i64 lib_storage_native_tell_64(FILE *file)
{ return _ftelli64(file); }
