/* Copyright 2012-2014 Neko. */

#ifndef NXVM_LINUXCON_H
#define NXVM_LINUXCON_H

#ifdef __cplusplus
/*extern "C" {*/
#endif

typedef struct vm_platform_run_context vm_platform_run_context;

void lnxcDisplaySetScreen();
void lnxcDisplayPaint();
void lnxcStartMachine(const vm_platform_run_context *context);

#ifdef __cplusplus
/*}_EOCD_*/
#endif

#endif
