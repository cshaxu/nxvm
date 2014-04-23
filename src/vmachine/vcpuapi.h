/* This file is a part of NXVM project. */

/* CPU App Interface (for Bochs testing) */

#ifndef NXVM_VCPUAPI_H
#define NXVM_VCPUAPI_H

#ifdef __cplusplus
extern "C" {
#endif

#include "vcpu.h"

#ifdef VGLOBAL_BOCHS
#define vramIsAddrInMem(ref) 0
void vcpuapiLoadSreg(t_cpu_sreg *rsreg, t_nubit32 lo, t_nubit32 hi);
void vcpuapiReadPhysical(t_nubit32 physical, t_vaddrcc rdata, t_nubit8 byte);
void vcpuapiWritePhysical(t_nubit32 physical, t_vaddrcc rdata, t_nubit8 byte);
void vcpuapiMemRec(t_nubit32 linear, t_vaddrcc rdata, t_nubit8 byte, t_bool write);
void vcpuapiInit();
void vcpuapiFinal();
void vcpuapiExecBefore();
void vcpuapiExecAfter();
#endif

#ifdef __cplusplus
}/*_EOCD_*/
#endif

#endif
