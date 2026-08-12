/* Copyright 2012-2014 Neko. */

#ifndef CORE_PRODUCT_UTILS_H
#define CORE_PRODUCT_UTILS_H

#ifdef __cplusplus
extern "C"
{
#endif

#include "type.h"
    /* Assembler Library */
    type_unsigned_8 core_product_utils_aasm32(const C_CHAR *stmt, type_unsigned_8 *rcode, C_INT flag32);
    type_unsigned_32 core_product_utils_aasm32x(const C_CHAR *stmt, type_unsigned_8 *rcode, C_INT flag32);
    type_unsigned_8 core_product_utils_dasm32(C_CHAR *stmt, type_unsigned_8 *rcode, C_INT flag32);

#ifdef __cplusplus
} /*_EOCD_*/
#endif

#endif
