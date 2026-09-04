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

#include "core/product/session/session_interface.h"
#include "core/product/session/session_provider.h"
#include "vm/product/console.h"

static type_status vm_product_console_exit_open(C_VOID *context,
    core_product_session_id id, const core_product_session_open_options *options,
    C_VOID **out_session)
{
    (C_VOID)context;
    (C_VOID)id;
    (C_VOID)options;
    (C_VOID)out_session;
    return TYPE_STATUS_UNSUPPORTED;
}

static type_status vm_product_console_exit_describe(C_VOID *context,
    const C_VOID *session, core_product_session_snapshot *snapshot)
{
    (C_VOID)context;
    (C_VOID)session;
    (C_VOID)snapshot;
    return TYPE_STATUS_UNSUPPORTED;
}

static type_status vm_product_console_exit_close(C_VOID *context, C_VOID *session)
{
    (C_VOID)context;
    (C_VOID)session;
    return TYPE_STATUS_OK;
}

static type_status vm_product_console_exit_stop_all(C_VOID *context)
{
    C_INT *count = (C_INT *)context;

    if (count == STD_NULL) return TYPE_STATUS_INVALID_ARGUMENT;
    ++*count;
    return TYPE_STATUS_OK;
}

int main(C_VOID)
{
    const core_product_session_provider session_provider = {
        vm_product_console_exit_open,
        vm_product_console_exit_describe,
        vm_product_console_exit_close,
        STD_NULL
    };
    vm_session_machine_provider machine_provider = {0};
    core_product_session_manager *session_manager = STD_NULL;
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
        core_product_session_manager_create(&session_provider, &session_manager) !=
            TYPE_STATUS_OK || vm_product_console_context_create(&console_context) !=
            TYPE_STATUS_OK) goto done;
    machine_provider.stop_all = vm_product_console_exit_stop_all;
    machine_provider.context = &stops;
    vm_product_console_main(console_context, &machine_provider, session_manager,
        ".");
    passed = stops == 1;

done:
    if (saved_stdin >= 0) {
        (C_VOID)TEST_DUP2(saved_stdin, TEST_FILENO(STD_STDIN));
        TEST_CLOSE(saved_stdin);
    }
    vm_product_console_context_destroy(console_context);
    core_product_session_manager_destroy(session_manager);
    if (input != STD_NULL) STD_FCLOSE(input);
    if (!passed) return 1;
    puts("M5:T518:S5:CONSOLE-EXIT-STOPS-ALL:OK");
    return 0;
}
