#include "lib/base/base.h"

#include <windows.h>

#include "lib/storage/native.h"

int lib_storage_native_file_replace(const char *source, const char *destination)
{
    return source == LIB_NULL || destination == LIB_NULL ||
        !MoveFileExA(source, destination, MOVEFILE_REPLACE_EXISTING | MOVEFILE_WRITE_THROUGH);
}

int lib_storage_native_file_remove(const char *path)
{ return path == LIB_NULL || !DeleteFileA(path); }
