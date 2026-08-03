/* Copyright 2012-2014 Neko. */

#ifndef NXVM_PLATFORM_H
#define NXVM_PLATFORM_H

#ifdef __cplusplus
extern "C" {
#endif

#include "type.h"
#include "vm/platform/execution.h"
#include "vm/platform/input.h"
#include "vm/platform/presentation_mailbox.h"

typedef struct vm_platform_run_context {
    const vm_platform_execution_transport *execution;
    const vm_platform_keyboard_transport *keyboard;
    const vm_platform_presentation_mailbox *presentation;
    void *host_console_output;
    void *host_window;
    int window_display;
    vm_platform_keyboard_state_sink keyboard_state_sink;
    void *keyboard_state_context;
} vm_platform_run_context;

void vm_platform_run_context_initialize(
    vm_platform_run_context *context,
    const vm_platform_execution_transport *execution,
    const vm_platform_keyboard_transport *keyboard,
    const vm_platform_presentation_mailbox *presentation);
void vm_platform_run_context_bind_keyboard_state(
    vm_platform_run_context *context, vm_platform_keyboard_state_sink sink,
    void *sink_context);
int vm_platform_run_context_get_window_display(
    const vm_platform_run_context *context);
void vm_platform_run_context_set_window_display(
    vm_platform_run_context *context, int enabled);

/* Device Operations */
void platformDisplaySetScreen(const vm_platform_run_context *context);
void platformDisplayPaint(const vm_platform_run_context *context);

void platformStart(const vm_platform_run_context *context);

void platformInit();
void platformFinal();

#ifdef __cplusplus
}/*_EOCD_*/
#endif

#endif
