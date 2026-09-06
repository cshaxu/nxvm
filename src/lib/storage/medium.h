#ifndef LIB_STORAGE_MEDIUM_H
#define LIB_STORAGE_MEDIUM_H

#include "lib/base/base.h"

typedef enum lib_storage_medium_mode {
    LIB_STORAGE_MEDIUM_DIRECT,
    LIB_STORAGE_MEDIUM_READONLY,
    LIB_STORAGE_MEDIUM_OVERLAY
} lib_storage_medium_mode;

typedef struct lib_storage_medium lib_storage_medium;

/* A medium is one exclusive byte-addressed lease. DIRECT and READONLY retain
 * their opened backing file; OVERLAY retains its readonly base and sparse
 * 4-KiB dirty pages. */
lib_status lib_storage_medium_open(const char *path, lib_storage_medium_mode mode,
    lib_storage_medium **out_medium);
lib_status lib_storage_medium_create_overlay(const void *bytes, size_t byte_count,
    lib_storage_medium **out_medium);
lib_status lib_storage_medium_create_zero_overlay(size_t byte_count,
    lib_storage_medium **out_medium);
void lib_storage_medium_destroy(lib_storage_medium *medium);

size_t lib_storage_medium_byte_count(const lib_storage_medium *medium);
lib_storage_medium_mode lib_storage_medium_mode_of(const lib_storage_medium *medium);
lib_status lib_storage_medium_read_at(const lib_storage_medium *medium,
    size_t offset, void *bytes, size_t byte_count);
lib_status lib_storage_medium_write_at(lib_storage_medium *medium,
    size_t offset, const void *bytes, size_t byte_count);
lib_status lib_storage_medium_fill_at(lib_storage_medium *medium,
    size_t offset, size_t byte_count, lib_u8 value);
lib_status lib_storage_medium_flush(lib_storage_medium *medium);
void lib_storage_medium_discard(lib_storage_medium **medium);

/* Replacement is the sole generic transfer of a live medium lease. */
lib_status lib_storage_medium_replace(lib_storage_medium **lease,
    lib_storage_medium *replacement, lib_storage_medium **out_retired);

#endif
