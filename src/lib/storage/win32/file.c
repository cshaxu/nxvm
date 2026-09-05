#include "type.h"

#include <windows.h>

#include "lib/storage/native.h"

C_INT lib_storage_native_file_replace(const C_CHAR *source, const C_CHAR *destination)
{
    return source == STD_NULL || destination == STD_NULL ||
        !MoveFileExA(source, destination, MOVEFILE_REPLACE_EXISTING | MOVEFILE_WRITE_THROUGH);
}

C_INT lib_storage_native_file_remove(const C_CHAR *path)
{ return path == STD_NULL || !DeleteFileA(path); }
