#include "lib/base/base.h"

#include <sys/types.h>

#include "lib/storage/internal/native.h"

int lib_storage_native_seek_64(FILE *file, lib_i64 offset, int origin)
{ return fseeko(file, (off_t)offset, origin); }

lib_i64 lib_storage_native_tell_64(FILE *file)
{ return (int64_t)ftello(file); }
