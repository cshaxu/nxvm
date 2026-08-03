/* Copyright 2012-2014 Neko. */

#ifndef NXVM_WIN32_H
#define NXVM_WIN32_H

#ifdef __cplusplus
extern "C" {
#endif

#include <windows.h>

#include "core/machine/keyboard_interface.h"
#include "type.h"
#include "vm/platform/platform.h"

VOID vm_platform_win32_keyboard_make_status_for(const vm_platform_run_context *context);
VOID vm_platform_win32_keyboard_make_key_for(const vm_platform_run_context *context,
                             UCHAR scanCode, UCHAR virtualKey);

#define vm_platform_win32_sleep Sleep
VOID vm_platform_win32_display_set_screen(BOOL flagWindow,
                           const vm_platform_run_context *context);
VOID vm_platform_win32_display_paint(BOOL flagWindow,
                       const vm_platform_run_context *context);
VOID vm_platform_win32_start_machine(BOOL flagWindow,
                       const vm_platform_run_context *context);

#ifdef __cplusplus
}/*_EOCD_*/
#endif

#endif
