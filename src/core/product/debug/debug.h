/* Copyright 2012-2014 Neko. */

#ifndef CORE_PRODUCT_DEBUG_H
#define CORE_PRODUCT_DEBUG_H

#ifdef __cplusplus
extern "C" {
#endif

#include "type.h"

#include "core/product/debug/debug_target.h"
#include "core/utils/wait_provider.h"

typedef struct core_product_debug_input_provider {
    C_VOID (*flush_console_input)(C_VOID *context);
    C_VOID *context;
} core_product_debug_input_provider;

typedef struct core_product_debugger core_product_debugger;

type_status core_product_debugger_create(core_product_debugger **out_debugger);
C_VOID core_product_debugger_destroy(core_product_debugger *debugger);
C_VOID core_product_debugger_run(core_product_debugger *debugger,
    const core_product_debug_target *target,
    const core_product_debug_input_provider *input_provider,
    const core_utils_wait_scope *wait_scope);

#ifdef __cplusplus
}/*_EOCD_*/
#endif

#endif
