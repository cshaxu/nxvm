#include "common/xasm32/xasm32_interface.h"

static int xasm_output_is_unchanged(const lib_u8 *code,
    lib_size code_bytes, lib_u8 expected, lib_size output_bytes,
    lib_size expected_bytes)
{
    lib_size index;

    if (output_bytes != expected_bytes) return LIB_FALSE;
    for (index = 0u; index < code_bytes; ++index) {
        if (code[index] != expected) return LIB_FALSE;
    }
    return LIB_TRUE;
}

int main(void)
{
    char exact_statement[COMMON_XASM32_MAX_STATEMENT_BYTES];
    char overlong_statement[COMMON_XASM32_MAX_STATEMENT_BYTES + 1u];
    char statement[8];
    lib_u8 code[COMMON_XASM32_MAX_CODE_BYTES];
    lib_size result_bytes = 37u;

    lib_memory_set(exact_statement, ' ', sizeof(exact_statement));
    exact_statement[0] = 'n';
    exact_statement[1] = 'o';
    exact_statement[2] = 'p';
    if (common_xasm32_assemble(exact_statement, sizeof(exact_statement),
            code, sizeof(code), &result_bytes, LIB_TRUE) != LIB_STATUS_OK ||
        result_bytes != 1u || code[0] != 0x90u) return 11;

    lib_memory_set(code, 0xa5, sizeof(code));
    result_bytes = 37u;
    if (common_xasm32_assemble("nop", 3u, code, 0u, &result_bytes,
            LIB_TRUE) != LIB_STATUS_INVALID_ARGUMENT ||
        !xasm_output_is_unchanged(code, sizeof(code), 0xa5u, result_bytes, 37u)) {
        return 12;
    }

    lib_memory_set(overlong_statement, ' ', sizeof(overlong_statement));
    lib_memory_set(code, 0xa5, sizeof(code));
    result_bytes = 37u;
    if (common_xasm32_assemble(overlong_statement,
            sizeof(overlong_statement), code, sizeof(code), &result_bytes,
            LIB_TRUE) != LIB_STATUS_INVALID_ARGUMENT ||
        !xasm_output_is_unchanged(code, sizeof(code), 0xa5u, result_bytes, 37u)) {
        return 13;
    }

    lib_memory_set(code, 0xa5, sizeof(code));
    result_bytes = 37u;
    if (common_xasm32_assemble_paragraph("nop\nnop", 7u, code, 1u,
            &result_bytes, LIB_TRUE) != LIB_STATUS_LIMIT_EXCEEDED ||
        !xasm_output_is_unchanged(code, sizeof(code), 0xa5u, result_bytes, 37u)) {
        return 14;
    }

    code[0] = 0x90u;
    lib_memory_set(statement, 0xa5, sizeof(statement));
    result_bytes = 37u;
    if (common_xasm32_disassemble(code, sizeof(code), statement, 1u,
            &result_bytes, LIB_TRUE) != LIB_STATUS_LIMIT_EXCEEDED ||
        statement[0] != (char)0xa5 || result_bytes != 37u) return 15;

    lib_memory_set(code, 0xa5, sizeof(code));
    result_bytes = 37u;
    if (common_xasm32_assemble("?", 1u, code, sizeof(code), &result_bytes,
            LIB_TRUE) != LIB_STATUS_UNSUPPORTED ||
        !xasm_output_is_unchanged(code, sizeof(code), 0xa5u, result_bytes, 37u)) {
        return 16;
    }

    return 0;
}
