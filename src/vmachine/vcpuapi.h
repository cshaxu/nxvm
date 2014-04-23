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

#ifdef VGLOBAL_BOCHS
void vapiCallBackMachineStop();
void vapiSleep(t_nubit32 milisec);
void vapiCallBackDebugPrintRegs(t_bool bit32);
#define vramIsAddrInMem(ref) 0
void vcpuapiLoadSreg(t_cpu_sreg *rsreg, t_nubit32 lo, t_nubit32 hi);
t_nubit64 vcpuapiReadPhysical(t_nubit32 phy, t_nubit8 byte);
void vcpuapiWritePhysical(t_nubit32 phy, t_nubit64 data, t_nubit8 byte);
#endif

t_nubit32 vcpuapiPrint(const t_string format, ...);
void vcpuapiPrintReg(t_cpu *rcpu);
void vcpuapiPrintSreg(t_cpu *rcpu);
void vcpuapiPrintCreg(t_cpu *rcpu);

void vcpuapiInit();
void vcpuapiFinal();
void vcpuapiExecBefore();
void vcpuapiExecAfter();

#ifdef __cplusplus
}/*_EOCD_*/
#endif

#endif
