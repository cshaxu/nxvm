#include "lib/base/base.h"
#include "lib/storage/medium.h"

#include <stdio.h>

static const char storage_medium_path[] = "storage-medium-smoke.img";

static int storage_medium_write_fixture(const void *bytes, size_t byte_count)
{
    FILE *file = fopen(storage_medium_path, "wb");
    int failed;

    if (file == LIB_NULL) return 1;
    failed = fwrite(bytes, 1u, byte_count, file) != byte_count;
    return fclose(file) != 0 || failed;
}

int main(void)
{
    static lib_u8 source[8193u];
    lib_storage_medium *direct = LIB_NULL;
    lib_storage_medium *readonly = LIB_NULL;
    lib_storage_medium *overlay = LIB_NULL;
    lib_u8 value = 0u;
    int failed = 0;

    source[0u] = 0x11u;
    source[1u] = 0x22u;
    source[4096u] = 0x33u;
    if (storage_medium_write_fixture(source, sizeof(source)) ||
        lib_storage_medium_open(storage_medium_path, LIB_STORAGE_MEDIUM_DIRECT,
            &direct) != LIB_STATUS_OK ||
        lib_storage_medium_write_at(direct, 1u, "Z", 1u) != LIB_STATUS_OK ||
        lib_storage_medium_read_at(direct, 1u, &value, 1u) != LIB_STATUS_OK ||
        value != (lib_u8)'Z') failed = 1;
    lib_storage_medium_destroy(direct);
    direct = LIB_NULL;
    if (!failed && (lib_storage_medium_open(storage_medium_path,
            LIB_STORAGE_MEDIUM_READONLY, &readonly) != LIB_STATUS_OK ||
        lib_storage_medium_read_at(readonly, 1u, &value, 1u) != LIB_STATUS_OK ||
        value != (lib_u8)'Z' || lib_storage_medium_write_at(readonly, 1u,
            "Q", 1u) != LIB_STATUS_INVALID_STATE)) failed = 1;
    lib_storage_medium_destroy(readonly);
    readonly = LIB_NULL;
    if (!failed && (lib_storage_medium_open(storage_medium_path,
            LIB_STORAGE_MEDIUM_OVERLAY, &overlay) != LIB_STATUS_OK ||
        lib_storage_medium_open(storage_medium_path, LIB_STORAGE_MEDIUM_DIRECT,
            &direct) != LIB_STATUS_OK ||
        lib_storage_medium_write_at(direct, 4096u, "R", 1u) != LIB_STATUS_OK ||
        lib_storage_medium_read_at(overlay, 4096u, &value, 1u) != LIB_STATUS_OK ||
        value != (lib_u8)'R' ||
        lib_storage_medium_write_at(overlay, 1u, "Q", 1u) != LIB_STATUS_OK ||
        lib_storage_medium_read_at(overlay, 1u, &value, 1u) != LIB_STATUS_OK ||
        value != (lib_u8)'Q' ||
        lib_storage_medium_write_at(direct, 0u, "P", 1u) != LIB_STATUS_OK ||
        lib_storage_medium_read_at(overlay, 0u, &value, 1u) != LIB_STATUS_OK ||
        value != 0x11u ||
        lib_storage_medium_read_at(direct, 1u, &value, 1u) != LIB_STATUS_OK ||
        value != (lib_u8)'Z')) failed = 1;
    lib_storage_medium_destroy(direct);
    lib_storage_medium_destroy(overlay);
    (void)remove(storage_medium_path);
    return failed;
}
