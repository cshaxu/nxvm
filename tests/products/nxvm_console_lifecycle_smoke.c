#ifndef _WIN32
#define _POSIX_C_SOURCE 200809L
#endif

#include "type.h"


#ifdef _WIN32

#include <io.h>
#define NXVM_DUP _dup
#define NXVM_DUP2 _dup2
#define NXVM_CLOSE _close
#define NXVM_FILENO _fileno
#else

#include <unistd.h>
#define NXVM_DUP dup
#define NXVM_DUP2 dup2
#define NXVM_CLOSE close
#define NXVM_FILENO fileno
#endif


#include "vm/composition/session/provider.h"
#include "vm/product/console.h"

C_INT main(C_VOID)
{
    core_product_session_provider session_provider;
    core_product_session_manager *session_manager = STD_NULL;
    vm_product_console_machine_provider machine_provider;
    nxvm_product_console_context console_context;
    STD_FILE *input;
    C_INT saved_stdin;

    input = tmpfile();
    if (input == STD_NULL ||
        STD_FPUTS("help\ninfo\nexit\n", input) < 0 ||
        fflush(input) != 0 ||
        STD_FSEEK(input, 0L, STD_SEEK_SET) != 0) {
        if (input != STD_NULL) STD_FCLOSE(input);
        return 1;
    }
    saved_stdin = NXVM_DUP(NXVM_FILENO(STD_STDIN));
    if (saved_stdin < 0 ||
        NXVM_DUP2(NXVM_FILENO(input), NXVM_FILENO(STD_STDIN)) < 0) {
        if (saved_stdin >= 0) NXVM_CLOSE(saved_stdin);
        STD_FCLOSE(input);
        return 1;
    }

    vm_session_provider_initialize(&session_provider);
    if (core_product_session_manager_create(&session_provider, &session_manager) !=
            NTVDM64_STATUS_OK ||
        core_product_session_manager_open(session_manager, STD_NULL) !=
            NTVDM64_STATUS_OK) {
        NXVM_DUP2(saved_stdin, NXVM_FILENO(STD_STDIN));
        NXVM_CLOSE(saved_stdin);
        STD_FCLOSE(input);
        return 1;
    }
    vm_session_machine_provider_initialize(&machine_provider, session_manager);
    vm_product_console_main(&console_context, &machine_provider);

    NXVM_DUP2(saved_stdin, NXVM_FILENO(STD_STDIN));
    NXVM_CLOSE(saved_stdin);
    STD_FCLOSE(input);
    core_product_session_manager_destroy(session_manager);
    puts("M5:T96:S1:CONSOLE-LIFECYCLE:OK");
    return 0;
}
