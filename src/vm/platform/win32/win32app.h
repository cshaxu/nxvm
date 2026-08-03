/* Copyright 2012-2014 Neko. */

#ifndef NXVM_WIN32APP_H
#define NXVM_WIN32APP_H

#ifdef __cplusplus
extern "C" {
#endif

#include <windows.h>

typedef struct vm_platform_run_context vm_platform_run_context;

extern HWND w32aHWnd;

VOID win32appDisplaySetScreen(const vm_platform_run_context *context);
VOID win32appDisplayPaint(const vm_platform_run_context *context);

VOID win32appStartMachine(const vm_platform_run_context *context);

#ifdef __cplusplus
}/*_EOCD_*/
#endif

#endif
