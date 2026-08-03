/* Copyright 2012-2014 Neko. */

#ifndef NXVM_COMPOSITION_MACHINE_H
#define NXVM_COMPOSITION_MACHINE_H

#ifdef __cplusplus
extern "C" {
#endif

#include "type.h"

#include "vm/composition/composition_live_machine.h"

C_VOID vm_composition_start(vm_composition_live_machine *machine);
C_VOID vm_composition_reset(vm_composition_live_machine *machine);
C_VOID vm_composition_stop(vm_composition_live_machine *machine);
C_VOID vm_composition_resume(vm_composition_live_machine *machine);

C_VOID vm_composition_initialize(vm_composition_live_machine *machine);
C_VOID vm_composition_finalize(vm_composition_live_machine *machine);

C_INT vm_composition_bind_execution_provider(vm_composition_live_machine *machine);

#ifdef __cplusplus
}/*_EOCD_*/
#endif

#endif
