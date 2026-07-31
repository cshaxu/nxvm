/* Copyright 2012-2014 Neko. */

#ifndef NXVM_VMACHINE_H
#define NXVM_VMACHINE_H

#ifdef __cplusplus
extern "C" {
#endif

#include "vglobal.h"

#define NXVM_DEVICE_MACHINE "IBM PC/AT"

void vmachineInit();
void vmachineReset();
void vmachineRefresh();
void vmachineFinal();

#ifdef __cplusplus
}/*_EOCD_*/
#endif

#endif
