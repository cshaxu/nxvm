/* Copyright 2012-2014 Neko. */

#ifndef VM_PLATFORM_WIN32_H
#define VM_PLATFORM_WIN32_H

#ifdef __cplusplus
extern "C" {
#endif

#include "type.h"
#include "core/platform/win32/keyboard.h"
#include "vm/platform/platform.h"
#include "vm/platform/win32/win32_types.h"

C_VOID vm_platform_win32_keyboard_make_key_for(const vm_platform_run_context *context,
    vm_platform_run_handle *owner, type_unsigned_16 scanCode, type_unsigned_16 virtualKey,
    C_INT pressed);
C_VOID vm_platform_win32_keyboard_make_character_for(const vm_platform_run_context *context,
    type_unsigned_32 scalar);
C_VOID vm_platform_win32_keyboard_make_utf16_for(
    core_platform_win32_keyboard_normalizer *state, const vm_platform_run_context *context,
    type_unsigned_16 code_unit);
C_VOID vm_platform_win32_mouse_relative_for(const vm_platform_run_context *context,
    type_signed_16 delta_x, type_signed_16 delta_y, type_unsigned_8 buttons);

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
