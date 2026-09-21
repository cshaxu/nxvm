/* Copyright 2012-2014 Neko. */

#ifndef VM_MACHINE_LIFECYCLE_H
#define VM_MACHINE_LIFECYCLE_H

#ifdef __cplusplus
extern "C" {
#endif

#include "type.h"


#include "core/machine/machine_interface.h"
#include "core/machine/control.h"
#include "core/machine/executor_state.h"

type_status vm_machine_start(vm_machine *machine);
type_status vm_machine_reset(vm_machine *machine);
type_status vm_machine_finish_reset(vm_machine *machine, type_status status);
C_VOID vm_machine_stop(vm_machine *machine);
type_status vm_machine_resume(vm_machine *machine);
type_status vm_machine_request_pause(vm_machine *machine);
type_status vm_machine_request_pause_reason(vm_machine *machine,
    vm_machine_pause_reason reason);
type_status vm_machine_request_step(vm_machine *machine);

type_status vm_machine_initialize(vm_machine *machine);
C_VOID vm_machine_finalize(vm_machine *machine);

type_status vm_machine_bind_execution_provider(vm_machine *machine);

#ifdef __cplusplus
}/*_EOCD_*/
#endif

#endif
