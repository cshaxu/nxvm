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
    const C_CHAR *valid = "schema: nxvm-session\nprofile: default-pc-at\nmemory_kib: 1\nfloppy_format: 720k\ndisplay: console\nboot: rom\nmedia:\n  floppy: null\n  hard_disk: null\n";
    vm_product_session_catalog *catalog = STD_NULL;
    vm_product_session_request first;
    vm_product_session_request second;
    vm_product_session_request third;
    vm_product_session_request fourth;
    vm_product_session_request fifth;
    vm_product_session_request sixth;
    C_INT result = 1;

    (C_VOID)TEST_RMDIR(directory);
    if (TEST_MKDIR(directory) != 0 || !write_file("t381-session-catalog/a.yaml", valid) ||
        !write_file("t381-session-catalog/b.yaml", "schema: nxvm-session\nprofile: default-pc-at\nmemory_kib: -1\ndisplay: console\nboot: rom\nmedia:\n  floppy: null\n  hard_disk: null\n") ||
        !write_file("t381-session-catalog/c.yaml", "schema: nxvm-session\nprofile: default-pc-at\nmemory_kib: 1x\ndisplay: console\nboot: rom\nmedia:\n  floppy: null\n  hard_disk: null\n") ||
        !write_file("t381-session-catalog/d.yaml", "schema: nxvm-session\nprofile: default-pc-at\nmemory_kib: 18446744073709551615\ndisplay: console\nboot: rom\nmedia:\n  floppy: null\n  hard_disk: null\n") ||
        !write_file("t381-session-catalog/e.yaml", "schema: nxvm-session\nprofile: default-pc-at\nmemory_kib: \ndisplay: console\nboot: rom\nmedia:\n  floppy: null\n  hard_disk: null\n") ||
        !write_file("t381-session-catalog/f.yaml", "schema: nxvm-session\nprofile: default-pc-at\nmemory_kib: 0\ndisplay: console\nboot: rom\nmedia:\n  floppy: null\n  hard_disk: null\n") ||
        !write_file("t381-session-catalog/g.yaml", "schema: nxvm-session\nprofile: default-pc-at\nmemory_kib: 18014398509481983\ndisplay: console\nboot: rom\nmedia:\n  floppy: null\n  hard_disk: null\n")) goto done;
    if (!write_file("t381-session-catalog/j.yaml", "schema: nxvm-session\nprofile: default-pc-at\ncpu: 80486\ndisplay: console\nboot: rom\nmedia:\n  floppy: null\n  hard_disk: null\n") ||
        !write_file("t381-session-catalog/k.yaml", "schema: nxvm-session\nprofile: default-pc-at\nfpu: 8087\ndisplay: console\nboot: rom\nmedia:\n  floppy: null\n  hard_disk: null\n") ||
        !write_file("t381-session-catalog/l.yaml", "schema: nxvm-session/v1\nprofile: default-pc-at\ndisplay: console\nboot: rom\nmedia:\n  floppy: null\n  hard_disk: null\n") ||
        !write_file("t381-session-catalog/m.yaml", "schema: nxvm-session\nmachine:\n  profile: default-pc-at\ndisplay: console\nboot: rom\nmedia:\n  floppy: null\n  hard_disk: null\n") ||
        !write_file("t381-session-catalog/n.yaml", "schema: nxvm-session\nprofile: default-pc-at\nmemory_kib: 1\nmemory_kib: 2\ndisplay: console\nboot: rom\nmedia:\n  floppy: null\n  hard_disk: null\n") ||
        !write_file("t381-session-catalog/o.yaml", "schema: nxvm-session\nprofile: default-pc-at\ndisplay: console\nboot: rom\nmedia:\n  floppy: null\n  floppy: null\n  hard_disk: null\n") ||
        !write_file("t381-session-catalog/p.yaml", "schema: nxvm-session\nprofile: default-pc-at\nunknown: value\ndisplay: console\nboot: rom\nmedia:\n  floppy: null\n  hard_disk: null\n")) goto done;
    if (!write_file("t381-session-catalog/h.yaml", "schema: nxvm-session\nprofile: compaq-deskpro-386-model-40\ndisplay: console\nboot: rom\nmedia:\n  floppy: null\n  hard_disk: null\nfirmware:\n  provenance: project-owned synthetic test input\n  rom_even:\n    slot: system-rom-even\n    path: even.bin\n    bytes: 16384\n    sha256: 4fe7b59af6de3b665b67788cc2f99892ab827efae3a467342b3bb4e3bc8e5bfe\n    map: read-only\n  rom_odd:\n    slot: system-rom-odd\n    path: odd.bin\n    bytes: 16384\n    sha256: 111ce3c2a38d83a2e4706bde4abddd509d7f8248116c6832b06745bdc349e09f\n    map: read-only\n")) goto done;
    if (!write_file("t381-session-catalog/i.yaml", "schema: nxvm-session\nprofile: compaq-deskpro-386-model-40\ndisplay: console\nboot: rom\nmedia:\n  floppy: null\n  hard_disk: null\nfirmware:\n  provenance: project-owned synthetic test input\n  rom_even:\n    slot: system-rom-even\n    path: even.bin\n    bytes: 16384\n    sha256: invalid\n    map: read-only\n  rom_odd:\n    slot: system-rom-odd\n    path: odd.bin\n    bytes: 16384\n    sha256: 111ce3c2a38d83a2e4706bde4abddd509d7f8248116c6832b06745bdc349e09f\n    map: read-only\n")) goto done;
    if (!write_file("t381-session-catalog/q.yaml", "schema: nxvm-session\nprofile: ibm-5160-model-268\ndisplay: console\nboot: rom\nmedia:\n  floppy: null\n  hard_disk: null\nfirmware:\n  provenance: project-owned synthetic test input\n  system_rom:\n    slot: xt-base-system\n    path: xt.bin\n    bytes: 65536\n    sha256: de2f256064a0af797747c2b97505dc0b9f3df0de4f489eac731c23ae9ca9cc31\n    map: read-only\n")) goto done;
    if (vm_product_session_catalog_create(directory, &catalog) != TYPE_STATUS_OK) goto done;
    if (vm_product_session_catalog_get_request(catalog, 0u, &first) != TYPE_STATUS_OK ||
        vm_product_session_catalog_get_request(catalog, 1u, &second) != TYPE_STATUS_OK ||
        vm_product_session_catalog_get_request(catalog, 2u, &third) != TYPE_STATUS_OK ||
        vm_product_session_catalog_get_request(catalog, 3u, &fourth) != TYPE_STATUS_OK ||
        vm_product_session_catalog_get_request(catalog, 4u, &fifth) != TYPE_STATUS_OK ||
        vm_product_session_catalog_get_request(catalog, 5u, &sixth) != TYPE_STATUS_OK) goto done;
    if (vm_product_session_catalog_count(catalog) == 6u &&
        vm_product_session_catalog_rejected(catalog) == 11u &&
        !STD_STRCMP(first.file_name, "a.yaml") &&
        !STD_STRCMP(first.profile, "default-pc-at") &&
        first.memory_bytes == 1024u &&
        !STD_STRCMP(first.floppy_format, "720k") &&
        !STD_STRCMP(second.file_name, "g.yaml") &&
        second.memory_bytes ==
            (~(STD_SIZE_T)0u & ~((STD_SIZE_T)1023u)) &&
        !STD_STRCMP(third.profile, "compaq-deskpro-386-model-40") &&
        !STD_STRCMP(third.model40_provenance, "project-owned synthetic test input") &&
        !STD_STRCMP(fourth.file_name, "j.yaml") &&
        !STD_STRCMP(fourth.cpu, "80486") &&
        !STD_STRCMP(fifth.file_name, "k.yaml") &&
        !STD_STRCMP(fifth.fpu, "8087") &&
        !STD_STRCMP(sixth.profile, "ibm-5160-model-268") &&
        !STD_STRCMP(sixth.xt_provenance, "project-owned synthetic test input")) result = 0;
done:
    vm_product_session_catalog_destroy(catalog);
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
    (C_VOID)STD_REMOVE("t381-session-catalog/l.yaml");
    (C_VOID)STD_REMOVE("t381-session-catalog/m.yaml");
    (C_VOID)STD_REMOVE("t381-session-catalog/n.yaml");
    (C_VOID)STD_REMOVE("t381-session-catalog/o.yaml");
    (C_VOID)STD_REMOVE("t381-session-catalog/p.yaml");
    (C_VOID)STD_REMOVE("t381-session-catalog/q.yaml");
    (C_VOID)TEST_RMDIR(directory);
    if (result == 0) {
        STD_PRINTF("M5:T381:S1:SESSION-CATALOG:OK\n");
        STD_PRINTF("M5:T482:S2:ROOT-PARSER:OK\n");
        STD_PRINTF("M5:T482:S2:REJECTIONS:OK\n");
        STD_PRINTF("M5:T482:S2:IMMUTABLE-REQUEST:OK\n");
    }
    return result;
}
