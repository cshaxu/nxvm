/* Copyright 2012-2014 Neko. */

#include "type.h"

#include "core/product/config.h"

type_status core_product_parse_memory_kib(const C_CHAR *text,
    STD_SIZE_T *out_memory_bytes)
{
    STD_SIZE_T value = 0u;
    STD_SIZE_T index;

    if (text == STD_NULL || out_memory_bytes == STD_NULL || text[0] == '\0') {
        return TYPE_STATUS_INVALID_ARGUMENT;
    }
    for (index = 0u; text[index] != '\0'; ++index) {
        type_unsigned_8 digit;

        if (text[index] < '0' || text[index] > '9') return TYPE_STATUS_INVALID_ARGUMENT;
        digit = (type_unsigned_8)(text[index] - '0');
        if (value > (((~(STD_SIZE_T)0u >> 10) - digit) / 10u)) {
            return TYPE_STATUS_INVALID_ARGUMENT;
        }
        value = value * 10u + digit;
    }
    if (value == 0u) return TYPE_STATUS_INVALID_ARGUMENT;
    *out_memory_bytes = value << 10;
    return TYPE_STATUS_OK;
}
