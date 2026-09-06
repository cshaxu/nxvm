#ifndef LIB_STORAGE_NATIVE_H
#define LIB_STORAGE_NATIVE_H

#include "lib/base/base.h"

int lib_storage_native_file_replace(const char *source, const char *destination);
int lib_storage_native_file_remove(const char *path);

#endif
