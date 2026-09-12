#include "common/xasm32/xasm32_interface.h"

int main(void)
{
    lib_u8 code[16] = {0};
    char statement[COMMON_XASM32_MAX_STATEMENT_BYTES + 1u] = {0};
    lib_size length;
    lib_size statement_length;
    lib_size paragraph_length;

    if (common_xasm32_assemble("nop", 3u, code, sizeof(code), &length,
            LIB_TRUE) != LIB_STATUS_OK || length != 1u || code[0] != 0x90u) return 11;
    if (common_xasm32_disassemble(code, sizeof(code), statement,
            sizeof(statement), &statement_length, LIB_TRUE) != LIB_STATUS_OK ||
        statement_length == 0u || statement[0] == '\0') return 12;

    if (common_xasm32_assemble_paragraph("nop\nnop", 7u, code,
            sizeof(code), &paragraph_length, LIB_TRUE) != LIB_STATUS_OK ||
        paragraph_length != 2u || code[0] != 0x90u || code[1] != 0x90u) return 13;

    return 0;
}
