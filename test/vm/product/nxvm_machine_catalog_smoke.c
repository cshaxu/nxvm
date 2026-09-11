#include "type.h"

#include "vm/product/session_catalog.h"

static C_INT catalog_parse(C_CHAR *document, vm_product_session_request *out_request)
{
    return vm_product_session_request_parse("unit-root", "case.yaml", document,
        out_request) == TYPE_STATUS_OK;
}

C_INT main(C_VOID)
{
    static C_CHAR valid[] =
        "schema: nxvm-session\nprofile: default-pc-at\ncpu: 8086\nfpu: 8087\n"
        "memory_kib: 4294967296\n"
        "floppy_format: 720k\ndisplay: console\nfirmware:\n  bios:\n"
        "    - path: system.rom\n  video: null\n  cmos: seed.cmos\n"
        "  font: font.bin\nmedia:\n  floppy:\n    - path: first.img\n"
        "    - path: /owner/second.img\n  fixed_disk:\n"
        "    - path: first-disk.img\n    - path: C:/owner/second-disk.img\n";
    static C_CHAR invalid_memory[] =
        "schema: nxvm-session\nprofile: default-pc-at\nmemory_kib: -1\n"
        "display: console\nmedia:\n  floppy: []\n  fixed_disk: []\n";
    static C_CHAR duplicate_media[] =
        "schema: nxvm-session\nprofile: default-pc-at\ndisplay: console\n"
        "media:\n  floppy: []\n  floppy: []\n  fixed_disk: []\n";
    static C_CHAR invalid_schema[] =
        "schema: nxvm-session/v1\nprofile: default-pc-at\ndisplay: console\n"
        "media:\n  floppy: []\n  fixed_disk: []\n";
    static C_CHAR model40[] =
        "schema: nxvm-session\nprofile: compaq-deskpro-386-model-40\n"
        "display: console\nmedia:\n  floppy: []\n  fixed_disk: []\n"
        "firmware:\n  bios:\n    - path: even.rom\n    - path: odd.rom\n"
        "  video: null\n  cmos: seed.cmos\n  font: font.bin\n";
    static C_CHAR xt[] =
        "schema: nxvm-session\nprofile: ibm-5160-model-268\ndisplay: console\n"
        "media:\n  floppy: []\n  fixed_disk: []\nfirmware:\n  bios:\n"
        "    - path: xt.rom\n  video: null\n  cmos: null\n  font: font.bin\n";
    vm_product_session_request request;

    if (!catalog_parse(valid, &request)) {
        STD_PRINTF("catalog valid parse failed\n");
        return 1;
    }
    if (
        STD_STRCMP(request.profile, "default-pc-at") ||
        request.memory_bytes != (STD_SIZE_T)4294967296u * 1024u ||
        STD_STRCMP(request.cpu, "8086") || STD_STRCMP(request.fpu, "8087") ||
        STD_STRCMP(request.floppy_format, "720k") || request.bios_count != 1u ||
        request.floppy_count != 2u || request.fixed_disk_count != 2u ||
        STD_STRCMP(request.bios[0u], "unit-root/system.rom") ||
        STD_STRCMP(request.cmos, "unit-root/seed.cmos") ||
        STD_STRCMP(request.font, "unit-root/font.bin") ||
        STD_STRCMP(request.floppy[0u], "unit-root/first.img") ||
        STD_STRCMP(request.floppy[1u], "/owner/second.img") ||
        STD_STRCMP(request.fixed_disk[0u], "unit-root/first-disk.img") ||
        STD_STRCMP(request.fixed_disk[1u], "C:/owner/second-disk.img")) {
        STD_PRINTF("catalog valid result mismatch\n");
        return 1;
    }
    if (catalog_parse(invalid_memory, &request) || catalog_parse(duplicate_media, &request) ||
        catalog_parse(invalid_schema, &request)) {
        STD_PRINTF("catalog invalid parse accepted\n");
        return 1;
    }
    if (!catalog_parse(model40, &request) || request.bios_count != 2u ||
        !catalog_parse(xt, &request) || request.bios_count != 1u) {
        STD_PRINTF("catalog profile fixture failed\n");
        return 1;
    }
    STD_PRINTF("M5:T515:S3:SESSION-CATALOG-PARSE:OK\n");
    return 0;
}
