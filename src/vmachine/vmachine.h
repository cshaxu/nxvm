/* This file is a part of NXVM project. */

// Global Variables and Virtual Machine Assembly

#ifndef NXVM_VMACHINE_H
#define NXVM_VMACHINE_H

#include "vglobal.h"
#include "vcpu.h"
#include "vram.h"
#include "vpic.h"
#include "vpit.h"
#include "vcmos.h"
#include "vdma.h"
#include "vfdc.h"
#include "vfdd.h"

extern t_bool vmachineinitflag;
extern t_bool vmachinerunflag;

void vmachineRefresh();

void vmachineInit();
void vmachinePowerOn();
void vmachineRunLoop();
void vmachinePowerOff();
void vmachineFinal();

#endif
