/* Copyright 2012-2014 Neko. */

#ifndef NXVM_WIN32CON_H
#define NXVM_WIN32CON_H

#ifdef __cplusplus
extern "C" {
#endif

#include <windows.h>

typedef struct vm_platform_run_context vm_platform_run_context;

VOID win32conDisplaySetScreen(const vm_platform_run_context *context);
VOID win32conDisplayPaint(const vm_platform_run_context *context);

VOID win32conStartMachine(const vm_platform_run_context *context);

#ifdef __cplusplus
}/*_EOCD_*/
#endif

#endif
