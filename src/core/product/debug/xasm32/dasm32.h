/* Copyright 2012-2014 Neko. */

#ifndef CORE_PRODUCT_DASM32_H
#define CORE_PRODUCT_DASM32_H

#ifdef __cplusplus
extern "C" {
#endif

#include "type.h"

type_unsigned_8 dasm32(C_CHAR *stmt, type_unsigned_8 *rcode, C_INT flag32);

#ifdef __cplusplus
}/*_EOCD_*/
#endif

#endif
