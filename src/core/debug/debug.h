/* Copyright 2012-2014 Neko. */

#ifndef CORE_DEBUG_H
#define CORE_DEBUG_H

#ifdef __cplusplus
extern "C" {
#endif

#include "type.h"

#include "core/debug/debug_target.h"

typedef struct core_debugger core_debugger;

type_status core_debugger_create(core_debugger **out_debugger);
C_VOID core_debugger_destroy(core_debugger *debugger);
C_VOID core_debugger_run(core_debugger *debugger,
    const core_debug_target *target);

#ifdef __cplusplus
}/*_EOCD_*/
#endif

#endif
