/* Copyright 2012-2014 Neko. */

/* UTILS implements code tracer and all utility functions. */

#include "type.h"

#include "core/product/debug/xasm32/aasm32.h"

#include "core/product/debug/xasm32/dasm32.h"

#include "core/product/utils.h"

/* Assembler Library */
type_unsigned_8 core_product_utils_aasm32(const C_CHAR *stmt, type_unsigned_8 *rcode, C_INT flag32)
{
    return aasm32(stmt, rcode, flag32);
}
type_unsigned_32 core_product_utils_aasm32x(const C_CHAR *stmt, type_unsigned_8 *rcode, C_INT flag32)
{
    return aasm32x(stmt, rcode, flag32);
}
type_unsigned_8 core_product_utils_dasm32(C_CHAR *stmt, type_unsigned_8 *rcode, C_INT flag32)
{
    return dasm32(stmt, rcode, flag32);
}
