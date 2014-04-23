/* This file is a part of NXVM project. */

#ifndef NXVM_VMACHINE_H
#define NXVM_VMACHINE_H

#include "vglobal.h"
#include "vcpu.h"
#include "vmemory.h"

extern t_bool initFlag;
extern t_bool runFlag;

void NXVMInit();
void NXVMPowerOn();
void NXVMRun();
void NXVMPowerOff();
void NXVMTerm();

#endif