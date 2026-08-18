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
    const C_CHAR *valid = "schema: nxvm-session/v1\nmachine:\n  profile: default-pc-at\n  memory_kib: 1\n  display: console\n  boot: rom\nmedia:\n  floppy: null\n  hard_disk: null\n";
    vm_product_session_catalog catalog;
    C_INT result = 1;

    (C_VOID)TEST_RMDIR(directory);
    if (TEST_MKDIR(directory) != 0 || !write_file("t381-session-catalog/a.yaml", valid) ||
        !write_file("t381-session-catalog/b.yaml", "schema: nxvm-session/v1\nmachine:\n  profile: default-pc-at\n  memory_kib: -1\n  display: console\n  boot: rom\nmedia:\n  floppy: null\n  hard_disk: null\n") ||
        !write_file("t381-session-catalog/c.yaml", "schema: nxvm-session/v1\nmachine:\n  profile: default-pc-at\n  memory_kib: 1x\n  display: console\n  boot: rom\nmedia:\n  floppy: null\n  hard_disk: null\n") ||
        !write_file("t381-session-catalog/d.yaml", "schema: nxvm-session/v1\nmachine:\n  profile: default-pc-at\n  memory_kib: 18446744073709551615\n  display: console\n  boot: rom\nmedia:\n  floppy: null\n  hard_disk: null\n") ||
        !write_file("t381-session-catalog/e.yaml", "schema: nxvm-session/v1\nmachine:\n  profile: default-pc-at\n  memory_kib: \n  display: console\n  boot: rom\nmedia:\n  floppy: null\n  hard_disk: null\n") ||
        !write_file("t381-session-catalog/f.yaml", "schema: nxvm-session/v1\nmachine:\n  profile: default-pc-at\n  memory_kib: 0\n  display: console\n  boot: rom\nmedia:\n  floppy: null\n  hard_disk: null\n") ||
        !write_file("t381-session-catalog/g.yaml", "schema: nxvm-session/v1\nmachine:\n  profile: default-pc-at\n  memory_kib: 18014398509481983\n  display: console\n  boot: rom\nmedia:\n  floppy: null\n  hard_disk: null\n")) goto done;
    if (!write_file("t381-session-catalog/j.yaml", "schema: nxvm-session/v1\nmachine:\n  profile: default-pc-at\n  cpu: 80486\n  display: console\n  boot: rom\nmedia:\n  floppy: null\n  hard_disk: null\n") ||
        !write_file("t381-session-catalog/k.yaml", "schema: nxvm-session/v1\nmachine:\n  profile: default-pc-at\n  fpu: 8087\n  display: console\n  boot: rom\nmedia:\n  floppy: null\n  hard_disk: null\n")) goto done;
    if (!write_file("t381-session-catalog/h.yaml", "schema: nxvm-session/v1\nmachine:\n  profile: compaq-deskpro-386-model-40\n  display: console\n  boot: rom\nmedia:\n  floppy: null\n  hard_disk: null\nfirmware:\n  provenance: project-owned synthetic test input\n  rom_even:\n    slot: system-rom-even\n    path: even.bin\n    bytes: 16384\n    sha256: 4fe7b59af6de3b665b67788cc2f99892ab827efae3a467342b3bb4e3bc8e5bfe\n    map: read-only\n  rom_odd:\n    slot: system-rom-odd\n    path: odd.bin\n    bytes: 16384\n    sha256: 111ce3c2a38d83a2e4706bde4abddd509d7f8248116c6832b06745bdc349e09f\n    map: read-only\n")) goto done;    if (!write_file("t381-session-catalog/i.yaml", "schema: nxvm-session/v1\nmachine:\n  profile: compaq-deskpro-386-model-40\n  display: console\n  boot: rom\nmedia:\n  floppy: null\n  hard_disk: null\nfirmware:\n  provenance: project-owned synthetic test input\n  rom_even:\n    slot: system-rom-even\n    path: even.bin\n    bytes: 16384\n    sha256: invalid\n    map: read-only\n  rom_odd:\n    slot: system-rom-odd\n    path: odd.bin\n    bytes: 16384\n    sha256: 111ce3c2a38d83a2e4706bde4abddd509d7f8248116c6832b06745bdc349e09f\n    map: read-only\n")) goto done;    vm_product_session_catalog_initialize(&catalog, directory);
    if (catalog.count == 3u && catalog.rejected == 8u &&
        !STD_STRCMP(catalog.entries[0].file_name, "a.yaml") &&
        !STD_STRCMP(catalog.entries[0].profile, "default-pc-at") &&
        catalog.entries[0].memory_bytes == 1024u &&
        !STD_STRCMP(catalog.entries[1].file_name, "g.yaml") &&
        catalog.entries[1].memory_bytes ==
            (~(STD_SIZE_T)0u & ~((STD_SIZE_T)1023u)) &&
        !STD_STRCMP(catalog.entries[2].profile, "compaq-deskpro-386-model-40") &&
        !STD_STRCMP(catalog.entries[2].model40_provenance, "project-owned synthetic test input")) result = 0;
done:
    (C_VOID)STD_REMOVE("t381-session-catalog/a.yaml");
    (C_VOID)STD_REMOVE("t381-session-catalog/b.yaml");
    (C_VOID)STD_REMOVE("t381-session-catalog/c.yaml");
    (C_VOID)STD_REMOVE("t381-session-catalog/d.yaml");
    (C_VOID)STD_REMOVE("t381-session-catalog/e.yaml");
    (C_VOID)STD_REMOVE("t381-session-catalog/f.yaml");
    (C_VOID)STD_REMOVE("t381-session-catalog/g.yaml");
    (C_VOID)STD_REMOVE("t381-session-catalog/h.yaml");
    (C_VOID)STD_REMOVE("t381-session-catalog/i.yaml");
    (C_VOID)STD_REMOVE("t381-session-catalog/j.yaml");
    (C_VOID)STD_REMOVE("t381-session-catalog/k.yaml");
    (C_VOID)TEST_RMDIR(directory);
    if (result == 0) STD_PRINTF("M5:T381:S1:SESSION-CATALOG:OK\n");
    return result;
}
