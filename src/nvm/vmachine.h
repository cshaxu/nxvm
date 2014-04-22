/* This file is a part of NekoVMac project. */

#ifndef NVM_VMACHINE_H
#define NVM_VMACHINE_H

#include "vglobal.h"

extern t_nubit32 memorySize;
extern t_vaddrcc memoryBase;
extern t_faddrcc InTable[0x10000];
extern t_faddrcc OutTable[0x10000];
extern t_faddrcc InsTable[0x100];
extern t_bool initFlag;
extern t_bool runFlag;

void NVMInit();
void NVMPowerOn();
void NVMRun();
void NVMPowerOff();
void NVMTerm();

#endif