#include "lib/base/base.h"

#include <fcntl.h>
#include <unistd.h>

#include "lib/storage/native.h"

FILE *lib_storage_native_open_exclusive_write(const char *path)
{
    int descriptor;
    FILE *file;

    if (path == LIB_NULL) return LIB_NULL;
    descriptor = open(path, O_WRONLY | O_CREAT | O_EXCL, 0600);
    if (descriptor < 0) return LIB_NULL;
    file = fdopen(descriptor, "wb");
    if (file != LIB_NULL) return file;
    (void)close(descriptor);
    (void)remove(path);
    return LIB_NULL;
}

int lib_storage_native_seek_64(FILE *file, int64_t offset, int origin)
{ return fseeko(file, (off_t)offset, origin); }

int64_t lib_storage_native_tell_64(FILE *file)
{ return (int64_t)ftello(file); }

int lib_storage_native_file_replace(const char *source, const char *destination)
{
    return source == LIB_NULL || destination == LIB_NULL ||
        rename(source, destination) != 0 ? LIB_TRUE : LIB_FALSE;
}

int lib_storage_native_file_remove(const char *path)
{ return path == LIB_NULL || remove(path) != 0 ? LIB_TRUE : LIB_FALSE; }
