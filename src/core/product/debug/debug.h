/* Copyright 2012-2014 Neko. */

#ifndef CORE_PRODUCT_DEBUG_H
#define CORE_PRODUCT_DEBUG_H

#ifdef __cplusplus
extern "C" {
#endif

#include "type.h"

#include "core/product/debug/debug_target.h"

typedef struct core_product_debugger core_product_debugger;

type_status core_product_debugger_create(core_product_debugger **out_debugger);
C_VOID core_product_debugger_destroy(core_product_debugger *debugger);
C_VOID core_product_debugger_run(core_product_debugger *debugger,
    const core_product_debug_target *target);

#ifdef __cplusplus
}/*_EOCD_*/
#endif

#endif
