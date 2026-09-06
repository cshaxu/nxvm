#ifndef LIB_STORAGE_NATIVE_H
#define LIB_STORAGE_NATIVE_H

#include "lib/base/base.h"

FILE *lib_storage_native_open_exclusive_write(const char *path);
int lib_storage_native_seek_64(FILE *file, int64_t offset, int origin);
int64_t lib_storage_native_tell_64(FILE *file);
int lib_storage_native_file_replace(const char *source, const char *destination);
int lib_storage_native_file_remove(const char *path);

#endif
