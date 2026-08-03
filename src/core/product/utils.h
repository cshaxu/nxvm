/* Copyright 2012-2014 Neko. */

#ifndef NXVM_UTILS_H
#define NXVM_UTILS_H

#ifdef __cplusplus
extern "C" {
#endif

#include "type.h"

/* NXVM Library */
void core_product_utils_sleep(uint32_t milisec);

/* NXVM Assembler Library */
uint8_t core_product_utils_aasm32(const char *stmt, uint8_t *rcode, int flag32);
uint32_t core_product_utils_aasm32x(const char *stmt, uint8_t *rcode, int flag32);
uint8_t core_product_utils_dasm32(char *stmt, uint8_t *rcode, int flag32);

#ifdef __cplusplus
}/*_EOCD_*/
#endif

#endif
