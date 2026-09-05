#include "type.h"

#include "lib/storage/native.h"

C_INT lib_storage_native_file_replace(const C_CHAR *source, const C_CHAR *destination)
{
    return source == STD_NULL || destination == STD_NULL ||
        STD_RENAME_REPLACE(source, destination) != 0 ? TYPE_TRUE : TYPE_FALSE;
}

C_INT lib_storage_native_file_remove(const C_CHAR *path)
{ return path == STD_NULL || STD_REMOVE(path) != 0 ? TYPE_TRUE : TYPE_FALSE; }
