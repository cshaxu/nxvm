#include "type.h"

#include "core/product/utils.h"

C_INT main(C_VOID)
{
    type_unsigned_8 code[16] = {0};
    C_CHAR statement[0x100] = {0};
    STD_SIZE_T length;
    STD_SIZE_T statement_length;
    STD_SIZE_T paragraph_length;

    if (core_product_utils_assemble("nop", 3u, code, sizeof(code), &length,
            TYPE_TRUE) != TYPE_STATUS_OK || length != 1u || code[0] != 0x90u) return 11;
    if (core_product_utils_disassemble(code, sizeof(code), statement,
            sizeof(statement), &statement_length, TYPE_TRUE) != TYPE_STATUS_OK ||
        statement_length == 0u || statement[0] == '\0') return 12;

    if (core_product_utils_assemble_paragraph("nop\nnop", 7u, code,
            sizeof(code), &paragraph_length, TYPE_TRUE) != TYPE_STATUS_OK ||
        paragraph_length != 2u || code[0] != 0x90u || code[1] != 0x90u) return 13;

    puts("M5:T129:S2:CORE-PRODUCT-XASM:OK");
    return 0;
}
