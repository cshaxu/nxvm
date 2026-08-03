/* Copyright 2012-2014 Neko. */

#ifndef NXVM_LINUXCON_H
#define NXVM_LINUXCON_H



#include "type.h"
#ifdef __cplusplus
/*extern "C" {*/
#endif

typedef struct vm_platform_run_context vm_platform_run_context;
typedef struct vm_platform_run_handle vm_platform_run_handle;

C_VOID lnxcDisplaySetScreen(const vm_platform_run_context *context);
C_VOID lnxcDisplayPaint(const vm_platform_run_context *context);
ntvdm64_status vm_platform_linuxcon_run_handle_start(
    const vm_platform_run_context *context, vm_platform_run_handle *handle);
C_VOID vm_platform_linuxcon_run_handle_request_stop(vm_platform_run_handle *handle);
C_VOID vm_platform_linuxcon_run_handle_join(vm_platform_run_handle *handle);
C_VOID vm_platform_linuxcon_run_handle_finalize(vm_platform_run_handle *handle);

#ifdef __cplusplus
/*}_EOCD_*/
#endif

#endif
