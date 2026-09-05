#include "type.h"

#include "lib/storage/image.h"

C_INT main(C_VOID)
{
    const type_unsigned_8 source[] = { 1u, 2u, 3u };
    lib_storage_image *overlay = STD_NULL;
    lib_storage_image *direct = STD_NULL;
    type_unsigned_8 *owned;
    type_unsigned_8 *bytes;

    if (lib_storage_image_create_overlay(source, sizeof(source), &overlay) !=
            TYPE_STATUS_OK || lib_storage_image_mode_of(overlay) !=
            LIB_STORAGE_IMAGE_OVERLAY || lib_storage_image_byte_count(overlay) !=
            sizeof(source)) goto failed;
    bytes = lib_storage_image_writable_bytes(overlay);
    if (bytes == STD_NULL || bytes[0u] != source[0u]) goto failed;
    bytes[0u] = 9u;
    if (source[0u] != 1u) goto failed;
    owned = STD_MALLOC(sizeof(source));
    if (owned == STD_NULL) goto failed;
    STD_MEMCPY(owned, source, sizeof(source));
    if (lib_storage_image_take_direct_readonly(owned, sizeof(source), &direct) !=
            TYPE_STATUS_OK || lib_storage_image_mode_of(direct) !=
            LIB_STORAGE_IMAGE_DIRECT_READONLY ||
        lib_storage_image_writable_bytes(direct) != STD_NULL ||
        ((const type_unsigned_8 *)lib_storage_image_const_bytes(direct))[2u] != 3u) {
        goto failed;
    }
    lib_storage_image_destroy(direct);
    lib_storage_image_destroy(overlay);
    puts("M5:T522:S9:STORAGE-IMAGE:OK");
    return 0;
failed:
    lib_storage_image_destroy(direct);
    lib_storage_image_destroy(overlay);
    return 1;
}
