/* Copyright 2012-2014 Neko. */

#ifndef NXVM_AASM32_H
#define NXVM_AASM32_H

#ifdef __cplusplus
extern "C" {
#endif

#include "../global.h"

/* assemble single instruction */
uint8_t aasm32(const char *stmt, uint8_t *rcode, int flag32);
/* assemble a paragraph of instructions */
uint32_t aasm32x(const char *stmt, uint8_t *rcode, int flag32);

#ifdef __cplusplus
}/*_EOCD_*/
#endif

#endif
