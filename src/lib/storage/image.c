#include "lib/base/base.h"

#include "lib/storage/image.h"
#include "lib/storage/commit.h"

struct lib_storage_image {
    void *bytes;
    size_t byte_count;
    lib_storage_image_mode mode;
};

static lib_status lib_storage_image_create(void *bytes, size_t byte_count,
    lib_storage_image_mode mode, lib_storage_image **out_image)
{
    lib_storage_image *image;

    if (out_image == LIB_NULL || (byte_count != 0u && bytes == LIB_NULL)) {
        return LIB_STATUS_INVALID_ARGUMENT;
    }
    *out_image = LIB_NULL;
    image = malloc(sizeof(*image));
    if (image == LIB_NULL) return LIB_STATUS_NO_MEMORY;
    *image = (lib_storage_image) { bytes, byte_count, mode };
    *out_image = image;
    return LIB_STATUS_OK;
}

lib_status lib_storage_image_take_direct_readonly(void *bytes,
    size_t byte_count, lib_storage_image **out_image)
{
    return lib_storage_image_create(bytes, byte_count,
        LIB_STORAGE_IMAGE_DIRECT_READONLY, out_image);
}

lib_status lib_storage_image_take_direct_writable(void *bytes,
    size_t byte_count, lib_storage_image **out_image)
{
    return lib_storage_image_create(bytes, byte_count,
        LIB_STORAGE_IMAGE_DIRECT_WRITABLE, out_image);
}

lib_status lib_storage_image_create_overlay(const void *bytes,
    size_t byte_count, lib_storage_image **out_image)
{
    void *copy;

    if ((byte_count != 0u && bytes == LIB_NULL) || out_image == LIB_NULL) {
        return LIB_STATUS_INVALID_ARGUMENT;
    }
    copy = byte_count == 0u ? LIB_NULL : malloc(byte_count);
    if (byte_count != 0u && copy == LIB_NULL) return LIB_STATUS_NO_MEMORY;
    if (byte_count != 0u) memcpy(copy, bytes, byte_count);
    if (lib_storage_image_create(copy, byte_count, LIB_STORAGE_IMAGE_OVERLAY,
            out_image) != LIB_STATUS_OK) {
        free(copy);
        return LIB_STATUS_NO_MEMORY;
    }
    return LIB_STATUS_OK;
}

lib_status lib_storage_image_create_zero_overlay(size_t byte_count,
    lib_storage_image **out_image)
{
    void *bytes = byte_count == 0u ? LIB_NULL : calloc(byte_count, 1u);

    if (out_image == LIB_NULL) return LIB_STATUS_INVALID_ARGUMENT;
    if (byte_count != 0u && bytes == LIB_NULL) return LIB_STATUS_NO_MEMORY;
    if (lib_storage_image_create(bytes, byte_count, LIB_STORAGE_IMAGE_OVERLAY,
            out_image) != LIB_STATUS_OK) {
        free(bytes);
        return LIB_STATUS_NO_MEMORY;
    }
    return LIB_STATUS_OK;
}

void lib_storage_image_destroy(lib_storage_image *image)
{
    if (image == LIB_NULL) return;
    free(image->bytes);
    free(image);
}

const void *lib_storage_image_const_bytes(const lib_storage_image *image)
{ return image == LIB_NULL ? LIB_NULL : image->bytes; }

void *lib_storage_image_writable_bytes(lib_storage_image *image)
{
    return image == LIB_NULL || image->mode == LIB_STORAGE_IMAGE_DIRECT_READONLY ?
        LIB_NULL : image->bytes;
}

size_t lib_storage_image_byte_count(const lib_storage_image *image)
{ return image == LIB_NULL ? 0u : image->byte_count; }

lib_storage_image_mode lib_storage_image_mode_of(const lib_storage_image *image)
{
    return image == LIB_NULL ? LIB_STORAGE_IMAGE_DIRECT_READONLY : image->mode;
}

lib_status lib_storage_image_commit(const lib_storage_image *image,
    const char *path)
{
    if (image == LIB_NULL || path == LIB_NULL ||
        image->mode == LIB_STORAGE_IMAGE_DIRECT_READONLY) {
        return LIB_STATUS_INVALID_ARGUMENT;
    }
    return lib_storage_commit_atomically(path, image->bytes, image->byte_count) ==
        LIB_FALSE ? LIB_STATUS_OK : LIB_STATUS_IO_ERROR;
}

void lib_storage_image_discard(lib_storage_image **image)
{
    if (image == LIB_NULL) return;
    lib_storage_image_destroy(*image);
    *image = LIB_NULL;
}
