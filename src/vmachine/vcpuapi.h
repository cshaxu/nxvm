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
#define vapiPrint printf
#define FOPEN fopen
#define SPRINTF sprintf
#define STRCPY strcpy
#define STRCAT strcat
void vapiCallBackMachineStop();
void vapiSleep(t_nubit32 milisec);
void vapiCallBackCpuPrintIns();
void vapiCallBackCpuPrintSreg();
void vapiCallBackCpuPrintCreg();
void vapiCallBackCpuPrintReg();
#define vramIsAddrInMem(ref) 0

void vcpuapiLoadSreg(t_cpu_sreg *rsreg, t_nubit32 lo, t_nubit32 hi);
void vcpuapiReadPhysical(t_nubit32 physical, t_vaddrcc rdata, t_nubit8 byte);
void vcpuapiWritePhysical(t_nubit32 physical, t_vaddrcc rdata, t_nubit8 byte);
#endif

t_nubit32 vcpuapiPrint(const t_string format, ...);

void vcpuapiPrintReg(t_cpu *rcpu);
void vcpuapiPrintSreg(t_cpu *rcpu);
void vcpuapiPrintCreg(t_cpu *rcpu);

void vcpuapiInit();
void vcpuapiFinal();
void vcpuapiExecBefore();
void vcpuapiExecAfter();
void vcpuapiMemRec(t_nubit32 linear, t_vaddrcc rdata, t_nubit8 byte, t_bool write);

#ifdef __cplusplus
}/*_EOCD_*/
#endif

#endif
