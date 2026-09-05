#ifndef LIB_STORAGE_IMAGE_H
#define LIB_STORAGE_IMAGE_H

#include "type.h"

typedef enum lib_storage_image_mode {
    LIB_STORAGE_IMAGE_DIRECT_READONLY,
    LIB_STORAGE_IMAGE_OVERLAY
} lib_storage_image_mode;

typedef struct lib_storage_image lib_storage_image;

/* Takes ownership of bytes.  Direct images expose no mutable byte view. */
type_status lib_storage_image_take_direct_readonly(C_VOID *bytes,
    STD_SIZE_T byte_count, lib_storage_image **out_image);
/* Creates one private writable byte image from source. */
type_status lib_storage_image_create_overlay(const C_VOID *bytes,
    STD_SIZE_T byte_count, lib_storage_image **out_image);
type_status lib_storage_image_create_zero_overlay(STD_SIZE_T byte_count,
    lib_storage_image **out_image);
C_VOID lib_storage_image_destroy(lib_storage_image *image);
const C_VOID *lib_storage_image_const_bytes(const lib_storage_image *image);
C_VOID *lib_storage_image_writable_bytes(lib_storage_image *image);
STD_SIZE_T lib_storage_image_byte_count(const lib_storage_image *image);
lib_storage_image_mode lib_storage_image_mode_of(const lib_storage_image *image);

#endif
