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

    if (common_xasm32_assemble("mov dr0,eax", 11u, code, sizeof(code), &length,
            LIB_TRUE) != LIB_STATUS_OK) return 14;
    if (length != 4u || code[0] != 0x67u) return 141;
    if (code[1] != 0x0fu) return (int)code[1];
    if (code[2] != 0x23u) return (int)code[2];
    if (code[3] != 0xc0u) return (int)code[3];
    if (common_xasm32_assemble("mov eax,dr0", 11u, code, sizeof(code), &length,
            LIB_TRUE) != LIB_STATUS_OK || length != 4u || code[0] != 0x67u ||
        code[1] != 0x0fu || code[2] != 0x21u || code[3] != 0xc0u) return 15;
    if (common_xasm32_assemble("mov tr6,eax", 11u, code, sizeof(code), &length,
            LIB_TRUE) != LIB_STATUS_OK || length != 4u || code[0] != 0x67u ||
        code[1] != 0x0fu || code[2] != 0x26u || code[3] != 0xf0u) return 16;
    if (common_xasm32_assemble("mov eax,tr6", 11u, code, sizeof(code), &length,
            LIB_TRUE) != LIB_STATUS_OK || length != 4u || code[0] != 0x67u ||
        code[1] != 0x0fu || code[2] != 0x24u || code[3] != 0xf0u) return 17;

    return 0;
}
