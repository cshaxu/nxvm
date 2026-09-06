#ifndef LIB_STORAGE_COMMIT_H
#define LIB_STORAGE_COMMIT_H

#include "lib/base/base.h"

int lib_storage_commit_atomically(const char *path, const void *bytes,
    size_t byte_count);
int lib_storage_commit_pair_atomically(const char *first_path,
    const void *first_bytes, size_t first_byte_count, const char *second_path,
    const void *second_bytes, size_t second_byte_count);

#endif
