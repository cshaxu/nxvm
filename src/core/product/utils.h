/* Copyright 2012-2014 Neko. */

#ifndef NXVM_UTILS_H
#define NXVM_UTILS_H

#ifdef __cplusplus
extern "C" {
#endif

#include "type.h"
#include "core/product/wait_provider.h"

/* NXVM Library */
C_VOID core_product_utils_sleep(const core_product_wait_scope *scope,
    uint32_t milisec);

/* NXVM Assembler Library */
uint8_t core_product_utils_aasm32(const C_CHAR *stmt, uint8_t *rcode, C_INT flag32);
uint32_t core_product_utils_aasm32x(const C_CHAR *stmt, uint8_t *rcode, C_INT flag32);
uint8_t core_product_utils_dasm32(C_CHAR *stmt, uint8_t *rcode, C_INT flag32);

#ifdef __cplusplus
}/*_EOCD_*/
#endif

#endif
