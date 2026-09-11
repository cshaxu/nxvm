/* Copyright 2012-2014 Neko. */

#ifndef VM_MACHINE_DEBUG_H
#define VM_MACHINE_DEBUG_H

#ifdef __cplusplus
extern "C" {
#endif

#include "type.h"
#include "core/machine/debug_interface.h"
#include "vm/machine/runtime/control.h"
#include "vm/machine/runtime/machine_interface.h"

#define VM_MACHINE_DEVICE_DEBUG "Unknown Hardware Debugger"

typedef struct {
    vm_machine_debug_observer observer;
    C_VOID *observer_context;
} t_debug_connect;

typedef struct {
    common_machine_debug_execution_plan_kind kind;
    type_unsigned_64 remaining;
    type_unsigned_64 executed;
    type_unsigned_32 breakpoint_linear;
    type_bool completion_pending;
    vm_machine_pause_reason completion_reason;
    type_unsigned_64 completion_executed;
} t_debug_execution_plan;

typedef struct {
    t_debug_execution_plan plan;
    core_machine_debug_instruction_observation observation;
    type_bool observation_valid;
    t_debug_connect connect;
} t_debug;

C_VOID vm_machine_debug_initialize(t_debug *debug);
C_VOID vm_machine_debug_reset(t_debug *debug);
C_VOID vm_machine_debug_refresh(t_debug *debug,
    const core_machine_debug_instruction_observation *observation);
C_VOID vm_machine_debug_finalize(t_debug *debug);
C_VOID vm_machine_debug_bind_observer(t_debug *debug,
    vm_machine_debug_observer observer, void *context);
type_status vm_machine_debug_set_execution_plan(t_debug *debug,
    const common_machine_debug_request *request);
C_VOID vm_machine_debug_clear_execution_plan(t_debug *debug);
type_unsigned_64 vm_machine_debug_limit_instruction_budget(
    const t_debug *debug, type_unsigned_64 requested);
C_INT vm_machine_debug_breakpoint_due(const t_debug *debug);
C_VOID vm_machine_debug_complete_breakpoint(t_debug *debug);
C_VOID vm_machine_debug_complete_run(t_debug *debug,
    type_unsigned_64 executed);
C_INT vm_machine_debug_completion_pending(const t_debug *debug,
    vm_machine_pause_reason *out_reason);
C_INT vm_machine_debug_take_completion(t_debug *debug,
    vm_machine_pause_reason *out_reason, type_unsigned_64 *out_executed);

#ifdef __cplusplus
}/*_EOCD_*/
#endif

#endif
