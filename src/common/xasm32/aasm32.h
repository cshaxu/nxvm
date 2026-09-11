/* Copyright 2012-2014 Neko. */

#ifndef COMMON_XASM32_AASM32_H
#define COMMON_XASM32_AASM32_H

#ifdef __cplusplus
extern "C" {
#endif

#include "lib/types/types_interface.h"
/* Internal xasm engine entries. Public callers use xasm32_interface.h. */
lib_u8 aasm32(const char *stmt, lib_u8 *rcode, int flag32);
lib_status aasm32x(const char *stmt, lib_size code_capacity,
    lib_u8 *rcode, lib_size *out_code_bytes, int flag32);

#ifdef __cplusplus
}/*_EOCD_*/
#endif

#endif
