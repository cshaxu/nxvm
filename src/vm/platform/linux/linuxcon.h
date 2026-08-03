/* Copyright 2012-2014 Neko. */

#ifndef NXVM_LINUXCON_H
#define NXVM_LINUXCON_H



#include "type.h"
#ifdef __cplusplus
/*extern "C" {*/
#endif

typedef struct vm_platform_run_context vm_platform_run_context;

C_VOID lnxcDisplaySetScreen(const vm_platform_run_context *context);
C_VOID lnxcDisplayPaint(const vm_platform_run_context *context);
C_VOID lnxcStartMachine(const vm_platform_run_context *context);

#ifdef __cplusplus
/*}_EOCD_*/
#endif

#endif
