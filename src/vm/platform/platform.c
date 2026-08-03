/* Copyright 2012-2014 Neko. */

/* PLATFORM is the hub that connects all platform-specific modules */

#include "core/product/utils.h"

#include "core/platform/display_frame.h"
#include "vm/platform/platform.h"

void vm_platform_run_context_initialize(
    vm_platform_run_context *context,
    const vm_platform_execution_transport *execution,
    const vm_platform_keyboard_transport *keyboard,
    const vm_platform_presentation_mailbox *presentation)
{
    if (context == NULL) return;
    context->execution = execution;
    context->keyboard = keyboard;
    context->presentation = presentation;
    context->host_console_output = NULL;
    context->host_window = NULL;
    context->window_display = 0;
    context->keyboard_state_sink = NULL;
    context->keyboard_state_context = NULL;
}

int vm_platform_run_context_get_window_display(
    const vm_platform_run_context *context)
{
    return context != NULL && context->window_display != 0;
}

void vm_platform_run_context_set_window_display(
    vm_platform_run_context *context, int enabled)
{
    if (context != NULL) context->window_display = enabled != 0;
}

void vm_platform_run_context_bind_keyboard_state(
    vm_platform_run_context *context, vm_platform_keyboard_state_sink sink,
    void *sink_context)
{
    if (context == NULL) return;
    context->keyboard_state_sink = sink;
    context->keyboard_state_context = sink_context;
}

#if GLOBAL_PLATFORM == GLOBAL_VAR_WIN32
#include "vm/platform/win32/win32.h"
void platformDisplaySetScreen(const vm_platform_run_context *context) {
    win32DisplaySetScreen(vm_platform_run_context_get_window_display(context), context);
}
void platformDisplayPaint(const vm_platform_run_context *context) {
    win32DisplayPaint(vm_platform_run_context_get_window_display(context), context);
}
void platformStart(const vm_platform_run_context *context) {
    win32StartMachine(vm_platform_run_context_get_window_display(context), context);
}
#elif GLOBAL_PLATFORM == GLOBAL_VAR_LINUX
#include "vm/platform/linux/linux.h"
void platformDisplaySetScreen(const vm_platform_run_context *context) {
    (void)context;
    linuxDisplaySetScreen(vm_platform_run_context_get_window_display(context), context);
}
void platformDisplayPaint(const vm_platform_run_context *context) {
    (void)context;
    linuxDisplayPaint(vm_platform_run_context_get_window_display(context), context);
}
void platformStart(const vm_platform_run_context *context) {
    linuxStartMachine(vm_platform_run_context_get_window_display(context), context);
}
#endif

void platformInit() {
}

void platformFinal() {}
