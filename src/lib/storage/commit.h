#ifndef LIB_STORAGE_COMMIT_H
#define LIB_STORAGE_COMMIT_H

#include "type.h"

C_INT lib_storage_commit_atomically(const C_CHAR *path, const C_VOID *bytes,
    STD_SIZE_T byte_count);
C_INT lib_storage_commit_pair_atomically(const C_CHAR *first_path,
    const C_VOID *first_bytes, STD_SIZE_T first_byte_count, const C_CHAR *second_path,
    const C_VOID *second_bytes, STD_SIZE_T second_byte_count);

#endif
