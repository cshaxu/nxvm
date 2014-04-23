/* This file is a part of NXVM project. */

// Global Variables and Virtual Machine Assembly

#ifndef NXVM_VMACHINE_H
#define NXVM_VMACHINE_H

#include "vglobal.h"
#include "vcpu.h"
#include "vram.h"
#include "vpic.h"
#include "vcmos.h"
#include "vpit.h"

extern t_bool vmachineinitflag;
extern t_bool vmachinerunflag;

void NXVMInit();
void NXVMPowerOn();
void NXVMRun();
void NXVMPowerOff();
void NXVMTerm();

#endif