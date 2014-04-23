
#ifndef NXVM_ECPUAPI_H
#define NXVM_ECPUAPI_H

#include "../vglobal.h"

void ecpuapiSyncRegs();
t_bool ecpuapiHasDiff();
t_bool ecpuapiScanINTR();
t_nubit8 ecpuapiGetINTR();
void ecpuapiPrintRegs();

#endif
