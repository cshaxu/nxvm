/* Copyright 2012-2014 Neko. */

#ifndef NXVM_WIN32CON_H
#define NXVM_WIN32CON_H

#ifdef __cplusplus
extern "C" {
#endif

#include "type.h"

typedef struct vm_platform_run_context vm_platform_run_context;
typedef struct vm_platform_run_handle vm_platform_run_handle;

C_VOID vm_platform_win32con_display_set_screen(const vm_platform_run_context *context);
C_VOID vm_platform_win32con_display_paint(const vm_platform_run_context *context);
ntvdm64_status vm_platform_win32con_run_handle_start(
    const vm_platform_run_context *context, vm_platform_run_handle *handle);
C_VOID vm_platform_win32con_run_handle_request_stop(vm_platform_run_handle *handle);
C_VOID vm_platform_win32con_run_handle_join(vm_platform_run_handle *handle);
C_VOID vm_platform_win32con_run_handle_finalize(vm_platform_run_handle *handle);

#ifdef __cplusplus
}/*_EOCD_*/
#endif

#endif
