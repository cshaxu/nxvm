/* Copyright 2012-2014 Neko. */

#ifndef CORE_PRODUCT_AASM32_H
#define CORE_PRODUCT_AASM32_H

#ifdef __cplusplus
extern "C" {
#endif

#include "type.h"

/* assemble single instruction */
uint8_t aasm32(const C_CHAR *stmt, uint8_t *rcode, C_INT flag32);
/* assemble a paragraph of instructions */
uint32_t aasm32x(const C_CHAR *stmt, uint8_t *rcode, C_INT flag32);

#ifdef __cplusplus
}/*_EOCD_*/
#endif

#endif
