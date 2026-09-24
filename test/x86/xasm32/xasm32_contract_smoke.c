#include "lib/types/test.h"
#include "lib/types/file.h"
#include "x86/xasm32/xasm32_interface.h"
#include "x86/xasm32/aasm32.c"

static lib_i32 assembly_writes_are_bounded(void)
{
    aasm32_context context;
    aasm32_context *aasmContext = &context;
    const lib_u8 widths[] = {1u, 2u, 4u};
    for (lib_size w = 0; w < sizeof(widths); ++w) {
        for (lib_u8 position = 0; position <= 15u; ++position) {
            lib_u8 before[15];
            lib_memory_set(&context, 0, sizeof(context));
            lib_memory_set(acode, 0xa5, sizeof(acode));
            lib_memory_copy(before, acode, sizeof(before));
            iop = position;
            if (widths[w] == 1u) _c_setbyte(aasmContext, 0x12);
            if (widths[w] == 2u) _c_setword(aasmContext, 0x1234);
            if (widths[w] == 4u) _c_setdword(aasmContext, 0x12345678);
            if (position + widths[w] > 15u) {
                if (!flagError || iop != position ||
                    lib_memory_compare(before, acode, sizeof(before))) return LIB_FALSE;
            } else if (flagError || iop != position + widths[w]) return LIB_FALSE;
        }
    }
    return LIB_TRUE;
}

static lib_i32 xasm_output_is_unchanged(const lib_u8 *code,
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
    if (!assembly_writes_are_bounded()) return 10;
    char exact_statement[X86_XASM32_MAX_STATEMENT_BYTES];
    char overlong_statement[X86_XASM32_MAX_STATEMENT_BYTES + 1u];
    char statement[8];
    lib_u8 code[X86_XASM32_MAX_CODE_BYTES];
    lib_size result_bytes = 37u;
    lib_size instruction_bytes = 19u;

    lib_memory_set(exact_statement, ' ', sizeof(exact_statement));
    exact_statement[0] = 'n';
    exact_statement[1] = 'o';
    exact_statement[2] = 'p';
    if (x86_xasm32_assemble(exact_statement, sizeof(exact_statement),
            code, sizeof(code), &result_bytes, LIB_TRUE) != LIB_STATUS_OK ||
        result_bytes != 1u || code[0] != 0x90u) return 11;

    lib_memory_set(code, 0xa5, sizeof(code));
    result_bytes = 37u;
    if (x86_xasm32_assemble("nop", 3u, code, 0u, &result_bytes,
            LIB_TRUE) != LIB_STATUS_INVALID_ARGUMENT ||
        !xasm_output_is_unchanged(code, sizeof(code), 0xa5u, result_bytes, 37u)) {
        return 12;
    }

    lib_memory_set(overlong_statement, ' ', sizeof(overlong_statement));
    lib_memory_set(code, 0xa5, sizeof(code));
    result_bytes = 37u;
    if (x86_xasm32_assemble(overlong_statement,
            sizeof(overlong_statement), code, sizeof(code), &result_bytes,
            LIB_TRUE) != LIB_STATUS_INVALID_ARGUMENT ||
        !xasm_output_is_unchanged(code, sizeof(code), 0xa5u, result_bytes, 37u)) {
        return 13;
    }

    lib_memory_set(code, 0xa5, sizeof(code));
    result_bytes = 37u;
    if (x86_xasm32_assemble_paragraph("nop\nnop", 7u, code, 1u,
            &result_bytes, LIB_TRUE) != LIB_STATUS_LIMIT_EXCEEDED ||
        !xasm_output_is_unchanged(code, sizeof(code), 0xa5u, result_bytes, 37u)) {
        return 14;
    }

    code[0] = 0x90u;
    lib_memory_set(statement, 0xa5, sizeof(statement));
    result_bytes = 37u;
    if (x86_xasm32_disassemble(code, sizeof(code), statement, 1u,
            &result_bytes, &instruction_bytes, LIB_TRUE) != LIB_STATUS_LIMIT_EXCEEDED ||
        statement[0] != (char)0xa5 || result_bytes != 37u || instruction_bytes != 19u) return 15;

    lib_memory_set(code, 0xa5, sizeof(code));
    result_bytes = 37u;
    if (x86_xasm32_assemble("?", 1u, code, sizeof(code), &result_bytes,
            LIB_TRUE) != LIB_STATUS_UNSUPPORTED ||
        !xasm_output_is_unchanged(code, sizeof(code), 0xa5u, result_bytes, 37u)) {
        return 16;
    }

    return 0;
}
