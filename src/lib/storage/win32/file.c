#include "lib/base/base.h"

#include "lib/storage/native.h"

int lib_storage_native_seek_64(FILE *file, int64_t offset, int origin)
{ return _fseeki64(file, offset, origin); }

int64_t lib_storage_native_tell_64(FILE *file)
{ return _ftelli64(file); }
