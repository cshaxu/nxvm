/* Copyright 2012-2014 Neko. */

#ifndef CORE_PRODUCT_AASM32_H
#define CORE_PRODUCT_AASM32_H

#ifdef __cplusplus
extern "C" {
#endif

#include "type.h"

/* Internal xasm engine entries.  Public callers use core/product/utils.h. */
type_unsigned_8 aasm32(const C_CHAR *stmt, type_unsigned_8 *rcode, C_INT flag32);
type_status aasm32x(const C_CHAR *stmt, STD_SIZE_T code_capacity,
    type_unsigned_8 *rcode, STD_SIZE_T *out_code_bytes, C_INT flag32);

#ifdef __cplusplus
}/*_EOCD_*/
#endif

#endif
