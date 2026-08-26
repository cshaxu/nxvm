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
#include "vm/product/session_catalog.h"

typedef struct console_memory_probe {
    C_INT exact_request_seen;
} console_memory_probe;

static type_status console_memory_open(C_VOID *context,
    core_product_session_id id, const core_product_session_open_options *options,
    C_VOID **out_session)
{
    console_memory_probe *probe = (console_memory_probe *)context;
    C_CHAR *session = (C_CHAR *)STD_MALLOC(1u);

    (C_VOID)id;
    if (session == STD_NULL || options == STD_NULL || out_session == STD_NULL) {
        STD_FREE(session);
        return TYPE_STATUS_NO_MEMORY;
    }
    if (options->argument_count == 0 && options->arguments == STD_NULL &&
        options->request_bytes == sizeof(vm_product_session_request) &&
        options->request != STD_NULL &&
        ((const vm_product_session_request *)options->request)->memory_bytes ==
            (STD_SIZE_T)4294967296u * 1024u &&
        !STD_STRCMP(((const vm_product_session_request *)options->request)->cpu,
            "8086") && !STD_STRCMP(((const vm_product_session_request *)options->request)->fpu,
            "8087")) {
        probe->exact_request_seen = 1;
    }
    *out_session = session;
    return TYPE_STATUS_OK;
}

static type_status console_memory_describe(C_VOID *context,
    const C_VOID *session, core_product_session_snapshot *snapshot)
{
    (C_VOID)context;
    (C_VOID)session;
    if (snapshot == STD_NULL) return TYPE_STATUS_INVALID_ARGUMENT;
    snapshot->state = CORE_PRODUCT_SESSION_STATE_STOPPED;
    snapshot->display = CORE_PRODUCT_SESSION_DISPLAY_CONSOLE;
    return TYPE_STATUS_OK;
}

static type_status console_memory_close(C_VOID *context, C_VOID *session)
{
    (C_VOID)context;
    STD_FREE(session);
    return TYPE_STATUS_OK;
}

static C_VOID console_memory_set_display(C_VOID *context,
    vm_session_display_mode mode)
{
    (C_VOID)context;
    (C_VOID)mode;
}

C_INT main(C_VOID)
{
    const C_CHAR *configuration = "t382-console-memory.yaml";
    const core_product_session_provider provider = {
        console_memory_open, console_memory_describe, console_memory_close, STD_NULL
    };
    core_product_session_provider configured_provider = provider;
    console_memory_probe probe = {0};
    vm_session_machine_provider machine_provider = {0};
    vm_product_console_context *console_context = STD_NULL;
    core_product_session_manager *manager = STD_NULL;
    STD_FILE *profile = STD_FOPEN(configuration, "wb");
    STD_FILE *input = tmpfile();
    C_INT saved_stdin = -1;
    C_INT result = 1;

    configured_provider.context = &probe;
    machine_provider.set_display_mode = console_memory_set_display;
    if (profile == STD_NULL || input == STD_NULL ||
        STD_FPUTS("schema: nxvm-session\nprofile: default-pc-at\ncpu: 8086\nfpu: 8087\nmemory_kib: 4294967296\ndisplay: console\nboot: rom\nmedia:\n  floppy: null\n  hard_disk: null\n", profile) < 0 ||
        STD_FCLOSE(profile) != 0 || STD_FPUTS("1\nexit\n", input) < 0 ||
        fflush(input) != 0 || STD_FSEEK(input, 0L, STD_SEEK_SET) != 0 ||
        core_product_session_manager_create(&configured_provider, &manager) !=
            TYPE_STATUS_OK) goto done;
    profile = STD_NULL;
    saved_stdin = TEST_DUP(TEST_FILENO(STD_STDIN));
    if (saved_stdin < 0 || TEST_DUP2(TEST_FILENO(input),
            TEST_FILENO(STD_STDIN)) < 0) goto done;
    if (vm_product_console_context_create(&console_context) != TYPE_STATUS_OK) goto done;
    vm_product_console_main(console_context, &machine_provider, manager, ".");
    if (probe.exact_request_seen) result = 0;
done:
    if (saved_stdin >= 0) {
        (C_VOID)TEST_DUP2(saved_stdin, TEST_FILENO(STD_STDIN));
        TEST_CLOSE(saved_stdin);
    }
    if (profile != STD_NULL) STD_FCLOSE(profile);
    if (input != STD_NULL) STD_FCLOSE(input);
    (C_VOID)STD_REMOVE(configuration);
    vm_product_console_context_destroy(console_context);
    core_product_session_manager_destroy(manager);
    if (result == 0) {
        STD_PRINTF("M5:T382:S8:CONSOLE-MEMORY:OK\n");
        STD_PRINTF("M5:T482:S3:CONSOLE-REQUEST:OK\n");
    }
    return result;
}
