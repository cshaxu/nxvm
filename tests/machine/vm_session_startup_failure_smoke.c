#include "type.h"

#ifdef _WIN32

#include <io.h>
#define TEST_CONSOLE_DUP _dup
#define TEST_CONSOLE_DUP2 _dup2
#define TEST_CONSOLE_CLOSE _close
#define TEST_CONSOLE_FILENO _fileno

#include "core/product/session/session_interface.h"
#include "vm/composition/session/provider.h"
#include "vm/composition/session/console_machine_adapter.h"
#include "vm/composition/session/session_private.h"
#include "vm/product/console.h"

#if !defined(VM_PLATFORM_TEST_FAILURE_STAGE)
#error "This smoke requires a dedicated failure-stage build."
#endif

static C_INT stage_uses_window(C_INT stage)
{
    return stage >= 1 && stage <= 5;
}

typedef struct startup_failure_session_check {
    C_INT window;
    C_INT failed;
} startup_failure_session_check;

static type_status configure_session(C_VOID *context, C_VOID *opaque)
{
    startup_failure_session_check *check =
        (startup_failure_session_check *)context;

    if (check == STD_NULL || opaque == STD_NULL) return TYPE_STATUS_INVALID_ARGUMENT;
    vm_platform_run_context_set_window_display(((vm_session *)opaque)->platform_run_context,
        check->window);
    return TYPE_STATUS_OK;
}

static type_status verify_session(C_VOID *context, C_VOID *opaque)
{
    startup_failure_session_check *check =
        (startup_failure_session_check *)context;
    vm_session *session = (vm_session *)opaque;

    if (check == STD_NULL || session == STD_NULL) return TYPE_STATUS_INVALID_ARGUMENT;
    check->failed = session->start_outcome.valid &&
        session->start_outcome.status != TYPE_STATUS_OK &&
        !vm_session_control_is_running(&session->control) &&
        !vm_platform_run_handle_is_active(session->platform_run_handle);
    return TYPE_STATUS_OK;
}

int main(void)
{
    core_product_session_provider session_provider;
    core_product_session_manager *session_manager = STD_NULL;
    vm_session_machine_provider machine_provider;
    vm_product_console_context *console_context = STD_NULL;
    startup_failure_session_check session_check = {
        stage_uses_window(VM_PLATFORM_TEST_FAILURE_STAGE), 0
    };
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
            TYPE_STATUS_OK || core_product_session_manager_apply_selected(
            session_manager, configure_session, &session_check) != TYPE_STATUS_OK) goto done;
    vm_composition_console_machine_provider_initialize(&machine_provider, session_manager);
    if (vm_product_console_context_create(&console_context) != TYPE_STATUS_OK) goto done;
    vm_product_console_main(console_context, &machine_provider, session_manager, ".");
    fflush(STD_STDOUT);
    if (TEST_CONSOLE_DUP2(saved_stdout, TEST_CONSOLE_FILENO(STD_STDOUT)) < 0) goto done;
    TEST_CONSOLE_CLOSE(saved_stdout);
    saved_stdout = -1;
    if (STD_FSEEK(output, 0L, STD_SEEK_SET) != 0 ||
        STD_FREAD(text, 1u, sizeof(text) - 1u, output) == 0u) goto done;
    text[sizeof(text) - 1u] = '\0';
    if (core_product_session_manager_apply_selected(session_manager,
            verify_session, &session_check) != TYPE_STATUS_OK ||
        !session_check.failed ||
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
    vm_product_console_context_destroy(console_context);
    if (input != STD_NULL) STD_FCLOSE(input);
    if (output != STD_NULL) STD_FCLOSE(output);
    if (!passed) return 1;
    STD_PRINTF("M5:T313:S5:START-OUTCOME:OK\n");
    return 0;
}

#else
int main(void) { return 0; }
#endif
