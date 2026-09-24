/* Copyright 2012-2014 Neko. */

#ifndef VM_MACHINE_LIFECYCLE_H
#define VM_MACHINE_LIFECYCLE_H

#ifdef __cplusplus
extern "C" {
#endif
#include "lib/types/types_interface.h"



#include "app-nxvm/machine/machine_interface.h"
#include "app-nxvm/machine/control.h"
#include "app-nxvm/machine/executor_state.h"

lib_status vm_machine_reset(vm_machine *machine);
lib_status vm_machine_finish_reset(vm_machine *machine, lib_status status);
void vm_machine_stop(vm_machine *machine);
lib_status vm_machine_resume(vm_machine *machine);
lib_status vm_machine_request_pause(vm_machine *machine);

lib_status vm_machine_initialize(vm_machine *machine);
void vm_machine_finalize(vm_machine *machine);

lib_status vm_machine_bind_execution_provider(vm_machine *machine);

#ifdef __cplusplus
}/*_EOCD_*/
#endif

#endif
