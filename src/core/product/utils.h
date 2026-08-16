/* Copyright 2012-2014 Neko. */

#ifndef CORE_PRODUCT_UTILS_H
#define CORE_PRODUCT_UTILS_H

#ifdef __cplusplus
extern "C"
{
#endif

#include "type.h"

#define CORE_PRODUCT_UTILS_XASM_MAX_STATEMENT_BYTES 255u
#define CORE_PRODUCT_UTILS_XASM_MAX_CODE_BYTES 15u

type_status core_product_utils_assemble(const C_CHAR *statement,
    STD_SIZE_T statement_bytes, type_unsigned_8 *code,
    STD_SIZE_T code_capacity, STD_SIZE_T *out_code_bytes, C_INT flag32);
type_status core_product_utils_assemble_paragraph(const C_CHAR *statement,
    STD_SIZE_T statement_bytes, type_unsigned_8 *code,
    STD_SIZE_T code_capacity, STD_SIZE_T *out_code_bytes, C_INT flag32);
type_status core_product_utils_disassemble(const type_unsigned_8 *code,
    STD_SIZE_T code_bytes, C_CHAR *statement, STD_SIZE_T statement_capacity,
    STD_SIZE_T *out_statement_bytes, C_INT flag32);
type_status core_product_utils_parse_memory_kib(const C_CHAR *text,
    STD_SIZE_T *out_memory_bytes);

#ifdef __cplusplus
} /*_EOCD_*/
#endif

#endif
