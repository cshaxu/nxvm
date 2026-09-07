#include "lib/base/base.h"

#include "lib/storage/internal/native.h"

int lib_storage_native_seek_64(FILE *file, lib_i64 offset, int origin)
{ return _fseeki64(file, offset, origin); }

lib_i64 lib_storage_native_tell_64(FILE *file)
{ return _ftelli64(file); }
