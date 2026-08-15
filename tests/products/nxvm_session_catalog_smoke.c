#include "type.h"

#ifdef _WIN32
#include <direct.h>
#define TEST_MKDIR(path) _mkdir(path)
#define TEST_RMDIR(path) _rmdir(path)
#else
#include <sys/stat.h>
#include <unistd.h>
#define TEST_MKDIR(path) mkdir(path, 0700)
#define TEST_RMDIR(path) rmdir(path)
#endif

#include "vm/product/session_catalog.h"

static C_INT write_file(const C_CHAR *path, const C_CHAR *text)
{
    STD_FILE *file = STD_FOPEN(path, "wb");
    if (file == STD_NULL) return 0;
    return STD_FPUTS(text, file) >= 0 && STD_FCLOSE(file) == 0;
}

C_INT main(C_VOID)
{
    const C_CHAR *directory = "t381-session-catalog";
    const C_CHAR *valid = "schema: nxvm-session/v1\nmachine:\n  profile: default-pc-at\n  display: console\n  boot: rom\nmedia:\n  floppy: null\n  hard_disk: null\n";
    const C_CHAR *invalid = "schema: nxvm-session/v1\nmachine:\n  profile: ibm-5170-model-339\n  memory_kib: 1024\n  display: console\n  boot: rom\nmedia:\n  floppy: null\n  hard_disk: null\n";
    vm_product_session_catalog catalog;
    C_INT result = 1;

    (C_VOID)TEST_RMDIR(directory);
    if (TEST_MKDIR(directory) != 0 || !write_file("t381-session-catalog/a.yaml", valid) ||
        !write_file("t381-session-catalog/z.yml", invalid)) goto done;
    vm_product_session_catalog_initialize(&catalog, directory);
    if (catalog.count == 1u && catalog.rejected == 1u &&
        !STD_STRCMP(catalog.entries[0].file_name, "a.yaml") &&
        !STD_STRCMP(catalog.entries[0].profile, "default-pc-at")) result = 0;
done:
    (C_VOID)STD_REMOVE("t381-session-catalog/a.yaml");
    (C_VOID)STD_REMOVE("t381-session-catalog/z.yml");
    (C_VOID)TEST_RMDIR(directory);
    if (result == 0) STD_PRINTF("M5:T381:S1:SESSION-CATALOG:OK\n");
    return result;
}
