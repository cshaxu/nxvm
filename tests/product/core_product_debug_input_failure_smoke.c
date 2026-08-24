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

#include "core/product/debug/debug.h"

static C_INT allocation_failure;
static STD_SIZE_T allocation_attempts;

C_VOID *test_debug_input_malloc(STD_SIZE_T size)
{
    allocation_attempts++;
    return allocation_failure ? STD_NULL : malloc(size);
}

static C_INT read_register(C_VOID *context, core_product_debug_register reg,
    type_unsigned_32 *out_value)
{
    (C_VOID)context;
    (C_VOID)reg;
    if (out_value == STD_NULL) return 1;
    *out_value = 0u;
    return 0;
}

static type_unsigned_32 code_base(C_VOID *context)
{
    (C_VOID)context;
    return 0u;
}

static C_INT write_register(C_VOID *context, core_product_debug_register reg,
    type_unsigned_32 value)
{
    STD_SIZE_T *writes = (STD_SIZE_T *)context;

    (C_VOID)reg;
    (C_VOID)value;
    (*writes)++;
    return 0;
}

static C_INT run_case(const C_CHAR *text, C_INT fail_allocation)
{
    core_product_debugger *debugger = STD_NULL;
    core_product_debug_target target = {0};
    STD_SIZE_T writes = 0u;
    STD_FILE *input = tmpfile();
    C_INT saved = -1;
    C_INT passed = 0;

    target.read_register = read_register;
    target.write_register = write_register;
    target.get_code_base = code_base;
    target.context = &writes;
    if (input == STD_NULL || (text != STD_NULL && STD_FPUTS(text, input) < 0) ||
        fflush(input) != 0 || STD_FSEEK(input, 0L, STD_SEEK_SET) != 0 ||
        core_product_debugger_create(&debugger) != TYPE_STATUS_OK) goto done;
    saved = TEST_DUP(TEST_FILENO(STD_STDIN));
    if (saved < 0 || TEST_DUP2(TEST_FILENO(input), TEST_FILENO(STD_STDIN)) < 0) goto done;
    clearerr(STD_STDIN);
    allocation_failure = fail_allocation;
    allocation_attempts = 0u;
    core_product_debugger_run(debugger, &target, STD_NULL, STD_NULL);
    if (allocation_attempts != 1u || writes != 0u) {
        goto done;
    }
    if (!fail_allocation) {
        if (STD_FSEEK(input, 0L, STD_SEEK_SET) != 0) goto done;
        clearerr(STD_STDIN);
        core_product_debugger_run(debugger, &target, STD_NULL, STD_NULL);
        if (allocation_attempts != 2u || writes != 0u) {
            goto done;
        }
    }
    passed = 1;
done:
    allocation_failure = 0;
    if (saved >= 0) {
        (C_VOID)TEST_DUP2(saved, TEST_FILENO(STD_STDIN));
        TEST_CLOSE(saved);
    }
    clearerr(STD_STDIN);
    core_product_debugger_destroy(debugger);
    if (input != STD_NULL) STD_FCLOSE(input);
    return passed;
}

C_INT main(C_VOID)
{
    if (!run_case(STD_NULL, 0) || !run_case(STD_NULL, 1) ||
        !run_case("a\n", 0) || !run_case("v\n", 0) ||
        !run_case("r ax\n", 0)) return 1;
    puts("M5:T333:S3:DEBUG-INPUT:OK");
    return 0;
}
