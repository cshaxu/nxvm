/* Copyright 2012-2014 Neko. */

#ifndef NXVM_VM_COMPOSITION_H
#define NXVM_VM_COMPOSITION_H

#ifdef __cplusplus
extern "C" {
#endif

#include "type.h"
#include "vm/composition/composition_live_machine.h"

#define VM_COMPOSITION_MACHINE_NAME "IBM PC/AT"

C_VOID vm_composition_providers_initialize(vm_composition_live_machine *machine);
C_VOID vm_composition_providers_refresh(vm_composition_live_machine *machine);
C_VOID vm_composition_providers_reset(vm_composition_live_machine *machine);
C_VOID vm_composition_providers_finalize(vm_composition_live_machine *machine);
C_VOID vm_composition_print_machine(const vm_composition_live_machine *machine);

#ifdef __cplusplus
}/*_EOCD_*/
#endif

#endif
