/* Copyright 2012-2014 Neko. */

#ifndef X86_NXVM_MACHINE_DEBUG_H
#define X86_NXVM_MACHINE_DEBUG_H

#ifdef __cplusplus
extern "C" {
#endif
#include "lib/types/types_interface.h"

#include "x86-core/machine/debug_interface.h"
#include "x86-debug/product/protocol_interface.h"

#define X86_NXVM_MACHINE_DEVICE_DEBUG "Unknown Hardware Debugger"

/* Debug completion is product-visible execution metadata, not a second
 * Machine lifecycle.  Common owns pause/resume state for composed runs. */
typedef enum x86_nxvm_machine_debug_stop_reason {
    X86_NXVM_MACHINE_DEBUG_STOP_NONE,
    X86_NXVM_MACHINE_DEBUG_STOP_BREAKPOINT,
    X86_NXVM_MACHINE_DEBUG_STOP_TRACE,
    X86_NXVM_MACHINE_DEBUG_STOP_WATCHPOINT
} x86_nxvm_machine_debug_stop_reason;

typedef struct {
    x86_debug_execution_plan_kind kind;
    lib_u64 remaining;
    lib_u64 executed;
    lib_u32 breakpoint_linear;
    lib_bool completion_pending;
    x86_nxvm_machine_debug_stop_reason completion_reason;
    lib_u64 completion_executed;
} t_debug_execution_plan;

typedef struct {
    t_debug_execution_plan plan;
    x86_core_cpu_debug_instruction_observation observation;
    lib_bool observation_valid;
} t_debug;

void x86_nxvm_machine_debug_initialize(t_debug *debug);
void x86_nxvm_machine_debug_reset(t_debug *debug);
void x86_nxvm_machine_debug_refresh(t_debug *debug,
    const x86_core_cpu_debug_instruction_observation *observation);
void x86_nxvm_machine_debug_finalize(t_debug *debug);
lib_status x86_nxvm_machine_debug_set_execution_plan(t_debug *debug,
    const x86_debug_request *request);
void x86_nxvm_machine_debug_clear_execution_plan(t_debug *debug);
lib_u64 x86_nxvm_machine_debug_limit_instruction_budget(
    const t_debug *debug, lib_u64 requested);
lib_bool x86_nxvm_machine_debug_breakpoint_due(const t_debug *debug);
void x86_nxvm_machine_debug_complete_breakpoint(t_debug *debug);
void x86_nxvm_machine_debug_complete_watchpoint(t_debug *debug);
void x86_nxvm_machine_debug_complete_run(t_debug *debug,
    lib_u64 executed);
lib_bool x86_nxvm_machine_debug_completion_pending(const t_debug *debug,
    x86_nxvm_machine_debug_stop_reason *out_reason);
lib_bool x86_nxvm_machine_debug_take_completion(t_debug *debug,
    x86_nxvm_machine_debug_stop_reason *out_reason, lib_u64 *out_executed);

#ifdef __cplusplus
}/*_EOCD_*/
#endif

#endif
