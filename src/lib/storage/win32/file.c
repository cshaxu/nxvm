#include "lib/base/base.h"

#include <windows.h>
#include <fcntl.h>
#include <io.h>
#include <sys/stat.h>

#include "lib/storage/native.h"

FILE *lib_storage_native_open_exclusive_write(const char *path)
{
    int descriptor;
    FILE *file;

    if (path == LIB_NULL) return LIB_NULL;
    descriptor = _open(path, _O_WRONLY | _O_CREAT | _O_EXCL | _O_BINARY,
        _S_IREAD | _S_IWRITE);
    if (descriptor < 0) return LIB_NULL;
    file = _fdopen(descriptor, "wb");
    if (file != LIB_NULL) return file;
    (void)_close(descriptor);
    (void)DeleteFileA(path);
    return LIB_NULL;
}

int lib_storage_native_seek_64(FILE *file, int64_t offset, int origin)
{ return _fseeki64(file, offset, origin); }

int64_t lib_storage_native_tell_64(FILE *file)
{ return _ftelli64(file); }

int lib_storage_native_file_replace(const char *source, const char *destination)
{
    return source == LIB_NULL || destination == LIB_NULL ||
        !MoveFileExA(source, destination, MOVEFILE_REPLACE_EXISTING | MOVEFILE_WRITE_THROUGH);
}

int lib_storage_native_file_remove(const char *path)
{ return path == LIB_NULL || !DeleteFileA(path); }
