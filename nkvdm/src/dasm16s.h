/* Copyright 2012-2014 Neko. */

#ifndef NKVDM_DASM16S_H
#define NKVDM_DASM16S_H

#ifdef __cplusplus
extern "C" {
#endif

#include "../../src/global.h"

uint8_t dasm16s(char *stmt, uint8_t *rcode, uint16_t offset);

#ifdef __cplusplus
}/*_EOCD_*/
#endif

#endif
