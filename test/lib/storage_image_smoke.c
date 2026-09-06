#include "lib/storage/image.h"
#include "lib/storage/file.h"

typedef struct storage_image_mode_row {
    lib_storage_image_mode mode;
    int writable;
} storage_image_mode_row;

int main(void)
{
    const lib_u8 source[] = { 1u, 2u, 3u };
    static const storage_image_mode_row rows[] = {
        { LIB_STORAGE_IMAGE_OVERLAY, LIB_TRUE },
        { LIB_STORAGE_IMAGE_DIRECT_READONLY, LIB_FALSE },
        { LIB_STORAGE_IMAGE_DIRECT_WRITABLE, LIB_TRUE }
    };
    lib_storage_image *overlay = LIB_NULL;
    lib_storage_image *image = LIB_NULL;
    lib_storage_image *replacement = LIB_NULL;
    lib_storage_image *retired = LIB_NULL;
    lib_u8 *owned;
    lib_u8 *bytes;
    size_t index;

    if (lib_storage_image_create_overlay(source, sizeof(source), &overlay) !=
            LIB_STATUS_OK || lib_storage_image_mode_of(overlay) !=
            LIB_STORAGE_IMAGE_OVERLAY || lib_storage_image_byte_count(overlay) !=
            sizeof(source)) goto failed;
    bytes = lib_storage_image_writable_bytes(overlay);
    if (bytes == LIB_NULL || bytes[0u] != source[0u]) goto failed;
    bytes[0u] = 9u;
    if (source[0u] != 1u) goto failed;
    if (lib_storage_image_create_zero_overlay(sizeof(source), &replacement) !=
            LIB_STATUS_OK || lib_storage_image_replace(&overlay, replacement,
            &retired) != LIB_STATUS_OK || retired == LIB_NULL ||
        lib_storage_image_const_bytes(overlay) == lib_storage_image_const_bytes(retired) ||
        lib_storage_image_replace(&overlay, overlay, &image) !=
            LIB_STATUS_INVALID_ARGUMENT) goto failed;
    if (lib_storage_image_replace(&overlay, LIB_NULL, &retired) !=
            LIB_STATUS_INVALID_ARGUMENT || overlay == LIB_NULL) goto failed;
    replacement = LIB_NULL;
    lib_storage_image_destroy(retired);
    retired = LIB_NULL;
    if (lib_storage_image_replace(&overlay, LIB_NULL, &retired) != LIB_STATUS_OK ||
        overlay != LIB_NULL || retired == LIB_NULL) goto failed;
    lib_storage_image_destroy(retired);
    retired = LIB_NULL;
    if (lib_storage_image_replace(&overlay, LIB_NULL, &retired) != LIB_STATUS_OK ||
        overlay != LIB_NULL || retired != LIB_NULL) goto failed;
    for (index = 0u; index < sizeof(rows) / sizeof(rows[0]); ++index) {
        lib_status status;

        if (rows[index].mode == LIB_STORAGE_IMAGE_OVERLAY) {
            status = lib_storage_image_create_overlay(source, sizeof(source), &image);
        } else {
            owned = malloc(sizeof(source));
            if (owned == LIB_NULL) goto failed;
            memcpy(owned, source, sizeof(source));
            status = rows[index].mode == LIB_STORAGE_IMAGE_DIRECT_READONLY ?
                lib_storage_image_take_direct_readonly(owned, sizeof(source), &image) :
                lib_storage_image_take_direct_writable(owned, sizeof(source), &image);
        }
        if (status !=
            LIB_STATUS_OK || lib_storage_image_mode_of(image) != rows[index].mode ||
            (lib_storage_image_writable_bytes(image) != LIB_NULL) != rows[index].writable) {
            if (rows[index].mode != LIB_STORAGE_IMAGE_OVERLAY && image == LIB_NULL) {
                free(owned);
            }
            goto failed;
        }
        if (rows[index].writable) {
            ((lib_u8 *)lib_storage_image_writable_bytes(image))[0u] = 9u;
            if (lib_storage_image_commit(image, "storage-image-smoke.img") !=
                LIB_STATUS_OK || lib_storage_file_remove("storage-image-smoke.img"))
                goto failed;
        } else if (lib_storage_image_commit(image, "storage-image-smoke.img") !=
            LIB_STATUS_INVALID_ARGUMENT) goto failed;
        lib_storage_image_discard(&image);
    }
    lib_storage_image_destroy(overlay);
    puts("M5:T523:S4:STORAGE-IMAGE:OK");
    return 0;
failed:
    lib_storage_image_destroy(retired);
    lib_storage_image_destroy(replacement);
    lib_storage_image_discard(&image);
    (void)lib_storage_file_remove("storage-image-smoke.img");
    lib_storage_image_destroy(overlay);
    return 1;
}
