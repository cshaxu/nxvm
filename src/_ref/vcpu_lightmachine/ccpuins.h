
#ifndef NXVM_CCPU_H
#define NXVM_CCPU_H

#include "../vglobal.h"
#include "../vcpuins.h"

extern t_cpuins ccpuins;

void ccpuinsExecIns();
void ccpuinsExecInt(t_nubit8 intid);

void ccpuinsInit();
void ccpuinsFinal();

#endif