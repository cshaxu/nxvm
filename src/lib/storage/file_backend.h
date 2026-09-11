#ifndef LIB_STORAGE_FILE_BACKEND_H
#define LIB_STORAGE_FILE_BACKEND_H

#include "lib/types/types_interface.h"

#include "lib/storage/medium_interface.h"

#include <stdio.h>

FILE *lib_storage_file_backend_open(const char *path, lib_storage_medium_mode mode);
int lib_storage_file_backend_seek_64(FILE *file, lib_i64 offset, int origin);
lib_i64 lib_storage_file_backend_tell_64(FILE *file);

#endif
