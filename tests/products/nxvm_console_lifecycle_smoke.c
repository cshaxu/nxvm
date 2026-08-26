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


#include "vm/composition/session/provider.h"
#include "vm/composition/session/console_machine_adapter.h"
#include "vm/composition/session/session_interface.h"
#include "vm/product/console.h"

static type_status verify_selected_speed(C_VOID *context, C_VOID *opaque)
{
    vm_session_speed speed;

    (C_VOID)context;
    return vm_session_get_speed((vm_session *)opaque, &speed) == TYPE_STATUS_OK &&
        speed == VM_SESSION_SPEED_TURBO ? TYPE_STATUS_OK : TYPE_STATUS_FAULT;
}

C_INT main(C_VOID)
{
    core_product_session_provider session_provider;
    core_product_session_manager *session_manager = STD_NULL;
    vm_session_machine_provider machine_provider;
    vm_product_console_context *console_context = STD_NULL;
    core_product_session_snapshot snapshots[2];
    const C_CHAR *configuration = "nxvm_console_lifecycle.yaml";
    STD_FILE *input;
    STD_FILE *profile;
    C_INT saved_stdin;

    profile = STD_FOPEN(configuration, "wb");
    input = tmpfile();
    if (profile == STD_NULL || input == STD_NULL ||
        STD_FPUTS("schema: nxvm-session\nprofile: ibm-5170-model-339\ndisplay: console\nboot: rom\nmedia:\n  floppy: null\n  hard_disk: null\n", profile) < 0 ||
        STD_FCLOSE(profile) != 0 ||
        STD_FPUTS("1\nspeed turbo\nsession list\nhelp\ninfo\nexit\n", input) < 0 ||
        fflush(input) != 0 ||
        STD_FSEEK(input, 0L, STD_SEEK_SET) != 0) {
        if (profile != STD_NULL) STD_FCLOSE(profile);
        if (input != STD_NULL) STD_FCLOSE(input);
        (C_VOID)STD_REMOVE(configuration);
        return 1;
    }
    saved_stdin = TEST_CONSOLE_DUP(TEST_CONSOLE_FILENO(STD_STDIN));
    if (saved_stdin < 0 ||
        TEST_CONSOLE_DUP2(TEST_CONSOLE_FILENO(input), TEST_CONSOLE_FILENO(STD_STDIN)) < 0) {
        if (saved_stdin >= 0) TEST_CONSOLE_CLOSE(saved_stdin);
        STD_FCLOSE(input);
        (C_VOID)STD_REMOVE(configuration);
        return 1;
    }

    vm_session_provider_initialize(&session_provider);
    if (core_product_session_manager_create(&session_provider, &session_manager) !=
            TYPE_STATUS_OK) {
        TEST_CONSOLE_DUP2(saved_stdin, TEST_CONSOLE_FILENO(STD_STDIN));
        TEST_CONSOLE_CLOSE(saved_stdin);
        STD_FCLOSE(input);
        return 1;
    }
    vm_composition_console_machine_provider_initialize(&machine_provider, session_manager);
    if (vm_product_console_context_create(&console_context) != TYPE_STATUS_OK) {
        TEST_CONSOLE_DUP2(saved_stdin, TEST_CONSOLE_FILENO(STD_STDIN));
        TEST_CONSOLE_CLOSE(saved_stdin);
        STD_FCLOSE(input);
        core_product_session_manager_destroy(session_manager);
        return 1;
    }
    vm_product_console_main(console_context, &machine_provider, session_manager, ".");

    TEST_CONSOLE_DUP2(saved_stdin, TEST_CONSOLE_FILENO(STD_STDIN));
    TEST_CONSOLE_CLOSE(saved_stdin);
    if (core_product_session_manager_list(session_manager, snapshots, 2u,
            &(STD_SIZE_T){0u}) != TYPE_STATUS_OK ||
        !snapshots[0].selected ||
        core_product_session_manager_apply_selected(session_manager,
            verify_selected_speed, STD_NULL) != TYPE_STATUS_OK ||
        STD_STRCMP(snapshots[0].details,
            "profile=ibm-5170-model-339 cpu=80286 fpu=none")) {
        STD_FCLOSE(input);
        (C_VOID)STD_REMOVE(configuration);
        vm_product_console_context_destroy(console_context);
        core_product_session_manager_destroy(session_manager);
        return 1;
    }
    STD_FCLOSE(input);
    (C_VOID)STD_REMOVE(configuration);
    vm_product_console_context_destroy(console_context);
    core_product_session_manager_destroy(session_manager);
    puts("M5:T96:S1:CONSOLE-LIFECYCLE:OK");
    return 0;
}
