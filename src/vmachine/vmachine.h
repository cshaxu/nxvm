/* This file is a part of NVMx86 project. */

#ifndef NVM_VMACHINE_H
#define NVM_VMACHINE_H

#include "vglobal.h"
#include "vcpu.h"
#include "vmemory.h"

extern t_bool initFlag;
extern t_bool runFlag;

void NVMInit();
void NVMPowerOn();
void NVMRun();
void NVMPowerOff();
void NVMTerm();

#endif