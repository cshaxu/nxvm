#include "lib/base/base.h"

#include "lib/storage/native.h"

int lib_storage_native_file_replace(const char *source, const char *destination)
{
    return source == LIB_NULL || destination == LIB_NULL ||
        rename(source, destination) != 0 ? LIB_TRUE : LIB_FALSE;
}

int lib_storage_native_file_remove(const char *path)
{ return path == LIB_NULL || remove(path) != 0 ? LIB_TRUE : LIB_FALSE; }
