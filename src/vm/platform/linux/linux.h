/* Copyright 2012-2014 Neko. */

#ifndef NXVM_LINUX_H
#define NXVM_LINUX_H

#ifdef __cplusplus
/*extern "C" {*/
#endif

#include "type.h"

typedef struct vm_platform_run_context vm_platform_run_context;

void linuxSleep(uint32_t milisec);
void linuxDisplaySetScreen(int window, const vm_platform_run_context *context);
void linuxDisplayPaint(int window, const vm_platform_run_context *context);
void linuxStartMachine(int window, const vm_platform_run_context *context);

#ifdef __cplusplus
/*}_EOCD_*/
#endif

#endif
