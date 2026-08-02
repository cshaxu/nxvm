/* Copyright 2012-2014 Neko. */

#ifndef NXVM_PLATFORM_H
#define NXVM_PLATFORM_H

#ifdef __cplusplus
extern "C" {
#endif

#include "type.h"
#include "vm/platform/execution.h"
#include "vm/platform/input.h"

typedef struct {
    int flagMode;  /* true if runs in window, otherwise in console */
} t_platform;

typedef struct vm_platform_run_context {
    const vm_platform_execution_transport *execution;
    const vm_platform_keyboard_transport *keyboard;
    vm_platform_keyboard_state_sink keyboard_state_sink;
    void *keyboard_state_context;
} vm_platform_run_context;

void vm_platform_run_context_initialize(
    vm_platform_run_context *context,
    const vm_platform_execution_transport *execution,
    const vm_platform_keyboard_transport *keyboard);
void vm_platform_run_context_bind_keyboard_state(
    vm_platform_run_context *context, vm_platform_keyboard_state_sink sink,
    void *sink_context);

extern t_platform platform;

/* Device Operations */
void platformDisplaySetScreen();
void platformDisplayPaint();

void platformStart(const vm_platform_run_context *context);

void platformInit();
void platformFinal();

#ifdef __cplusplus
}/*_EOCD_*/
#endif

#endif
