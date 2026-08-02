/* Copyright 2012-2014 Neko. */

#ifndef NTVDM64_CORE_PRODUCT_DEBUG_H
#define NTVDM64_CORE_PRODUCT_DEBUG_H

#ifdef __cplusplus
extern "C" {
#endif

#include "core/product/debug/debug_target.h"

void debugMain(const core_product_debug_target *target);

#ifdef __cplusplus
}/*_EOCD_*/
#endif

#endif
