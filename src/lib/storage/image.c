#include "type.h"

#include "lib/storage/image.h"

struct lib_storage_image {
    C_VOID *bytes;
    STD_SIZE_T byte_count;
    lib_storage_image_mode mode;
};

static type_status lib_storage_image_create(C_VOID *bytes, STD_SIZE_T byte_count,
    lib_storage_image_mode mode, lib_storage_image **out_image)
{
    lib_storage_image *image;

    if (out_image == STD_NULL || (byte_count != 0u && bytes == STD_NULL)) {
        return TYPE_STATUS_INVALID_ARGUMENT;
    }
    *out_image = STD_NULL;
    image = STD_MALLOC(sizeof(*image));
    if (image == STD_NULL) return TYPE_STATUS_NO_MEMORY;
    *image = (lib_storage_image) { bytes, byte_count, mode };
    *out_image = image;
    return TYPE_STATUS_OK;
}

type_status lib_storage_image_take_direct_readonly(C_VOID *bytes,
    STD_SIZE_T byte_count, lib_storage_image **out_image)
{
    return lib_storage_image_create(bytes, byte_count,
        LIB_STORAGE_IMAGE_DIRECT_READONLY, out_image);
}

type_status lib_storage_image_create_overlay(const C_VOID *bytes,
    STD_SIZE_T byte_count, lib_storage_image **out_image)
{
    C_VOID *copy;

    if ((byte_count != 0u && bytes == STD_NULL) || out_image == STD_NULL) {
        return TYPE_STATUS_INVALID_ARGUMENT;
    }
    copy = byte_count == 0u ? STD_NULL : STD_MALLOC(byte_count);
    if (byte_count != 0u && copy == STD_NULL) return TYPE_STATUS_NO_MEMORY;
    if (byte_count != 0u) STD_MEMCPY(copy, bytes, byte_count);
    if (lib_storage_image_create(copy, byte_count, LIB_STORAGE_IMAGE_OVERLAY,
            out_image) != TYPE_STATUS_OK) {
        STD_FREE(copy);
        return TYPE_STATUS_NO_MEMORY;
    }
    return TYPE_STATUS_OK;
}

type_status lib_storage_image_create_zero_overlay(STD_SIZE_T byte_count,
    lib_storage_image **out_image)
{
    C_VOID *bytes = byte_count == 0u ? STD_NULL : STD_CALLOC(byte_count, 1u);

    if (out_image == STD_NULL) return TYPE_STATUS_INVALID_ARGUMENT;
    if (byte_count != 0u && bytes == STD_NULL) return TYPE_STATUS_NO_MEMORY;
    if (lib_storage_image_create(bytes, byte_count, LIB_STORAGE_IMAGE_OVERLAY,
            out_image) != TYPE_STATUS_OK) {
        STD_FREE(bytes);
        return TYPE_STATUS_NO_MEMORY;
    }
    return TYPE_STATUS_OK;
}

C_VOID lib_storage_image_destroy(lib_storage_image *image)
{
    if (image == STD_NULL) return;
    STD_FREE(image->bytes);
    STD_FREE(image);
}

const C_VOID *lib_storage_image_const_bytes(const lib_storage_image *image)
{ return image == STD_NULL ? STD_NULL : image->bytes; }

C_VOID *lib_storage_image_writable_bytes(lib_storage_image *image)
{
    return image == STD_NULL || image->mode != LIB_STORAGE_IMAGE_OVERLAY ?
        STD_NULL : image->bytes;
}

STD_SIZE_T lib_storage_image_byte_count(const lib_storage_image *image)
{ return image == STD_NULL ? 0u : image->byte_count; }

lib_storage_image_mode lib_storage_image_mode_of(const lib_storage_image *image)
{
    return image == STD_NULL ? LIB_STORAGE_IMAGE_DIRECT_READONLY : image->mode;
}
