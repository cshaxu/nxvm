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

static C_INT verify_repository_templates(C_VOID)
{
    vm_product_session_catalog *catalog = STD_NULL;
    STD_SIZE_T index;
    C_INT default_cpu_mask = 0;
    C_INT model40 = 0;
    C_INT model339 = 0;
    C_INT xt = 0;
    C_INT valid = 1;

    if (vm_product_session_catalog_create(PROJECT_SOURCE_DIR "/assets/sessions",
            &catalog) != TYPE_STATUS_OK || catalog == STD_NULL ||
        vm_product_session_catalog_count(catalog) != 7u ||
        vm_product_session_catalog_rejected(catalog) != 0u) valid = 0;
    for (index = 0u; valid && index < vm_product_session_catalog_count(catalog); ++index) {
        vm_product_session_request request;

        if (vm_product_session_catalog_get_request(catalog, index, &request) != TYPE_STATUS_OK) {
            valid = 0;
        } else if (!STD_STRCMP(request.profile, "default-pc-at")) {
            if (!STD_STRCMP(request.cpu, "8086")) default_cpu_mask |= 1;
            else if (!STD_STRCMP(request.cpu, "80186")) default_cpu_mask |= 2;
            else if (!STD_STRCMP(request.cpu, "80286")) default_cpu_mask |= 4;
            else if (!STD_STRCMP(request.cpu, "80386")) default_cpu_mask |= 8;
            else valid = 0;
            if (request.floppy_count != 1u || request.fixed_disk_count != 1u ||
                request.cmos[0] == '\0' || request.font[0] == '\0') valid = 0;
        } else if (!STD_STRCMP(request.profile, "compaq-deskpro-386-model-40")) {
            ++model40;
            if (request.bios_count != 2u || request.floppy_count != 1u ||
                request.fixed_disk_count != 1u || request.cmos[0] == '\0' ||
                request.font[0] == '\0') valid = 0;
        } else if (!STD_STRCMP(request.profile, "ibm-5170-model-339")) {
            ++model339;
            if (request.bios_count != 2u || request.floppy_count != 1u ||
                request.fixed_disk_count != 0u ||
                request.font[0] == '\0') valid = 0;
        } else if (!STD_STRCMP(request.profile, "ibm-5160-model-268")) {
            ++xt;
            if (request.bios_count != 1u || request.floppy_count != 1u ||
                request.fixed_disk_count != 0u || request.cmos[0] != '\0' ||
                request.font[0] == '\0') valid = 0;
        } else {
            valid = 0;
        }
    }
    vm_product_session_catalog_destroy(catalog);
    return valid && default_cpu_mask == 15 && model40 == 1 && model339 == 1 && xt == 1;
}

C_INT main(C_VOID)
{
    const C_CHAR *directory = "t381-session-catalog";
    const C_CHAR *valid = "schema: nxvm-session\nprofile: default-pc-at\nmemory_kib: 1\nfloppy_format: 720k\ndisplay: console\nboot: rom\nfirmware:\n  bios: []\n  video: null\n  cmos: seed.cmos\n  font: font.bin\nmedia:\n  floppy: []\n  fixed_disk: []\n";
    vm_product_session_catalog *catalog = STD_NULL;
    vm_product_session_request first;
    vm_product_session_request second;
    vm_product_session_request third;
    vm_product_session_request fourth;
    vm_product_session_request fifth;
    vm_product_session_request sixth;
    vm_product_session_request seventh;
    C_INT result = 1;

    (C_VOID)TEST_RMDIR(directory);
    if (TEST_MKDIR(directory) != 0 || !write_file("t381-session-catalog/a.yaml", valid) ||
        !write_file("t381-session-catalog/b.yaml", "schema: nxvm-session\nprofile: default-pc-at\nmemory_kib: -1\ndisplay: console\nboot: rom\nmedia:\n  floppy: []\n  fixed_disk: []\n") ||
        !write_file("t381-session-catalog/c.yaml", "schema: nxvm-session\nprofile: default-pc-at\nmemory_kib: 1x\ndisplay: console\nboot: rom\nmedia:\n  floppy: []\n  fixed_disk: []\n") ||
        !write_file("t381-session-catalog/d.yaml", "schema: nxvm-session\nprofile: default-pc-at\nmemory_kib: 18446744073709551615\ndisplay: console\nboot: rom\nmedia:\n  floppy: []\n  fixed_disk: []\n") ||
        !write_file("t381-session-catalog/e.yaml", "schema: nxvm-session\nprofile: default-pc-at\nmemory_kib: \ndisplay: console\nboot: rom\nmedia:\n  floppy: []\n  fixed_disk: []\n") ||
        !write_file("t381-session-catalog/f.yaml", "schema: nxvm-session\nprofile: default-pc-at\nmemory_kib: 0\ndisplay: console\nboot: rom\nmedia:\n  floppy: []\n  fixed_disk: []\n") ||
        !write_file("t381-session-catalog/g.yaml", "schema: nxvm-session\nprofile: default-pc-at\nmemory_kib: 18014398509481983\ndisplay: console\nboot: rom\nmedia:\n  floppy: []\n  fixed_disk: []\n")) goto done;
    if (!write_file("t381-session-catalog/j.yaml", "schema: nxvm-session\nprofile: default-pc-at\ncpu: 80486\ndisplay: console\nboot: rom\nmedia:\n  floppy: []\n  fixed_disk: []\n") ||
        !write_file("t381-session-catalog/k.yaml", "schema: nxvm-session\nprofile: default-pc-at\nfpu: 8087\ndisplay: console\nboot: rom\nmedia:\n  floppy: []\n  fixed_disk: []\n") ||
        !write_file("t381-session-catalog/l.yaml", "schema: nxvm-session/v1\nprofile: default-pc-at\ndisplay: console\nboot: rom\nmedia:\n  floppy: []\n  fixed_disk: []\n") ||
        !write_file("t381-session-catalog/m.yaml", "schema: nxvm-session\nmachine:\n  profile: default-pc-at\ndisplay: console\nboot: rom\nmedia:\n  floppy: []\n  fixed_disk: []\n") ||
        !write_file("t381-session-catalog/n.yaml", "schema: nxvm-session\nprofile: default-pc-at\nmemory_kib: 1\nmemory_kib: 2\ndisplay: console\nboot: rom\nmedia:\n  floppy: []\n  fixed_disk: []\n") ||
        !write_file("t381-session-catalog/o.yaml", "schema: nxvm-session\nprofile: default-pc-at\ndisplay: console\nboot: rom\nmedia:\n  floppy: []\n  floppy: []\n  fixed_disk: []\n") ||
        !write_file("t381-session-catalog/p.yaml", "schema: nxvm-session\nprofile: default-pc-at\nunknown: value\ndisplay: console\nboot: rom\nmedia:\n  floppy: []\n  fixed_disk: []\n")) goto done;
    if (!write_file("t381-session-catalog/h.yaml", "schema: nxvm-session\nprofile: compaq-deskpro-386-model-40\ndisplay: console\nboot: rom\nmedia:\n  floppy: []\n  fixed_disk: []\nfirmware:\n  bios:\n    - path: even.bin\n    - path: odd.bin\n  video: null\n  cmos: null\n")) goto done;
    if (!write_file("t381-session-catalog/i.yaml", "schema: nxvm-session\nprofile: compaq-deskpro-386-model-40\ndisplay: console\nboot: rom\nmedia:\n  floppy: []\n  fixed_disk: []\nfirmware:\n  provenance: project-owned synthetic test input\n  rom_even:\n    slot: system-rom-even\n    path: even.bin\n    bytes: 16384\n    sha256: invalid\n    map: read-only\n  rom_odd:\n    slot: system-rom-odd\n    path: odd.bin\n    bytes: 16384\n    sha256: 111ce3c2a38d83a2e4706bde4abddd509d7f8248116c6832b06745bdc349e09f\n    map: read-only\n")) goto done;
    if (!write_file("t381-session-catalog/q.yaml", "schema: nxvm-session\nprofile: ibm-5160-model-268\ndisplay: console\nboot: rom\nmedia:\n  floppy: []\n  fixed_disk: []\nfirmware:\n  bios:\n    - path: xt.bin\n  video: null\n  cmos: null\n") ||
        !write_file("t381-session-catalog/r.yaml", "schema: nxvm-session\nprofile: default-pc-at\ndisplay: console\nboot: rom\nfirmware:\n  bios: []\n  video: /owner/video.rom\n  cmos: C:/owner/seed.cmos\nmedia:\n  floppy:\n    - path: first.img\n    - path: /owner/second.img\n  fixed_disk:\n    - path: first-disk.img\n    - path: C:/owner/second-disk.img\n")) goto done;
    if (vm_product_session_catalog_create(directory, &catalog) != TYPE_STATUS_OK) goto done;
    if (vm_product_session_catalog_get_request(catalog, 0u, &first) != TYPE_STATUS_OK ||
        vm_product_session_catalog_get_request(catalog, 1u, &second) != TYPE_STATUS_OK ||
        vm_product_session_catalog_get_request(catalog, 2u, &third) != TYPE_STATUS_OK ||
        vm_product_session_catalog_get_request(catalog, 3u, &fourth) != TYPE_STATUS_OK ||
        vm_product_session_catalog_get_request(catalog, 4u, &fifth) != TYPE_STATUS_OK ||
        vm_product_session_catalog_get_request(catalog, 5u, &sixth) != TYPE_STATUS_OK ||
        vm_product_session_catalog_get_request(catalog, 6u, &seventh) != TYPE_STATUS_OK) goto done;
    if (vm_product_session_catalog_count(catalog) == 7u &&
        vm_product_session_catalog_rejected(catalog) == 11u &&
        !STD_STRCMP(first.file_name, "a.yaml") &&
        !STD_STRCMP(first.profile, "default-pc-at") &&
        first.memory_bytes == 1024u &&
        !STD_STRCMP(first.floppy_format, "720k") &&
        !STD_STRCMP(first.cmos, "t381-session-catalog/seed.cmos") &&
        !STD_STRCMP(first.font, "t381-session-catalog/font.bin") &&
        !STD_STRCMP(second.file_name, "g.yaml") &&
        second.memory_bytes ==
            (~(STD_SIZE_T)0u & ~((STD_SIZE_T)1023u)) &&
        !STD_STRCMP(third.profile, "compaq-deskpro-386-model-40") &&
        third.bios_count == 2u &&
        !STD_STRCMP(fourth.file_name, "j.yaml") &&
        !STD_STRCMP(fourth.cpu, "80486") &&
        !STD_STRCMP(fifth.file_name, "k.yaml") &&
        !STD_STRCMP(fifth.fpu, "8087") &&
        !STD_STRCMP(sixth.profile, "ibm-5160-model-268") &&
        sixth.bios_count == 1u &&
        !STD_STRCMP(seventh.file_name, "r.yaml") &&
        seventh.floppy_count == 2u && seventh.fixed_disk_count == 2u &&
        !STD_STRCMP(seventh.floppy[0u], "t381-session-catalog/first.img") &&
        !STD_STRCMP(seventh.floppy[1u], "/owner/second.img") &&
        !STD_STRCMP(seventh.fixed_disk[0u], "t381-session-catalog/first-disk.img") &&
        !STD_STRCMP(seventh.fixed_disk[1u], "C:/owner/second-disk.img") &&
        !STD_STRCMP(seventh.video, "/owner/video.rom") &&
        !STD_STRCMP(seventh.cmos, "C:/owner/seed.cmos") &&
        verify_repository_templates()) result = 0;
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
    (C_VOID)STD_REMOVE("t381-session-catalog/r.yaml");
    (C_VOID)TEST_RMDIR(directory);
    if (result == 0) {
        STD_PRINTF("M5:T381:S1:SESSION-CATALOG:OK\n");
        STD_PRINTF("M5:T482:S2:ROOT-PARSER:OK\n");
        STD_PRINTF("M5:T482:S2:REJECTIONS:OK\n");
        STD_PRINTF("M5:T482:S2:IMMUTABLE-REQUEST:OK\n");
    }
    return result;
}
