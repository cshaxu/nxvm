/* Copyright 2012-2014 Neko. */

#ifndef NXVM_VMACHINE_H
#define NXVM_VMACHINE_H

#ifdef __cplusplus
extern "C" {
#endif

#include "core/machine/vglobal.h"
#include "vm/composition_live_machine.h"

#define NXVM_DEVICE_MACHINE "IBM PC/AT"

void vmachineInit(vm_composition_live_machine *machine);
void vmachineReset(vm_composition_live_machine *machine);
void vmachineRefresh(vm_composition_live_machine *machine);
void vmachineFinal(vm_composition_live_machine *machine);
void devicePrintMachine();

#ifdef __cplusplus
}/*_EOCD_*/
#endif

#endif
