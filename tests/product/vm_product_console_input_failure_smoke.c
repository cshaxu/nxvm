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

static C_INT allocation_failure;
static STD_SIZE_T allocation_attempts;

C_VOID *test_console_input_malloc(STD_SIZE_T size)
{
    allocation_attempts++;
    return allocation_failure ? STD_NULL : malloc(size);
}

static C_INT run_case(C_INT fail_allocation)
{
    vm_product_console_context *context = STD_NULL;
    vm_session_machine_provider provider = {0};
    STD_FILE *input = tmpfile();
    C_INT saved = -1;
    C_INT passed = 0;

    if (input == STD_NULL || fflush(input) != 0 ||
        STD_FSEEK(input, 0L, STD_SEEK_SET) != 0) goto done;
    saved = TEST_DUP(TEST_FILENO(STD_STDIN));
    if (saved < 0 || TEST_DUP2(TEST_FILENO(input), TEST_FILENO(STD_STDIN)) < 0) goto done;
    clearerr(STD_STDIN);
    allocation_failure = fail_allocation;
    allocation_attempts = 0u;
    if (vm_product_console_context_create(&context) != TYPE_STATUS_OK) goto done;
    vm_product_console_main(context, &provider,
        (core_product_session_manager *)(type_native_unsigned)1u, ".");
    if (allocation_attempts != 1u) goto done;
    if (!fail_allocation) {
        clearerr(STD_STDIN);
        vm_product_console_main(context, &provider,
            (core_product_session_manager *)(type_native_unsigned)1u, ".");
        if (allocation_attempts != 2u) goto done;
    }
    passed = 1;
done:
    allocation_failure = 0;
    if (saved >= 0) {
        (C_VOID)TEST_DUP2(saved, TEST_FILENO(STD_STDIN));
        TEST_CLOSE(saved);
    }
    clearerr(STD_STDIN);
    vm_product_console_context_destroy(context);
    if (input != STD_NULL) STD_FCLOSE(input);
    return passed;
}

C_INT main(C_VOID)
{
    if (!run_case(0) || !run_case(1)) return 1;
    puts("M5:T333:S3:CONSOLE-INPUT:OK");
    return 0;
}
