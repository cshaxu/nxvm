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

typedef struct vm_product_console_lifecycle_fixture {
    vm_product_console_lifecycle_reporter reporter;
    C_VOID *reporter_context;
    C_INT starts;
    C_INT stops;
} vm_product_console_lifecycle_fixture;

static type_status vm_product_console_lifecycle_open(C_VOID *context,
    core_product_session_id id, const core_product_session_open_options *options,
    C_VOID **out_session)
{
    (C_VOID)id;
    (C_VOID)options;
    if (context == STD_NULL || out_session == STD_NULL) return TYPE_STATUS_INVALID_ARGUMENT;
    *out_session = context;
    return TYPE_STATUS_OK;
}

static type_status vm_product_console_lifecycle_describe(C_VOID *context,
    const C_VOID *session, core_product_session_snapshot *snapshot)
{
    (C_VOID)context;
    (C_VOID)session;
    (C_VOID)snapshot;
    return TYPE_STATUS_OK;
}

static type_status vm_product_console_lifecycle_close(C_VOID *context,
    C_VOID *session)
{
    (C_VOID)context;
    (C_VOID)session;
    return TYPE_STATUS_OK;
}

static C_VOID vm_product_console_lifecycle_set_reporter(C_VOID *opaque,
    vm_product_console_lifecycle_reporter reporter, C_VOID *reporter_context)
{
    vm_product_console_lifecycle_fixture *fixture = opaque;

    if (fixture == STD_NULL) return;
    fixture->reporter = reporter;
    fixture->reporter_context = reporter_context;
}

static type_status vm_product_console_lifecycle_start(C_VOID *opaque)
{
    vm_product_console_lifecycle_fixture *fixture = opaque;

    if (fixture == STD_NULL || fixture->reporter == STD_NULL) {
        return TYPE_STATUS_INVALID_STATE;
    }
    ++fixture->starts;
    fixture->reporter(fixture->reporter_context, 7u, VM_SESSION_RUNNING);
    fixture->reporter(fixture->reporter_context, 7u, VM_SESSION_PAUSED);
    return TYPE_STATUS_OK;
}

static type_status vm_product_console_lifecycle_stop_all(C_VOID *opaque)
{
    vm_product_console_lifecycle_fixture *fixture = opaque;

    if (fixture == STD_NULL) return TYPE_STATUS_INVALID_ARGUMENT;
    ++fixture->stops;
    if (fixture->reporter != STD_NULL) {
        fixture->reporter(fixture->reporter_context, 7u, VM_SESSION_STOPPED);
    }
    return TYPE_STATUS_OK;
}

static C_INT vm_product_console_lifecycle_contains(const C_CHAR *text,
    const C_CHAR *needle)
{
    return text != STD_NULL && needle != STD_NULL && strstr(text, needle) != STD_NULL;
}

int main(C_VOID)
{
    vm_product_console_lifecycle_fixture fixture = {0};
    const core_product_session_provider session_provider = {
        vm_product_console_lifecycle_open,
        vm_product_console_lifecycle_describe,
        vm_product_console_lifecycle_close,
        &fixture
    };
    vm_session_machine_provider machine_provider = {0};
    core_product_session_manager *session_manager = STD_NULL;
    vm_product_console_context *console_context = STD_NULL;
    STD_FILE *input = STD_NULL;
    STD_FILE *output = STD_NULL;
    C_CHAR transcript[4096];
    STD_SIZE_T transcript_bytes;
    C_INT saved_input = -1;
    C_INT saved_output = -1;
    C_INT passed = 0;

    input = tmpfile();
    output = tmpfile();
    if (input == STD_NULL || output == STD_NULL ||
        STD_FPUTS("start\nhelp\nexit\n", input) < 0 || fflush(input) != 0 ||
        STD_FSEEK(input, 0L, STD_SEEK_SET) != 0 ||
        (saved_input = TEST_DUP(TEST_FILENO(STD_STDIN))) < 0 ||
        (saved_output = TEST_DUP(TEST_FILENO(STD_STDOUT))) < 0 ||
        TEST_DUP2(TEST_FILENO(input), TEST_FILENO(STD_STDIN)) < 0 ||
        TEST_DUP2(TEST_FILENO(output), TEST_FILENO(STD_STDOUT)) < 0 ||
        core_product_session_manager_create(&session_provider, &session_manager) !=
            TYPE_STATUS_OK ||
        core_product_session_manager_open(session_manager, STD_NULL) != TYPE_STATUS_OK ||
        vm_product_console_context_create(&console_context) != TYPE_STATUS_OK) goto done;
    machine_provider.context = &fixture;
    machine_provider.set_lifecycle_reporter = vm_product_console_lifecycle_set_reporter;
    machine_provider.start = vm_product_console_lifecycle_start;
    machine_provider.stop_all = vm_product_console_lifecycle_stop_all;
    vm_product_console_main(console_context, &machine_provider, session_manager, ".");
    STD_MEMSET(transcript, 0, sizeof(transcript));
    if (fflush(STD_STDOUT) != 0 || fflush(output) != 0 ||
        STD_FSEEK(output, 0L, STD_SEEK_SET) != 0 ||
        (transcript_bytes = fread(transcript, 1u, sizeof(transcript) - 1u,
            output)) == 0u) goto done;
    transcript[transcript_bytes] = '\0';
    passed = fixture.starts == 1 && fixture.stops == 1 &&
        vm_product_console_lifecycle_contains(transcript, "Session 7 running.") &&
        vm_product_console_lifecycle_contains(transcript, "Session 7 paused.") &&
        vm_product_console_lifecycle_contains(transcript, "Session 7 stopped.") &&
        strstr(transcript, "Session 7 paused.") < strstr(transcript, "Show help");

done:
    if (saved_input >= 0) {
        (C_VOID)TEST_DUP2(saved_input, TEST_FILENO(STD_STDIN));
        TEST_CLOSE(saved_input);
    }
    if (saved_output >= 0) {
        (C_VOID)TEST_DUP2(saved_output, TEST_FILENO(STD_STDOUT));
        TEST_CLOSE(saved_output);
    }
    vm_product_console_context_destroy(console_context);
    core_product_session_manager_destroy(session_manager);
    if (input != STD_NULL) STD_FCLOSE(input);
    if (output != STD_NULL) STD_FCLOSE(output);
    if (!passed) return 1;
    puts("M5:T526:S3:CONSOLE-LIFECYCLE-REPORT:OK");
    return 0;
}
