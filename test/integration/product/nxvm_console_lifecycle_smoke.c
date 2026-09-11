#ifndef _WIN32
#define _POSIX_C_SOURCE 200809L
#endif

#include "type.h"

#ifdef _WIN32
#include <io.h>
#define TEST_CONSOLE_DUP _dup
#define TEST_CONSOLE_DUP2 _dup2
#define TEST_CONSOLE_CLOSE _close
#define TEST_CONSOLE_FILENO _fileno
#else
#include <unistd.h>
#define TEST_CONSOLE_DUP dup
#define TEST_CONSOLE_DUP2 dup2
#define TEST_CONSOLE_CLOSE close
#define TEST_CONSOLE_FILENO fileno
#endif

#include "vm/session/session.h"
#include "vm/product/console.h"
#include "vm/session/catalog.h"

static C_INT session_choice(const C_CHAR *directory, const C_CHAR *file_name)
{
    vm_product_session_catalog *catalog = STD_NULL;
    STD_SIZE_T index;
    C_INT choice = 0;

    if (vm_product_session_catalog_create(directory, &catalog) != TYPE_STATUS_OK) return 0;
    for (index = 0u; index < vm_product_session_catalog_count(catalog); ++index) {
        vm_product_session_request request;

        if (vm_product_session_catalog_get_request(catalog, index, &request) == TYPE_STATUS_OK &&
            !STD_STRCMP(request.file_name, file_name)) {
            choice = (C_INT)index + 1;
            break;
        }
    }
    vm_product_session_catalog_destroy(catalog);
    return choice;
}

C_INT main(C_INT argc, C_CHAR **argv)
{
    vm_session *session = STD_NULL;
    vm_product_console_context *console_context = STD_NULL;
    STD_FILE *input = STD_NULL;
    C_CHAR commands[128];
    C_INT saved_stdin = -1;
    C_INT choice;
    C_INT result = 1;
    vm_session_speed speed = VM_SESSION_SPEED_STANDARD;

    if (argc != 2 || (choice = session_choice(argv[1], "ibm-5170-model-339-1200k.yaml")) == 0 ||
        (input = tmpfile()) == STD_NULL ||
        STD_SNPRINTF(commands, sizeof(commands), "%d\nspeed turbo\nhelp\ninfo\nexit\n",
            choice) < 0 || STD_FPUTS(commands, input) < 0 || fflush(input) != 0 ||
        STD_FSEEK(input, 0L, STD_SEEK_SET) != 0) {
        if (input != STD_NULL) STD_FCLOSE(input);
        return 1;
    }
    saved_stdin = TEST_CONSOLE_DUP(TEST_CONSOLE_FILENO(STD_STDIN));
    if (saved_stdin < 0 || TEST_CONSOLE_DUP2(TEST_CONSOLE_FILENO(input),
            TEST_CONSOLE_FILENO(STD_STDIN)) < 0) goto done;
    if (vm_session_create(&session) != TYPE_STATUS_OK ||
        vm_product_console_context_create(&console_context) != TYPE_STATUS_OK) goto done;
    vm_product_console_main(console_context, session, argv[1]);
    if (vm_session_get_speed(session, &speed) != TYPE_STATUS_OK ||
        speed != VM_SESSION_SPEED_TURBO) {
        goto done;
    }
    result = 0;
done:
    if (saved_stdin >= 0) {
        (C_VOID)TEST_CONSOLE_DUP2(saved_stdin, TEST_CONSOLE_FILENO(STD_STDIN));
        TEST_CONSOLE_CLOSE(saved_stdin);
    }
    STD_FCLOSE(input);
    vm_product_console_context_destroy(console_context);
    vm_session_destroy(session);
    if (result == 0) STD_PRINTF("M5:T515:S3:CONSOLE-YAML-LIFECYCLE:OK\n");
    return result;
}
