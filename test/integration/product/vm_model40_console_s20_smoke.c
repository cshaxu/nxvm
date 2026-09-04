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

#include "vm/composition/session/console_machine_adapter.h"
#include "vm/composition/session/provider.h"
#include "vm/product/console.h"
#include "vm/product/session_catalog.h"

static C_INT model40_choice(const C_CHAR *directory)
{
    vm_product_session_catalog *catalog = STD_NULL;
    STD_SIZE_T index;
    C_INT choice = 0;

    if (vm_product_session_catalog_create(directory, &catalog) != TYPE_STATUS_OK) return 0;
    for (index = 0u; index < vm_product_session_catalog_count(catalog); ++index) {
        vm_product_session_request request;

        if (vm_product_session_catalog_get_request(catalog, index, &request) == TYPE_STATUS_OK &&
            !STD_STRCMP(request.file_name, "compaq-deskpro-386-model-40-1200k.yaml")) {
            choice = (C_INT)index + 1;
            break;
        }
    }
    vm_product_session_catalog_destroy(catalog);
    return choice;
}

C_INT main(C_INT argc, C_CHAR **argv)
{
    core_product_session_provider provider;
    core_product_session_manager *manager = STD_NULL;
    vm_session_machine_provider machine_provider;
    vm_product_console_context *console = STD_NULL;
    core_product_session_snapshot snapshot;
    STD_FILE *input = STD_NULL;
    C_CHAR commands[48];
    C_INT stdin_copy = -1;
    C_INT choice;
    C_INT failed = 0;

    if (argc != 2 || (choice = model40_choice(argv[1])) == 0 ||
        (input = tmpfile()) == STD_NULL ||
        STD_SNPRINTF(commands, sizeof(commands), "%d\nsession list\nexit\n", choice) < 0 ||
        STD_FPUTS(commands, input) < 0 || fflush(input) != 0 ||
        STD_FSEEK(input, 0L, STD_SEEK_SET) != 0) failed = 1;
    if (!failed) stdin_copy = TEST_DUP(TEST_FILENO(STD_STDIN));
    if (!failed && (stdin_copy < 0 || TEST_DUP2(TEST_FILENO(input),
            TEST_FILENO(STD_STDIN)) < 0)) failed = 1;
    if (!failed) {
        vm_session_provider_initialize(&provider);
        failed = core_product_session_manager_create(&provider, &manager) != TYPE_STATUS_OK;
    }
    if (!failed) {
        vm_composition_console_machine_provider_initialize(&machine_provider, manager);
        failed = vm_product_console_context_create(&console) != TYPE_STATUS_OK;
    }
    if (!failed) {
        vm_product_console_main(console, &machine_provider, manager, argv[1]);
        failed = core_product_session_manager_list(manager, &snapshot, 1u,
            &(STD_SIZE_T){0u}) != TYPE_STATUS_OK || !snapshot.selected ||
            STD_STRCMP(snapshot.details,
                "profile=compaq-deskpro-386-model-40 cpu=80386 fpu=none");
    }
    if (stdin_copy >= 0) {
        (C_VOID)TEST_DUP2(stdin_copy, TEST_FILENO(STD_STDIN));
        TEST_CLOSE(stdin_copy);
    }
    if (input != STD_NULL) STD_FCLOSE(input);
    vm_product_console_context_destroy(console);
    if (manager != STD_NULL) core_product_session_manager_destroy(manager);
    if (!failed) STD_PRINTF("M5:T515:S3:MODEL40-CONSOLE-YAML:OK\n");
    return failed;
}
