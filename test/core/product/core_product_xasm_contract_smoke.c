#include "type.h"

#include "core/product/utils.h"

static STD_SIZE_T allocation_attempts;
static STD_SIZE_T allocation_failure_attempt;

C_VOID *test_xasm_malloc(STD_SIZE_T size)
{
    allocation_attempts++;
    return allocation_attempts == allocation_failure_attempt ? STD_NULL :
        malloc(size);
}

static C_INT xasm_output_is_unchanged(const type_unsigned_8 *code,
    STD_SIZE_T code_bytes, type_unsigned_8 expected, STD_SIZE_T output_bytes,
    STD_SIZE_T expected_bytes)
{
    STD_SIZE_T index;

    if (output_bytes != expected_bytes) return TYPE_FALSE;
    for (index = 0u; index < code_bytes; ++index) {
        if (code[index] != expected) return TYPE_FALSE;
    }
    return TYPE_TRUE;
}

int main(C_VOID)
{
    C_CHAR text[4] = "x";
    C_CHAR exact_statement[CORE_PRODUCT_UTILS_XASM_MAX_STATEMENT_BYTES];
    C_CHAR overlong_statement[CORE_PRODUCT_UTILS_XASM_MAX_STATEMENT_BYTES + 1u];
    C_CHAR statement[8];
    type_unsigned_8 code[CORE_PRODUCT_UTILS_XASM_MAX_CODE_BYTES];
    STD_SIZE_T result_bytes = 37u;

    if (core_product_utils_append_text(text, sizeof(text), "yz") !=
            TYPE_STATUS_OK || STD_STRCMP(text, "xyz") ||
        core_product_utils_append_text(text, sizeof(text), "q") !=
            TYPE_STATUS_INVALID_ARGUMENT || STD_STRCMP(text, "xyz") ||
        core_product_utils_copy_text(text, sizeof(text), "abcd") !=
            TYPE_STATUS_INVALID_ARGUMENT || STD_STRCMP(text, "xyz")) return 10;

    STD_MEMSET(exact_statement, ' ', sizeof(exact_statement));
    exact_statement[0] = 'n';
    exact_statement[1] = 'o';
    exact_statement[2] = 'p';
    if (core_product_utils_assemble(exact_statement, sizeof(exact_statement),
            code, sizeof(code), &result_bytes, TYPE_TRUE) != TYPE_STATUS_OK ||
        result_bytes != 1u || code[0] != 0x90u) return 11;

    STD_MEMSET(code, 0xa5, sizeof(code));
    result_bytes = 37u;
    if (core_product_utils_assemble("nop", 3u, code, 0u, &result_bytes,
            TYPE_TRUE) != TYPE_STATUS_INVALID_ARGUMENT ||
        !xasm_output_is_unchanged(code, sizeof(code), 0xa5u, result_bytes, 37u)) {
        return 12;
    }

    STD_MEMSET(overlong_statement, ' ', sizeof(overlong_statement));
    STD_MEMSET(code, 0xa5, sizeof(code));
    result_bytes = 37u;
    if (core_product_utils_assemble(overlong_statement,
            sizeof(overlong_statement), code, sizeof(code), &result_bytes,
            TYPE_TRUE) != TYPE_STATUS_INVALID_ARGUMENT ||
        !xasm_output_is_unchanged(code, sizeof(code), 0xa5u, result_bytes, 37u)) {
        return 13;
    }

    STD_MEMSET(code, 0xa5, sizeof(code));
    result_bytes = 37u;
    if (core_product_utils_assemble_paragraph("nop\nnop", 7u, code, 1u,
            &result_bytes, TYPE_TRUE) != TYPE_STATUS_FAULT ||
        !xasm_output_is_unchanged(code, sizeof(code), 0xa5u, result_bytes, 37u)) {
        return 14;
    }

    code[0] = 0x90u;
    STD_MEMSET(statement, 0xa5, sizeof(statement));
    result_bytes = 37u;
    if (core_product_utils_disassemble(code, sizeof(code), statement, 1u,
            &result_bytes, TYPE_TRUE) != TYPE_STATUS_FAULT ||
        statement[0] != (C_CHAR)0xa5 || result_bytes != 37u) return 15;

    STD_MEMSET(code, 0xa5, sizeof(code));
    result_bytes = 37u;
    if (core_product_utils_assemble("?", 1u, code, sizeof(code), &result_bytes,
            TYPE_TRUE) != TYPE_STATUS_FAULT ||
        !xasm_output_is_unchanged(code, sizeof(code), 0xa5u, result_bytes, 37u)) {
        return 16;
    }

    allocation_attempts = 0u;
    allocation_failure_attempt = 1u;
    STD_MEMSET(code, 0xa5, sizeof(code));
    result_bytes = 37u;
    if (core_product_utils_assemble_paragraph("nop\nnop", 7u, code,
            sizeof(code), &result_bytes, TYPE_TRUE) != TYPE_STATUS_NO_MEMORY ||
        !xasm_output_is_unchanged(code, sizeof(code), 0xa5u, result_bytes, 37u)) {
        return 17;
    }

    allocation_attempts = 0u;
    allocation_failure_attempt = 2u;
    STD_MEMSET(code, 0xa5, sizeof(code));
    result_bytes = 37u;
    if (core_product_utils_assemble_paragraph("nop\nnop", 7u, code,
            sizeof(code), &result_bytes, TYPE_TRUE) != TYPE_STATUS_NO_MEMORY ||
        !xasm_output_is_unchanged(code, sizeof(code), 0xa5u, result_bytes, 37u)) {
        return 18;
    }

    allocation_failure_attempt = 0u;
    puts("M5:T335:S3:XASM-CONTRACT:OK");
    return 0;
}
