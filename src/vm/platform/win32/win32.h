/* Copyright 2012-2014 Neko. */

#ifndef NXVM_WIN32_H
#define NXVM_WIN32_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <windows.h>

#include "core/machine/keyboard_interface.h"
#include "type.h"
#include "vm/platform/platform.h"

typedef vm_platform_keyboard_state_sink nxvm_win32_keyboard_state_sink;

VOID win32KeyboardMakeStatus();
VOID win32KeyboardMakeKey(UCHAR scanCode, UCHAR virtualKey);
VOID win32KeyboardMakeStatusFor(const vm_platform_run_context *context);
VOID win32KeyboardMakeKeyFor(const vm_platform_run_context *context,
                             UCHAR scanCode, UCHAR virtualKey);
void win32KeyboardBindStateSink(nxvm_win32_keyboard_state_sink sink,
                                void *opaque);

#define win32Sleep Sleep
VOID win32DisplaySetScreen(BOOL flagWindow,
                           const vm_platform_run_context *context);
VOID win32DisplayPaint(BOOL flagWindow,
                       const vm_platform_run_context *context);
VOID win32StartMachine(BOOL flagWindow,
                       const vm_platform_run_context *context);

#ifdef __cplusplus
}/*_EOCD_*/
#endif

#endif
