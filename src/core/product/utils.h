/* Copyright 2012-2014 Neko. */

#ifndef NXVM_UTILS_H
#define NXVM_UTILS_H

#ifdef __cplusplus
extern "C" {
#endif

#include "type.h"

/* NXVM Library */
void utilsSleep(uint32_t milisec);

/* NXVM Assembler Library */
uint8_t utilsAasm32(const char *stmt, uint8_t *rcode, int flag32);
uint32_t utilsAasm32x(const char *stmt, uint8_t *rcode, int flag32);
uint8_t utilsDasm32(char *stmt, uint8_t *rcode, int flag32);

#ifdef __cplusplus
}/*_EOCD_*/
#endif

#endif
