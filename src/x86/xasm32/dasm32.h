/* Copyright 2012-2014 Neko. */

#ifndef X86_XASM32_DASM32_H
#define X86_XASM32_DASM32_H

#ifdef __cplusplus
extern "C" {
#endif

#include "lib/types/types_interface.h"

lib_u8 dasm32(char *stmt, lib_u8 *rcode, lib_bool flag32);

#ifdef __cplusplus
}/*_EOCD_*/
#endif

#endif
