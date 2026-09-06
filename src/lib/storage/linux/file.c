#include "lib/base/base.h"

#include <sys/types.h>

#include "lib/storage/native.h"

int lib_storage_native_seek_64(FILE *file, int64_t offset, int origin)
{ return fseeko(file, (off_t)offset, origin); }

int64_t lib_storage_native_tell_64(FILE *file)
{ return (int64_t)ftello(file); }
