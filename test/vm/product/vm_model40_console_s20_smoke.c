#ifndef _WIN32
#define _POSIX_C_SOURCE 200809L
#endif

#include "type.h"

#ifdef _WIN32
#include <io.h>
#define TEST_DUP _dup
#define TEST_DUP2 _dup2
#define TEST_CLOSE _close
#define TEST_FILENO _fileno
#else
#include <unistd.h>
#define TEST_DUP dup
#define TEST_DUP2 dup2
#define TEST_CLOSE close
#define TEST_FILENO fileno
#endif

#include "vm/composition/session/provider.h"
#include "vm/composition/session/console_machine_adapter.h"
#include "vm/product/console.h"

static C_INT write_chip(const C_CHAR *path, type_unsigned_8 value)
{
    type_unsigned_8 bytes[16384];
    STD_FILE *file;
    STD_MEMSET(bytes, value, sizeof(bytes));
    file = STD_FOPEN(path, "wb");
    return file != STD_NULL && STD_FWRITE(bytes, 1u, sizeof(bytes), file) == sizeof(bytes) &&
        STD_FCLOSE(file) == 0;
}

C_INT main(C_VOID)
{
    static const C_CHAR yaml[] = "schema: nxvm-session\nprofile: compaq-deskpro-386-model-40\ndisplay: console\nboot: rom\nmedia:\n  floppy: null\n  hard_disk: null\nfirmware:\n  provenance: project-owned synthetic test input\n  rom_even:\n    slot: system-rom-even\n    path: t386-s20-console-even.bin\n    bytes: 16384\n    sha256: 4fe7b59af6de3b665b67788cc2f99892ab827efae3a467342b3bb4e3bc8e5bfe\n    map: read-only\n  rom_odd:\n    slot: system-rom-odd\n    path: t386-s20-console-odd.bin\n    bytes: 16384\n    sha256: 111ce3c2a38d83a2e4706bde4abddd509d7f8248116c6832b06745bdc349e09f\n    map: read-only\n";
    core_product_session_provider provider;
    core_product_session_manager *manager = STD_NULL;
    vm_session_machine_provider machine_provider;
    vm_product_console_context *console = STD_NULL;
    core_product_session_snapshot snapshot;
    STD_FILE *profile = STD_NULL;
    STD_FILE *input = STD_NULL;
    C_INT stdin_copy = -1;
    C_INT failed = 0;

    profile = STD_FOPEN("t386-s20-console.yaml", "wb");
    input = tmpfile();
    if (!write_chip("t386-s20-console-even.bin", 0u) ||
        !write_chip("t386-s20-console-odd.bin", 1u) || profile == STD_NULL || input == STD_NULL ||
        STD_FPUTS(yaml, profile) < 0 || STD_FCLOSE(profile) != 0 ||
        STD_FPUTS("1\nsession list\nexit\n", input) < 0 || fflush(input) != 0 ||
        STD_FSEEK(input, 0L, STD_SEEK_SET) != 0) failed = 1;
    profile = STD_NULL;
    if (!failed) stdin_copy = TEST_DUP(TEST_FILENO(STD_STDIN));
    if (stdin_copy < 0 || TEST_DUP2(TEST_FILENO(input), TEST_FILENO(STD_STDIN)) < 0) failed = 1;
    if (!failed) {
        vm_session_provider_initialize(&provider);
        failed = core_product_session_manager_create(&provider, &manager) != TYPE_STATUS_OK;
    }
    if (!failed) {
        vm_composition_console_machine_provider_initialize(&machine_provider, manager);
        failed = vm_product_console_context_create(&console) != TYPE_STATUS_OK;
    }
    if (!failed) {
        vm_product_console_main(console, &machine_provider, manager, ".");
        failed = core_product_session_manager_list(manager, &snapshot, 1u, &(STD_SIZE_T){0u}) != TYPE_STATUS_OK ||
            !snapshot.selected || STD_STRCMP(snapshot.details,
                "profile=compaq-deskpro-386-model-40 cpu=80386 fpu=none");
    }
    if (stdin_copy >= 0) { TEST_DUP2(stdin_copy, TEST_FILENO(STD_STDIN)); TEST_CLOSE(stdin_copy); }
    if (input != STD_NULL) STD_FCLOSE(input);
    if (profile != STD_NULL) STD_FCLOSE(profile);
    vm_product_console_context_destroy(console);
    if (manager != STD_NULL) core_product_session_manager_destroy(manager);
    (C_VOID)STD_REMOVE("t386-s20-console.yaml");
    (C_VOID)STD_REMOVE("t386-s20-console-even.bin");
    (C_VOID)STD_REMOVE("t386-s20-console-odd.bin");
    if (!failed) STD_PRINTF("M5:T386:S20:MODEL40-CONSOLE-PRODUCTION-ROUTE:OK\n");
    return failed;
}
