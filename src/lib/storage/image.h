#ifndef LIB_STORAGE_IMAGE_H
#define LIB_STORAGE_IMAGE_H

#include "lib/base/base.h"

typedef enum lib_storage_image_mode {
    LIB_STORAGE_IMAGE_DIRECT_READONLY,
    LIB_STORAGE_IMAGE_DIRECT_WRITABLE,
    LIB_STORAGE_IMAGE_OVERLAY
} lib_storage_image_mode;

typedef struct lib_storage_image lib_storage_image;

/* An image is an exclusive byte lease: callers do not concurrently access,
 * replace or destroy it.  Direct images take ownership of bytes; overlay
 * images own a private copy. */
lib_status lib_storage_image_take_direct_readonly(void *bytes,
    size_t byte_count, lib_storage_image **out_image);
/* Takes ownership of bytes and exposes the one writable direct view. */
lib_status lib_storage_image_take_direct_writable(void *bytes,
    size_t byte_count, lib_storage_image **out_image);
/* Creates one private writable byte image from source. */
lib_status lib_storage_image_create_overlay(const void *bytes,
    size_t byte_count, lib_storage_image **out_image);
lib_status lib_storage_image_create_zero_overlay(size_t byte_count,
    lib_storage_image **out_image);
void lib_storage_image_destroy(lib_storage_image *image);
const void *lib_storage_image_const_bytes(const lib_storage_image *image);
void *lib_storage_image_writable_bytes(lib_storage_image *image);
size_t lib_storage_image_byte_count(const lib_storage_image *image);
lib_storage_image_mode lib_storage_image_mode_of(const lib_storage_image *image);
/* Transfers replacement (or no lease) into the caller-owned lease at its safe
 * point and returns the retired lease through an initially-null output for
 * later destruction. A no-lease to no-lease transfer succeeds. It does not
 * destroy or change bytes. */
lib_status lib_storage_image_replace(lib_storage_image **lease,
    lib_storage_image *replacement, lib_storage_image **out_retired);
/* Commit never changes the lease or its bytes.  A failure leaves both intact. */
lib_status lib_storage_image_commit(const lib_storage_image *image,
    const char *path);
void lib_storage_image_discard(lib_storage_image **image);

#endif
