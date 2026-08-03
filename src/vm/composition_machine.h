/* Copyright 2012-2014 Neko. */

#ifndef NXVM_COMPOSITION_MACHINE_H
#define NXVM_COMPOSITION_MACHINE_H

#ifdef __cplusplus
extern "C" {
#endif

#include "vm/composition_live_machine.h"

void machineStart(vm_composition_live_machine *machine);
void machineReset(vm_composition_live_machine *machine);
void machineStop(vm_composition_live_machine *machine);
void machineResume(vm_composition_live_machine *machine);

void machineInit(vm_composition_live_machine *machine);
void machineFinal(vm_composition_live_machine *machine);

int vm_composition_bind_execution_provider(vm_composition_live_machine *machine);

#ifdef __cplusplus
}/*_EOCD_*/
#endif

#endif
