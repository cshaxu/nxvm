/* Copyright 2012-2014 Neko. */

#ifndef NXVM_WIN32APP_H
#define NXVM_WIN32APP_H

#ifdef __cplusplus
extern "C" {
#endif

#include <windows.h>

typedef struct vm_platform_run_context vm_platform_run_context;

VOID vm_platform_win32app_display_set_screen(const vm_platform_run_context *context);
VOID vm_platform_win32app_display_paint(const vm_platform_run_context *context);

VOID vm_platform_win32app_start_machine(const vm_platform_run_context *context);

#ifdef __cplusplus
}/*_EOCD_*/
#endif

#endif
