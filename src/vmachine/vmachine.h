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
#include "vkbc.h"
#include "vkeyb.h"

typedef struct {
	t_bool flaginit;
	t_bool flagrun;
} t_machine;

extern t_machine vmachine;

void vmachineRefresh();
void vmachineInit();
void vmachineRunLoop();
void vmachineFinal();

#endif
