/* Copyright 2012-2014 Neko. */

#ifndef VM_PLATFORM_WIN32APP_H
#define VM_PLATFORM_WIN32APP_H

#ifdef __cplusplus
extern "C" {
#endif

#include "type.h"
#include "vm/platform/win32/win32_types.h"

typedef struct vm_platform_run_context vm_platform_run_context;
typedef struct vm_platform_run_handle vm_platform_run_handle;
typedef struct vm_platform_win32_window_presenter
    vm_platform_win32_window_presenter;

type_unsigned_16 vm_platform_win32app_decode_scan_code(LPARAM l_param);

type_status vm_platform_win32_window_presenter_start(
    const vm_platform_run_context *context, vm_platform_run_handle *owner,
    C_INT wait_for_execution_start,
    vm_platform_win32_window_presenter **out_presenter);
C_VOID vm_platform_win32_window_presenter_request_stop(
    vm_platform_win32_window_presenter *presenter);
C_VOID vm_platform_win32_window_presenter_join(
    vm_platform_win32_window_presenter *presenter);
C_VOID vm_platform_win32_window_presenter_destroy(
    vm_platform_win32_window_presenter *presenter);

C_VOID vm_platform_win32app_display_set_screen(const vm_platform_run_context *context);
C_VOID vm_platform_win32app_display_paint(const vm_platform_run_context *context);
type_status vm_platform_win32app_run_handle_start(
    const vm_platform_run_context *context, vm_platform_run_handle *handle);
C_VOID vm_platform_win32app_run_handle_request_stop(vm_platform_run_handle *handle);
C_VOID vm_platform_win32app_run_handle_join(vm_platform_run_handle *handle);
C_VOID vm_platform_win32app_run_handle_finalize(vm_platform_run_handle *handle);

#ifdef __cplusplus
}/*_EOCD_*/
#endif

#endif
