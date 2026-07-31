/* Copyright 2012-2014 Neko. */

#ifndef NXVM_DASM32_H
#define NXVM_DASM32_H

#ifdef __cplusplus
extern "C" {
#endif

#include "../global.h"

uint8_t dasm32(char *stmt, uint8_t *rcode, int flag32);

#ifdef __cplusplus
}/*_EOCD_*/
#endif

#endif
