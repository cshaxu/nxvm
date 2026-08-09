#include "type.h"

#ifdef _WIN32

#include <io.h>
#define TEST_CONSOLE_DUP _dup
#define TEST_CONSOLE_DUP2 _dup2
#define TEST_CONSOLE_CLOSE _close
#define TEST_CONSOLE_FILENO _fileno

#include "core/product/session/session_interface.h"
#include "vm/composition/session/provider.h"
#include "vm/composition/session/session.h"
#include "vm/product/console.h"

#if !defined(NTVDM64_VM_PLATFORM_TEST_FAILURE_STAGE)
#error "This smoke requires a dedicated failure-stage build."
#endif

static C_INT stage_uses_window(C_INT stage)
{
    return stage >= 1 && stage <= 5;
}

int main(void)
{
    core_product_session_provider session_provider;
    core_product_session_manager *session_manager = STD_NULL;
    vm_product_console_machine_provider machine_provider;
    vm_product_console_context console_context;
    vm_session *session = STD_NULL;
    STD_FILE *input = STD_NULL;
    STD_FILE *output = STD_NULL;
    C_CHAR text[4096];
    C_INT saved_stdin = -1;
    C_INT saved_stdout = -1;
    C_INT passed = 0;

    input = tmpfile();
    output = tmpfile();
    if (input == STD_NULL || output == STD_NULL ||
        STD_FPUTS("start\nexit\n", input) < 0 || fflush(input) != 0 ||
        STD_FSEEK(input, 0L, STD_SEEK_SET) != 0) goto done;
    saved_stdin = TEST_CONSOLE_DUP(TEST_CONSOLE_FILENO(STD_STDIN));
    saved_stdout = TEST_CONSOLE_DUP(TEST_CONSOLE_FILENO(STD_STDOUT));
    if (saved_stdin < 0 || saved_stdout < 0 ||
        TEST_CONSOLE_DUP2(TEST_CONSOLE_FILENO(input), TEST_CONSOLE_FILENO(STD_STDIN)) < 0 ||
        TEST_CONSOLE_DUP2(TEST_CONSOLE_FILENO(output), TEST_CONSOLE_FILENO(STD_STDOUT)) < 0) {
        goto done;
    }
    vm_session_provider_initialize(&session_provider);
    if (core_product_session_manager_create(&session_provider, &session_manager) !=
            TYPE_STATUS_OK || core_product_session_manager_borrow_selected(
            session_manager, (C_VOID **)&session) != TYPE_STATUS_OK ||
        session == STD_NULL) goto done;
    vm_platform_run_context_set_window_display(&session->platform_run_context,
        stage_uses_window(NTVDM64_VM_PLATFORM_TEST_FAILURE_STAGE));
    vm_session_machine_provider_initialize(&machine_provider, session_manager);
    vm_product_console_main(&console_context, &machine_provider, session_manager);
    fflush(STD_STDOUT);
    if (TEST_CONSOLE_DUP2(saved_stdout, TEST_CONSOLE_FILENO(STD_STDOUT)) < 0) goto done;
    TEST_CONSOLE_CLOSE(saved_stdout);
    saved_stdout = -1;
    if (STD_FSEEK(output, 0L, STD_SEEK_SET) != 0 ||
        STD_FREAD(text, 1u, sizeof(text) - 1u, output) == 0u) goto done;
    text[sizeof(text) - 1u] = '\0';
    if (!session->start_outcome.valid ||
        session->start_outcome.status == TYPE_STATUS_OK ||
        vm_session_control_is_running(&session->control) ||
        vm_platform_run_handle_is_active(&session->platform_run_handle) ||
        strstr(text, "START failed:") == STD_NULL) goto done;
    passed = 1;

done:
    if (saved_stdin >= 0) {
        (C_VOID)TEST_CONSOLE_DUP2(saved_stdin, TEST_CONSOLE_FILENO(STD_STDIN));
        TEST_CONSOLE_CLOSE(saved_stdin);
    }
    if (saved_stdout >= 0) {
        (C_VOID)TEST_CONSOLE_DUP2(saved_stdout, TEST_CONSOLE_FILENO(STD_STDOUT));
        TEST_CONSOLE_CLOSE(saved_stdout);
    }
    if (session_manager != STD_NULL) core_product_session_manager_destroy(session_manager);
    if (input != STD_NULL) STD_FCLOSE(input);
    if (output != STD_NULL) STD_FCLOSE(output);
    if (!passed) return 1;
    STD_PRINTF("M5:T313:S5:START-OUTCOME:OK\n");
    return 0;
}

#else
int main(void) { return 0; }
#endif
