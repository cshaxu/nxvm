
#ifndef NXVM_LCPUINS_H
#define NXVM_LCPUINS_H

#include "vglobal.h"
#include "lcpuins/CentreProcessorUnit.h"

#define lcpu   (*pcpu)
extern CentreProcessorUnit *pcpu;

void lcpuinsDebugPrintlregs();

void lcpuinsSyncRegs();

void lcpuinsExecIns();
void vcpuinsExecInt();

void lcpuinsInit();
void lcpuinsFinal();


#endif
