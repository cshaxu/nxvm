#ifndef LIB_STORAGE_NATIVE_H
#define LIB_STORAGE_NATIVE_H

#include "type.h"

C_INT lib_storage_native_file_replace(const C_CHAR *source, const C_CHAR *destination);
C_INT lib_storage_native_file_remove(const C_CHAR *path);

#endif
