/* Copyright 2012-2014 Neko. */

#ifndef VM_PLATFORM_WIN32_H
#define VM_PLATFORM_WIN32_H

#ifdef __cplusplus
extern "C" {
#endif

#include "type.h"
#include "vm/platform/platform.h"
#include "vm/platform/win32/win32_types.h"

C_VOID vm_platform_win32_keyboard_make_key_for(const vm_platform_run_context *context,
    vm_platform_run_handle *owner, uint16_t scanCode, uint16_t virtualKey,
    C_INT pressed);
C_VOID vm_platform_win32_mouse_relative_for(const vm_platform_run_context *context,
    int16_t delta_x, int16_t delta_y, uint8_t buttons);

C_VOID vm_platform_win32_display_set_screen(WIN32_BOOL flagWindow,
                           const vm_platform_run_context *context);
C_VOID vm_platform_win32_display_paint(WIN32_BOOL flagWindow,
                       const vm_platform_run_context *context);
type_status vm_platform_win32_run_handle_start(
    const vm_platform_run_context *context, vm_platform_run_handle *handle);
C_VOID vm_platform_win32_run_handle_request_stop(vm_platform_run_handle *handle);
C_VOID vm_platform_win32_run_handle_join(vm_platform_run_handle *handle);
C_VOID vm_platform_win32_run_handle_finalize(vm_platform_run_handle *handle);

#ifdef __cplusplus
}/*_EOCD_*/
#endif

#endif
