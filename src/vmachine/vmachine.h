/* This file is a part of NXVM project. */

// Global Variables and Virtual Machine Assembly

#ifndef NXVM_VMACHINE_H
#define NXVM_VMACHINE_H

#include "vglobal.h"
#include "vcpu.h"
#include "vmemory.h"
#include "vpic.h"

extern t_bool initFlag;
extern t_bool runFlag;

void NXVMInit();
void NXVMPowerOn();
void NXVMRun();
void NXVMPowerOff();
void NXVMTerm();

#endif