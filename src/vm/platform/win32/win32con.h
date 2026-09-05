/* Copyright 2012-2014 Neko. */

#ifndef VM_PLATFORM_WIN32CON_H
#define VM_PLATFORM_WIN32CON_H

#ifdef __cplusplus
extern "C" {
#endif

#include "type.h"
#include "core/platform/win32/keyboard.h"
#include "vm/platform/win32/win32_types.h"

typedef struct vm_platform_run_context vm_platform_run_context;
typedef struct vm_platform_run_handle vm_platform_run_handle;

type_unsigned_16 vm_platform_win32con_decode_scan_code(type_unsigned_16 raw_scan_code,
    DWORD control_key_state);
C_VOID vm_platform_win32con_submit_input_record(
    const vm_platform_run_context *context, vm_platform_run_handle *owner,
    core_platform_win32_keyboard_normalizer *normalizer, const INPUT_RECORD *input);

C_VOID vm_platform_win32con_display_set_screen(const vm_platform_run_context *context);
C_VOID vm_platform_win32con_display_paint(const vm_platform_run_context *context);
type_status vm_platform_win32con_run_handle_start(
    const vm_platform_run_context *context, vm_platform_run_handle *handle);
C_VOID vm_platform_win32con_run_handle_request_stop(vm_platform_run_handle *handle);
C_VOID vm_platform_win32con_run_handle_join(vm_platform_run_handle *handle);
C_VOID vm_platform_win32con_run_handle_finalize(vm_platform_run_handle *handle);

#ifdef __cplusplus
}/*_EOCD_*/
#endif

#endif
