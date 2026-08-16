/* Copyright 2012-2014 Neko. */

/* UTILS implements code tracer and all utility functions. */

#include "type.h"

#include "core/product/debug/xasm32/aasm32.h"

#include "core/product/debug/xasm32/dasm32.h"

#include "core/product/utils.h"

#define CORE_PRODUCT_UTILS_XASM_STATEMENT_CAPACITY \
    (CORE_PRODUCT_UTILS_XASM_MAX_STATEMENT_BYTES + 1u)

static type_status core_product_utils_validate_statement(const C_CHAR *statement,
    STD_SIZE_T statement_bytes, C_INT paragraph)
{
    STD_SIZE_T index;
    STD_SIZE_T line_bytes = 0u;

    if (statement == STD_NULL || statement_bytes == 0u) {
        return TYPE_STATUS_INVALID_ARGUMENT;
    }
    for (index = 0u; index < statement_bytes; ++index) {
        if (statement[index] == '\0') return TYPE_STATUS_INVALID_ARGUMENT;
        if (statement[index] == '\n' && paragraph) {
            if (line_bytes > CORE_PRODUCT_UTILS_XASM_MAX_STATEMENT_BYTES) {
                return TYPE_STATUS_INVALID_ARGUMENT;
            }
            line_bytes = 0u;
        } else {
            ++line_bytes;
            if (line_bytes > CORE_PRODUCT_UTILS_XASM_MAX_STATEMENT_BYTES) {
                return TYPE_STATUS_INVALID_ARGUMENT;
            }
        }
    }
    return TYPE_STATUS_OK;
}

type_status core_product_utils_assemble(const C_CHAR *statement,
    STD_SIZE_T statement_bytes, type_unsigned_8 *code,
    STD_SIZE_T code_capacity, STD_SIZE_T *out_code_bytes, C_INT flag32)
{
    C_CHAR local_statement[CORE_PRODUCT_UTILS_XASM_STATEMENT_CAPACITY];
    type_unsigned_8 local_code[CORE_PRODUCT_UTILS_XASM_MAX_CODE_BYTES];
    type_unsigned_8 code_bytes;
    type_status status;

    if (code == STD_NULL || out_code_bytes == STD_NULL || code_capacity == 0u ||
        (flag32 != TYPE_FALSE && flag32 != TYPE_TRUE)) return TYPE_STATUS_INVALID_ARGUMENT;
    status = core_product_utils_validate_statement(statement, statement_bytes,
        TYPE_FALSE);
    if (status != TYPE_STATUS_OK) return status;
    STD_MEMCPY(local_statement, statement, statement_bytes);
    local_statement[statement_bytes] = '\0';
    code_bytes = aasm32(local_statement, local_code, flag32);
    if (code_bytes == 0u || code_bytes > code_capacity) return TYPE_STATUS_FAULT;
    STD_MEMCPY(code, local_code, code_bytes);
    *out_code_bytes = code_bytes;
    return TYPE_STATUS_OK;
}

type_status core_product_utils_assemble_paragraph(const C_CHAR *statement,
    STD_SIZE_T statement_bytes, type_unsigned_8 *code,
    STD_SIZE_T code_capacity, STD_SIZE_T *out_code_bytes, C_INT flag32)
{
    C_CHAR *local_statement;
    type_status status;

    if (code == STD_NULL || out_code_bytes == STD_NULL || code_capacity == 0u ||
        statement_bytes == (STD_SIZE_T)-1 ||
        (flag32 != TYPE_FALSE && flag32 != TYPE_TRUE)) return TYPE_STATUS_INVALID_ARGUMENT;
    status = core_product_utils_validate_statement(statement, statement_bytes,
        TYPE_TRUE);
    if (status != TYPE_STATUS_OK) return status;
    local_statement = (C_CHAR *)STD_MALLOC(statement_bytes + 1u);
    if (local_statement == STD_NULL) return TYPE_STATUS_NO_MEMORY;
    STD_MEMCPY(local_statement, statement, statement_bytes);
    local_statement[statement_bytes] = '\0';
    status = aasm32x(local_statement, code_capacity, code, out_code_bytes, flag32);
    STD_FREE(local_statement);
    return status;
}

type_status core_product_utils_disassemble(const type_unsigned_8 *code,
    STD_SIZE_T code_bytes, C_CHAR *statement, STD_SIZE_T statement_capacity,
    STD_SIZE_T *out_statement_bytes, C_INT flag32)
{
    C_CHAR local_statement[CORE_PRODUCT_UTILS_XASM_STATEMENT_CAPACITY];
    type_unsigned_8 decoded_bytes;
    STD_SIZE_T statement_bytes;

    if (code == STD_NULL || statement == STD_NULL || out_statement_bytes == STD_NULL ||
        code_bytes < CORE_PRODUCT_UTILS_XASM_MAX_CODE_BYTES ||
        statement_capacity == 0u ||
        (flag32 != TYPE_FALSE && flag32 != TYPE_TRUE)) return TYPE_STATUS_INVALID_ARGUMENT;
    decoded_bytes = dasm32(local_statement, (type_unsigned_8 *)code, flag32);
    statement_bytes = STD_STRLEN(local_statement);
    if (decoded_bytes == 0u || statement_bytes >= statement_capacity) {
        return TYPE_STATUS_FAULT;
    }
    STD_MEMCPY(statement, local_statement, statement_bytes + 1u);
    *out_statement_bytes = statement_bytes;
    return TYPE_STATUS_OK;
}

type_status core_product_utils_parse_memory_kib(const C_CHAR *text,
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

type_status core_product_utils_copy_text(C_CHAR *destination,
    STD_SIZE_T destination_capacity, const C_CHAR *source)
{
    STD_SIZE_T source_bytes;

    if (destination == STD_NULL || source == STD_NULL ||
        destination_capacity == 0u) return TYPE_STATUS_INVALID_ARGUMENT;
    source_bytes = STD_STRLEN(source);
    if (source_bytes >= destination_capacity) return TYPE_STATUS_INVALID_ARGUMENT;
    STD_MEMCPY(destination, source, source_bytes + 1u);
    return TYPE_STATUS_OK;
}

type_status core_product_utils_append_text(C_CHAR *destination,
    STD_SIZE_T destination_capacity, const C_CHAR *source)
{
    STD_SIZE_T destination_bytes;
    STD_SIZE_T source_bytes;

    if (destination == STD_NULL || source == STD_NULL ||
        destination_capacity == 0u) return TYPE_STATUS_INVALID_ARGUMENT;
    destination_bytes = STD_STRLEN(destination);
    source_bytes = STD_STRLEN(source);
    if (destination_bytes >= destination_capacity ||
        source_bytes >= destination_capacity - destination_bytes) {
        return TYPE_STATUS_INVALID_ARGUMENT;
    }
    STD_MEMCPY(destination + destination_bytes, source, source_bytes + 1u);
    return TYPE_STATUS_OK;
}
