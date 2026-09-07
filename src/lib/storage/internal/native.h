#ifndef LIB_STORAGE_NATIVE_H
#define LIB_STORAGE_NATIVE_H

#include "lib/base/base.h"

int lib_storage_native_seek_64(FILE *file, lib_i64 offset, int origin);
lib_i64 lib_storage_native_tell_64(FILE *file);

#endif
