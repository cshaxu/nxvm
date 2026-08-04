/* Copyright 2012-2014 Neko. */

/* PLATFORM is the hub that connects all platform-specific modules */

#include "type.h"

#include "core/product/utils.h"


#include "core/platform/display_frame.h"

#include "vm/platform/platform.h"

C_VOID vm_platform_run_context_initialize(
    vm_platform_run_context *context,
    const vm_platform_execution_transport *execution,
    const vm_platform_keyboard_transport *keyboard,
    const vm_platform_presentation_mailbox *presentation,
    const core_product_wait_scope *wait_scope)
{
    if (context == STD_NULL) return;
    context->execution = execution;
    context->keyboard = keyboard;
    context->presentation = presentation;
    context->wait_scope = wait_scope;
    core_platform_host_surface_context_initialize(&context->console_surface,
        CORE_PLATFORM_HOST_SURFACE_CONSOLE, STD_NULL);
    core_platform_host_surface_context_initialize(&context->window_surface,
        CORE_PLATFORM_HOST_SURFACE_WINDOW, STD_NULL);
    context->console_renderer = STD_NULL;
    context->window_renderer = STD_NULL;
    context->terminal_displayed_generation = 0u;
    context->window_display = 0;
    context->keyboard_state_sink = STD_NULL;
    context->keyboard_state_context = STD_NULL;
}

C_INT vm_platform_run_context_get_window_display(
    const vm_platform_run_context *context)
{
    return context != STD_NULL && context->window_display != 0;
}

C_VOID vm_platform_run_context_set_window_display(
    vm_platform_run_context *context, C_INT enabled)
{
    if (context != STD_NULL) context->window_display = enabled != 0;
}

C_VOID vm_platform_run_context_bind_keyboard_state(
    vm_platform_run_context *context, vm_platform_keyboard_state_sink sink,
    C_VOID *sink_context)
{
    if (context == STD_NULL) return;
    context->keyboard_state_sink = sink;
    context->keyboard_state_context = sink_context;
}

C_VOID vm_platform_run_handle_initialize(vm_platform_run_handle *handle)
{
    if (handle != STD_NULL) STD_MEMSET(handle, 0, sizeof(*handle));
}

C_INT vm_platform_run_handle_is_active(const vm_platform_run_handle *handle)
{
    return handle != STD_NULL && handle->active;
}

C_INT vm_platform_run_handle_is_window_display(
    const vm_platform_run_handle *handle)
{
    return handle != STD_NULL && handle->window_display;
}

#if GLOBAL_PLATFORM == GLOBAL_VAR_WIN32

#include "vm/platform/win32/win32.h"
C_VOID vm_platform_display_set_screen(const vm_platform_run_context *context) {
    vm_platform_win32_display_set_screen(vm_platform_run_context_get_window_display(context), context);
}
C_VOID vm_platform_display_paint(const vm_platform_run_context *context) {
    vm_platform_win32_display_paint(vm_platform_run_context_get_window_display(context), context);
}
type_status vm_platform_start(const vm_platform_run_context *context,
    vm_platform_run_handle *handle) {
    return vm_platform_win32_run_handle_start(context, handle);
}
C_VOID vm_platform_run_handle_request_stop(vm_platform_run_handle *handle) {
    vm_platform_win32_run_handle_request_stop(handle);
}
C_VOID vm_platform_run_handle_join(vm_platform_run_handle *handle) {
    vm_platform_win32_run_handle_join(handle);
}
C_VOID vm_platform_run_handle_finalize(vm_platform_run_handle *handle) {
    vm_platform_win32_run_handle_finalize(handle);
}
#elif GLOBAL_PLATFORM == GLOBAL_VAR_LINUX

#include "vm/platform/linux/linux.h"
C_VOID vm_platform_display_set_screen(const vm_platform_run_context *context) {
    (C_VOID)context;
    vm_platform_linux_display_set_screen(vm_platform_run_context_get_window_display(context), context);
}
C_VOID vm_platform_display_paint(const vm_platform_run_context *context) {
    (C_VOID)context;
    vm_platform_linux_display_paint(vm_platform_run_context_get_window_display(context), context);
}
type_status vm_platform_start(const vm_platform_run_context *context,
    vm_platform_run_handle *handle) {
    return vm_platform_linux_run_handle_start(context, handle);
}
C_VOID vm_platform_run_handle_request_stop(vm_platform_run_handle *handle) {
    vm_platform_linux_run_handle_request_stop(handle);
}
C_VOID vm_platform_run_handle_join(vm_platform_run_handle *handle) {
    vm_platform_linux_run_handle_join(handle);
}
C_VOID vm_platform_run_handle_finalize(vm_platform_run_handle *handle) {
    vm_platform_linux_run_handle_finalize(handle);
}
#endif
