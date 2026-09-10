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

#include "vm/product/console.h"

static type_status vm_product_console_exit_stop(C_VOID *context)
{
    C_INT *count = (C_INT *)context;

    if (count == STD_NULL) return TYPE_STATUS_INVALID_ARGUMENT;
    ++*count;
    return TYPE_STATUS_OK;
}

int main(C_VOID)
{
    vm_session_machine_provider machine_provider = {0};
    vm_product_console_context *console_context = STD_NULL;
    STD_FILE *input = STD_NULL;
    C_INT saved_stdin = -1;
    C_INT stops = 0;
    C_INT passed = 0;

    input = tmpfile();
    if (input == STD_NULL || STD_FPUTS("exit\n", input) < 0 ||
        fflush(input) != 0 || STD_FSEEK(input, 0L, STD_SEEK_SET) != 0) goto done;
    saved_stdin = TEST_DUP(TEST_FILENO(STD_STDIN));
    if (saved_stdin < 0 || TEST_DUP2(TEST_FILENO(input),
            TEST_FILENO(STD_STDIN)) < 0 ||
        vm_product_console_context_create(&console_context) != TYPE_STATUS_OK) goto done;
    machine_provider.stop = vm_product_console_exit_stop;
    machine_provider.context = &stops;
    vm_product_console_main(console_context, &machine_provider,
        ".");
    passed = stops == 1;

done:
    if (saved_stdin >= 0) {
        (C_VOID)TEST_DUP2(saved_stdin, TEST_FILENO(STD_STDIN));
        TEST_CLOSE(saved_stdin);
    }
    vm_product_console_context_destroy(console_context);
    if (input != STD_NULL) STD_FCLOSE(input);
    if (!passed) return 1;
    puts("M5:T518:S5:CONSOLE-EXIT-STOPS-ALL:OK");
    return 0;
}
