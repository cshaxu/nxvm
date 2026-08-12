#include "type.h"

#include "core/product/utils.h"

C_INT main(C_VOID)
{
    type_unsigned_8 code[16] = {0};
    C_CHAR statement[0x100] = {0};
    type_unsigned_8 length;
    type_unsigned_32 paragraph_length;

    length = core_product_utils_aasm32("nop", code, 16);
    if (length != 1u || code[0] != 0x90u) return 11;
    if (core_product_utils_dasm32(statement, code, 16) != 1u || statement[0] == '\0') return 12;

    paragraph_length = core_product_utils_aasm32x("nop\nnop", code, 16);
    if (paragraph_length != 2u || code[0] != 0x90u || code[1] != 0x90u) return 13;

    puts("M5:T129:S2:CORE-PRODUCT-XASM:OK");
    return 0;
}
