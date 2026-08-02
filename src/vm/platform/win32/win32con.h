/* Copyright 2012-2014 Neko. */

#ifndef NXVM_WIN32CON_H
#define NXVM_WIN32CON_H

#ifdef __cplusplus
extern "C" {
#endif

#include <windows.h>

typedef struct vm_platform_run_context vm_platform_run_context;

extern HANDLE hOut;

VOID win32conDisplaySetScreen();
VOID win32conDisplayPaint();

VOID win32conStartMachine(const vm_platform_run_context *context);

#ifdef __cplusplus
}/*_EOCD_*/
#endif

#endif
