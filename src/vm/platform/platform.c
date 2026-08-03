/* Copyright 2012-2014 Neko. */

/* PLATFORM is the hub that connects all platform-specific modules */

#include "core/product/utils.h"

#include "core/platform/display_frame.h"
#include "vm/platform/platform.h"

t_platform platform;

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
    context->keyboard_state_sink = NULL;
    context->keyboard_state_context = NULL;
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
    win32DisplaySetScreen(platform.flagMode, context);
}
void platformDisplayPaint(const vm_platform_run_context *context) {
    win32DisplayPaint(platform.flagMode, context);
}
void platformStart(const vm_platform_run_context *context) {
    win32StartMachine(platform.flagMode, context);
}
#elif GLOBAL_PLATFORM == GLOBAL_VAR_LINUX
#include "vm/platform/linux/linux.h"
void platformDisplaySetScreen(const vm_platform_run_context *context) {
    (void)context;
    linuxDisplaySetScreen(platform.flagMode, context);
}
void platformDisplayPaint(const vm_platform_run_context *context) {
    (void)context;
    linuxDisplayPaint(platform.flagMode, context);
}
void platformStart(const vm_platform_run_context *context) {
    linuxStartMachine(platform.flagMode, context);
}
#endif

void platformInit() {
    MEMSET((void *)(&platform), 0x00, sizeof(t_platform));
    platform.flagMode = 0;
}

void platformFinal() {}
