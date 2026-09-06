#ifndef LIB_STORAGE_NATIVE_H
#define LIB_STORAGE_NATIVE_H

#include "lib/base/base.h"

int lib_storage_native_seek_64(FILE *file, int64_t offset, int origin);
int64_t lib_storage_native_tell_64(FILE *file);

#endif
