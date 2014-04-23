
#ifndef NXVM_CCPUAPI_H
#define NXVM_CCPUAPI_H

#include "../vglobal.h"
#include "ccpu.h"

void ccpuapiDebugPrintRegs();
void ccpuapiSyncRegs();
void ccpuapiExecIns();
void ccpuapiExecInt(t_nubit8 intid);
t_bool ccpuapiHasDiff();

void ccpuapiInit();
void ccpuapiFinal();

#endif
