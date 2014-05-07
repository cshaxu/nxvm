/* Copyright 2012-2014 Neko. */

#ifndef NXVM_BOCHSAPI_H
#define NXVM_BOCHSAPI_H

#include "../src/device/vglobal.h"

#define BOCHSAPI_BOCHS_BIOS "d:/nxvm/bochx/bochs-2.6/bios"

#ifdef __cplusplus
extern "C" {
#endif

/* reads from physical address of bochs ram
 * called by nxvm::bochx::vcpuapi.c::vramReadPhysical */
void bochsApiRamReadPhysical(t_nubit32 physical, t_vaddrcc rdata, t_nubit8 byte);
/* writes to physical address of bochs ram
 * called by nxvm::bochx::vcpuapi.c::vramWritePhysical */
void bochsApiRamWritePhysical(t_nubit32 physical, t_vaddrcc rdata, t_nubit8 byte);
/* stops bochs cpu_loop
 * called by nxvm::bochx::vcpuapi.c::deviceStop */
void bochsApiDeviceStop();

#ifdef __cplusplus
}/*_EOCD_*/
#endif

/* initializes nxvm cpu
 * called by bochs::cpu.cc::cpu_loop */
void bochsApiInit();
/* finalizes nxvm cpu
 * called by bochs::cpu.cc::cpu_loop */
void bochsApiFinal();
/* copies bochs cpu to nxvm cpu and executes nxvm cpu
 * called by bochs::cpu.cc::cpu_loop */
void bochsApiExecBefore();
/* compare bochs cpu with nxvm cpu
 * called by bochs::cpu.cc::cpu_loop */
void bochsApiExecAfter();
/* records linear address accessing of bochs ram
 * called by bochs::paging.cc::access_(read/write)_linear */
void bochsApiRecordRam(t_nubit32 linear, t_vaddrcc rdata, t_nubit8 byte,
                       t_bool flagWrite);

#endif
