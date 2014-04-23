/* This file is a part of NXVM project. */

/* CPU App Interface (for Bochs testing) */

#ifndef NXVM_VCPUAPI_H
#define NXVM_VCPUAPI_H

#ifdef __cplusplus
extern "C" {
#endif

#include "stdarg.h"

#include "vcpu.h"

#define MAXLINE 0x100

void vcpuapiPrintReg(t_cpu *rcpu);
void vcpuapiPrintSreg(t_cpu *rcpu);
void vcpuapiPrintCreg(t_cpu *rcpu);
void vcpuapiLoadSreg(t_cpu_sreg *rsreg, t_nubit32 lo, t_nubit32 hi);

void vcpuapiInit();
void vcpuapiFinal();
t_bool vcpuapiExecBefore(t_cpu *rcpu);
t_bool vcpuapiExecAfter(t_cpu *rcpu);

t_nubit64 vcpuapiReadPhysical(t_nubit32 phy, t_nubit8 byte);
void vcpuapiWritePhysical(t_nubit32 phy, t_nubit64 data, t_nubit8 byte);

#ifdef VGLOBAL_BOCHS
t_nubit32 vapiPrint(const t_string format, ...);
void vapiCallBackMachineStop();
void vapiSleep(t_nubit32 milisec);
void vapiCallBackDebugPrintRegs(t_bool bit32);
#define vramSize 2097152
#define vramAddr(phy) 0
#define vramIsAddrInMem(ref) 0

#endif

#ifdef __cplusplus
}/*_EOCD_*/
#endif

#endif
