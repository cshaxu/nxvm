/* Copyright 2012-2014 Neko. */

#ifndef NXVM_WIN32_H
#define NXVM_WIN32_H

#ifdef __cplusplus
extern "C" {
#endif

#include "core/machine/keyboard_interface.h"
#include "type.h"
#include "vm/platform/platform.h"
#include "vm/platform/win32/win32_types.h"

C_VOID vm_platform_win32_keyboard_make_status_for(const vm_platform_run_context *context);
C_VOID vm_platform_win32_keyboard_make_key_for(const vm_platform_run_context *context,
                             C_UCHAR scanCode, C_UCHAR virtualKey);

#define vm_platform_win32_sleep Sleep
C_VOID vm_platform_win32_display_set_screen(WIN32_BOOL flagWindow,
                           const vm_platform_run_context *context);
C_VOID vm_platform_win32_display_paint(WIN32_BOOL flagWindow,
                       const vm_platform_run_context *context);
C_VOID vm_platform_win32_start_machine(WIN32_BOOL flagWindow,
                       const vm_platform_run_context *context);

#ifdef __cplusplus
}/*_EOCD_*/
#endif

#endif
