/* Copyright 2012-2014 Neko. */

#ifndef NXVM_LINUX_H
#define NXVM_LINUX_H

#ifdef __cplusplus
/*extern "C" {*/
#endif

#include "type.h"

typedef struct vm_platform_run_context vm_platform_run_context;
typedef struct vm_platform_run_handle vm_platform_run_handle;

C_VOID vm_platform_linux_sleep(uint32_t milisec);
C_VOID vm_platform_linux_display_set_screen(C_INT window, const vm_platform_run_context *context);
C_VOID vm_platform_linux_display_paint(C_INT window, const vm_platform_run_context *context);
C_VOID vm_platform_linux_start_machine(C_INT window, const vm_platform_run_context *context);
ntvdm64_status vm_platform_linux_run_handle_start(
    const vm_platform_run_context *context, vm_platform_run_handle *handle);
C_VOID vm_platform_linux_run_handle_request_stop(vm_platform_run_handle *handle);
C_VOID vm_platform_linux_run_handle_join(vm_platform_run_handle *handle);
C_VOID vm_platform_linux_run_handle_finalize(vm_platform_run_handle *handle);

#ifdef __cplusplus
/*}_EOCD_*/
#endif

#endif
