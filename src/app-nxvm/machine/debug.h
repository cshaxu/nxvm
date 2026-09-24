/* Copyright 2012-2014 Neko. */

#ifndef VM_MACHINE_DEBUG_H
#define VM_MACHINE_DEBUG_H

#ifdef __cplusplus
extern "C" {
#endif
#include "lib/types/types_interface.h"

#include "app-nxvm/devices/debug_interface.h"
#include "x86/debug/protocol_interface.h"
#include "app-nxvm/machine/machine_interface.h"

#define VM_MACHINE_DEVICE_DEBUG "Unknown Hardware Debugger"

/* Debug completion is product-visible execution metadata, not a second
 * Machine lifecycle.  Common owns pause/resume state for composed runs. */
typedef enum vm_machine_debug_stop_reason {
    VM_MACHINE_DEBUG_STOP_NONE,
    VM_MACHINE_DEBUG_STOP_BREAKPOINT,
    VM_MACHINE_DEBUG_STOP_TRACE,
    VM_MACHINE_DEBUG_STOP_WATCHPOINT
} vm_machine_debug_stop_reason;

typedef struct {
    x86_debug_execution_plan_kind kind;
    lib_u64 remaining;
    lib_u64 executed;
    lib_u32 breakpoint_linear;
    lib_u8 completion_pending;
    vm_machine_debug_stop_reason completion_reason;
    lib_u64 completion_executed;
} t_debug_execution_plan;

typedef struct {
    t_debug_execution_plan plan;
    core_machine_debug_instruction_observation observation;
    lib_u8 observation_valid;
} t_debug;

void vm_machine_debug_initialize(t_debug *debug);
void vm_machine_debug_reset(t_debug *debug);
void vm_machine_debug_refresh(t_debug *debug,
    const core_machine_debug_instruction_observation *observation);
void vm_machine_debug_finalize(t_debug *debug);
lib_status vm_machine_debug_set_execution_plan(t_debug *debug,
    const x86_debug_request *request);
void vm_machine_debug_clear_execution_plan(t_debug *debug);
lib_u64 vm_machine_debug_limit_instruction_budget(
    const t_debug *debug, lib_u64 requested);
lib_i32 vm_machine_debug_breakpoint_due(const t_debug *debug);
void vm_machine_debug_complete_breakpoint(t_debug *debug);
void vm_machine_debug_complete_watchpoint(t_debug *debug);
void vm_machine_debug_complete_run(t_debug *debug,
    lib_u64 executed);
lib_i32 vm_machine_debug_completion_pending(const t_debug *debug,
    vm_machine_debug_stop_reason *out_reason);
lib_i32 vm_machine_debug_take_completion(t_debug *debug,
    vm_machine_debug_stop_reason *out_reason, lib_u64 *out_executed);

#ifdef __cplusplus
}/*_EOCD_*/
#endif

#endif
