/* Copyright 2012-2014 Neko. */

#ifndef NXVM_COMPOSITION_MACHINE_H
#define NXVM_COMPOSITION_MACHINE_H

#ifdef __cplusplus
extern "C" {
#endif

#include "vm/composition_live_machine.h"

void machineStart();
void machineReset();
void machineStop();
void machineResume();

void machineInit(vm_composition_live_machine *machine);
void machineFinal(vm_composition_live_machine *machine);

#ifdef __cplusplus
}/*_EOCD_*/
#endif

#endif
