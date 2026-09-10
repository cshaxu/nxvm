#include "lib/base/base_interface.h"
#include "lib/storage/medium_interface.h"

#include <stdio.h>
#include <process.h>

static const char storage_medium_path[] = "storage-medium-smoke.img";

static int storage_medium_write_fixture(const void *bytes, size_t byte_count)
{
    FILE *file = fopen(storage_medium_path, "wb");
    int failed;

    if (file == LIB_NULL) return 1;
    failed = fwrite(bytes, 1u, byte_count, file) != byte_count;
    return fclose(file) != 0 || failed;
}

static int storage_medium_child(const char *mode)
{
    lib_storage_medium *medium = LIB_NULL;
    lib_storage_medium_mode requested = mode[0] == 'd' ?
        LIB_STORAGE_MEDIUM_DIRECT : LIB_STORAGE_MEDIUM_READONLY;
    int result = lib_storage_medium_open(storage_medium_path, requested, &medium) ==
        LIB_STATUS_OK ? 0 : 1;

    lib_storage_medium_destroy(&medium);
    return result;
}

static int storage_medium_spawn(const char *program, const char *mode)
{
    const char *arguments[] = { program, mode, LIB_NULL };
    return _spawnv(_P_WAIT, program, arguments);
}

int main(int argc, char **argv)
{
    static lib_u8 source[8193u];
    lib_storage_medium *direct = LIB_NULL;
    lib_storage_medium *readonly = LIB_NULL;
    lib_storage_medium *second_readonly = LIB_NULL;
    lib_storage_medium *overlay = LIB_NULL;
    lib_u8 value = 0u;
    int failed = 0;

    if (argc == 2) return storage_medium_child(argv[1]);
    if (argc != 1) return 1;
    source[0u] = 0x11u;
    source[1u] = 0x22u;
    source[4096u] = 0x33u;
    if (storage_medium_write_fixture(source, sizeof(source)) ||
        lib_storage_medium_open(storage_medium_path, LIB_STORAGE_MEDIUM_DIRECT,
            &direct) != LIB_STATUS_OK ||
        lib_storage_medium_open(storage_medium_path, LIB_STORAGE_MEDIUM_READONLY,
            &readonly) != LIB_STATUS_IO_ERROR || readonly != LIB_NULL ||
        storage_medium_spawn(argv[0], "readonly") != 1 ||
        lib_storage_medium_open(storage_medium_path, LIB_STORAGE_MEDIUM_OVERLAY,
            &overlay) != LIB_STATUS_IO_ERROR || overlay != LIB_NULL ||
        lib_storage_medium_write_at(direct, 1u, "Z", 1u) != LIB_STATUS_OK ||
        lib_storage_medium_read_at(direct, 1u, &value, 1u) != LIB_STATUS_OK ||
        value != (lib_u8)'Z') failed = 1;
    lib_storage_medium_destroy(&direct);
    if (direct != LIB_NULL) failed = 1;
    if (!failed && (lib_storage_medium_open(storage_medium_path,
            LIB_STORAGE_MEDIUM_READONLY, &readonly) != LIB_STATUS_OK ||
        lib_storage_medium_open(storage_medium_path, LIB_STORAGE_MEDIUM_OVERLAY,
            &overlay) != LIB_STATUS_OK ||
        lib_storage_medium_open(storage_medium_path, LIB_STORAGE_MEDIUM_READONLY,
            &second_readonly) != LIB_STATUS_OK ||
        lib_storage_medium_open(storage_medium_path, LIB_STORAGE_MEDIUM_DIRECT,
            &direct) != LIB_STATUS_IO_ERROR || direct != LIB_NULL ||
        storage_medium_spawn(argv[0], "direct") != 1 ||
        storage_medium_spawn(argv[0], "readonly") != 0 ||
        lib_storage_medium_read_at(readonly, 1u, &value, 1u) != LIB_STATUS_OK ||
        value != (lib_u8)'Z' || lib_storage_medium_write_at(readonly, 1u,
            "Q", 1u) != LIB_STATUS_INVALID_STATE)) failed = 1;
    lib_storage_medium_destroy(&second_readonly);
    lib_storage_medium_destroy(&overlay);
    lib_storage_medium_destroy(&readonly);
    if (readonly != LIB_NULL || overlay != LIB_NULL || second_readonly != LIB_NULL) failed = 1;
    if (!failed && (lib_storage_medium_open(storage_medium_path,
            LIB_STORAGE_MEDIUM_OVERLAY, &overlay) != LIB_STATUS_OK ||
        lib_storage_medium_open(storage_medium_path, LIB_STORAGE_MEDIUM_DIRECT,
            &direct) != LIB_STATUS_IO_ERROR || direct != LIB_NULL ||
        lib_storage_medium_open(storage_medium_path, LIB_STORAGE_MEDIUM_READONLY,
            &readonly) != LIB_STATUS_OK ||
        lib_storage_medium_read_at(overlay, 4096u, &value, 1u) != LIB_STATUS_OK ||
        value != 0x33u ||
        lib_storage_medium_write_at(overlay, 1u, "Q", 1u) != LIB_STATUS_OK ||
        lib_storage_medium_read_at(overlay, 1u, &value, 1u) != LIB_STATUS_OK ||
        value != (lib_u8)'Q' ||
        lib_storage_medium_read_at(overlay, 0u, &value, 1u) != LIB_STATUS_OK ||
        value != 0x11u ||
        lib_storage_medium_read_at(readonly, 1u, &value, 1u) != LIB_STATUS_OK ||
        value != (lib_u8)'Z')) failed = 1;
    lib_storage_medium_destroy(&direct);
    lib_storage_medium_destroy(&overlay);
    lib_storage_medium_destroy(&readonly);
    if (direct != LIB_NULL || overlay != LIB_NULL) failed = 1;
    (void)remove(storage_medium_path);
    return failed;
}
