/* Copyright 2012-2014 Neko. */

/* PLATFORM is the hub that connects all platform-specific modules */

#include "type.h"



#include "core/platform/display_frame.h"

#include "vm/platform/platform_internal.h"

type_status vm_platform_run_context_create(
    const vm_platform_execution_transport *execution,
    const vm_platform_host_input_sink *input_sink,
    const core_platform_presentation_mailbox *presentation,
    const core_utils_wait_scope *wait_scope, const C_CHAR *font_path,
    vm_platform_run_context **out_context)
{
    vm_platform_run_context *context;

    if (out_context == STD_NULL) return TYPE_STATUS_INVALID_ARGUMENT;
    *out_context = STD_NULL;
    context = STD_MALLOC(sizeof(*context));
    if (context == STD_NULL) return TYPE_STATUS_NO_MEMORY;
    context->execution = execution;
    context->input_sink = input_sink == STD_NULL ?
        (vm_platform_host_input_sink){0} : *input_sink;
    context->presentation = presentation;
    context->wait_scope = wait_scope;
    context->font_path = font_path;
    vm_platform_host_surface_context_initialize(&context->console_surface,
        VM_PLATFORM_HOST_SURFACE_CONSOLE, STD_NULL);
    vm_platform_host_surface_context_initialize(&context->window_surface,
        VM_PLATFORM_HOST_SURFACE_WINDOW, STD_NULL);
    context->console_renderer = STD_NULL;
    context->window_renderer = STD_NULL;
    context->terminal_displayed_generation = 0u;
    context->display_mode = VM_PLATFORM_DISPLAY_CONSOLE;
    STD_ATOMIC_INIT(&context->console_window_active, TYPE_FALSE);
    STD_ATOMIC_INIT(&context->console_window_start_pending, TYPE_FALSE);
    STD_ATOMIC_INIT(&context->console_window_stop_pending, TYPE_FALSE);
    *out_context = context;
    return TYPE_STATUS_OK;
}

C_VOID vm_platform_run_context_destroy(vm_platform_run_context *context)
{
    STD_FREE(context);
}

type_status vm_platform_host_input_sink_submit(
    const vm_platform_host_input_sink *sink,
    const core_platform_input_event *event)
{
    if (sink == STD_NULL || sink->submit == STD_NULL) {
        return TYPE_STATUS_INVALID_STATE;
    }
    return sink->submit(sink->context, event);
}

C_INT vm_platform_run_context_get_window_display(
    const vm_platform_run_context *context)
{
    return context != STD_NULL && (context->display_mode ==
        VM_PLATFORM_DISPLAY_WINDOW || (context->display_mode ==
        VM_PLATFORM_DISPLAY_CONSOLE && STD_ATOMIC_LOAD(
        &context->console_window_active)));
}

C_INT vm_platform_run_context_get_display_mode(
    const vm_platform_run_context *context)
{
    return context == STD_NULL ? VM_PLATFORM_DISPLAY_CONSOLE :
        context->display_mode;
}

C_VOID vm_platform_run_context_set_display_mode(
    vm_platform_run_context *context, vm_platform_display_mode mode)
{
    if (context == STD_NULL || mode < VM_PLATFORM_DISPLAY_CONSOLE ||
        mode > VM_PLATFORM_DISPLAY_WINDOW) return;
    context->display_mode = mode;
    STD_ATOMIC_STORE(&context->console_window_active, TYPE_FALSE);
    STD_ATOMIC_STORE(&context->console_window_start_pending, TYPE_FALSE);
    STD_ATOMIC_STORE(&context->console_window_stop_pending, TYPE_FALSE);
}

C_VOID vm_platform_run_context_set_window_display(
    vm_platform_run_context *context, C_INT enabled)
{
    vm_platform_run_context_set_display_mode(context, enabled ?
        VM_PLATFORM_DISPLAY_WINDOW : VM_PLATFORM_DISPLAY_CONSOLE);
}

C_INT vm_platform_run_context_request_console_window_start(
    vm_platform_run_context *context)
{
    if (context == STD_NULL || context->display_mode != VM_PLATFORM_DISPLAY_CONSOLE ||
        STD_ATOMIC_LOAD(&context->console_window_active) ||
        STD_ATOMIC_EXCHANGE(&context->console_window_start_pending, TYPE_TRUE)) {
        return TYPE_FALSE;
    }
    STD_ATOMIC_STORE(&context->console_window_stop_pending, TYPE_FALSE);
    return TYPE_TRUE;
}

C_INT vm_platform_run_context_take_console_window_start(
    vm_platform_run_context *context)
{
    return context != STD_NULL && STD_ATOMIC_EXCHANGE(
        &context->console_window_start_pending, TYPE_FALSE);
}

C_VOID vm_platform_run_context_confirm_console_window_started(
    vm_platform_run_context *context)
{
    if (context != STD_NULL) STD_ATOMIC_STORE(&context->console_window_active,
        TYPE_TRUE);
}

C_INT vm_platform_run_context_request_console_window_stop(
    vm_platform_run_context *context)
{
    C_INT active;
    C_INT pending;

    if (context == STD_NULL || context->display_mode != VM_PLATFORM_DISPLAY_CONSOLE) {
        return TYPE_FALSE;
    }
    active = STD_ATOMIC_EXCHANGE(&context->console_window_active, TYPE_FALSE);
    pending = STD_ATOMIC_EXCHANGE(&context->console_window_start_pending,
        TYPE_FALSE);
    if (!active && !pending) return TYPE_FALSE;
    if (active) STD_ATOMIC_STORE(&context->console_window_stop_pending,
        TYPE_TRUE);
    return TYPE_TRUE;
}

C_INT vm_platform_run_context_take_console_window_stop(
    vm_platform_run_context *context)
{
    return context != STD_NULL && STD_ATOMIC_EXCHANGE(
        &context->console_window_stop_pending, TYPE_FALSE);
}

type_status vm_platform_run_handle_create(vm_platform_run_handle **out_handle)
{
    vm_platform_run_handle *handle;

    if (out_handle == STD_NULL) return TYPE_STATUS_INVALID_ARGUMENT;
    *out_handle = STD_NULL;
    handle = STD_CALLOC(1u, sizeof(*handle));
    if (handle == STD_NULL) return TYPE_STATUS_NO_MEMORY;
    vm_platform_run_handle_initialize(handle);
    *out_handle = handle;
    return TYPE_STATUS_OK;
}

C_VOID vm_platform_run_handle_initialize(vm_platform_run_handle *handle)
{
    if (handle == STD_NULL) return;
    STD_MEMSET(handle, 0, sizeof(*handle));
    STD_ATOMIC_INIT(&handle->last_event, VM_PLATFORM_RUN_EVENT_NONE);
    STD_ATOMIC_INIT(&handle->stop_reported, TYPE_FALSE);
    STD_ATOMIC_INIT(&handle->pause_reported, TYPE_FALSE);
}

C_VOID vm_platform_run_handle_destroy(vm_platform_run_handle *handle)
{
    if (handle == STD_NULL) return;
    STD_FREE(handle);
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

C_VOID vm_platform_run_handle_report(
    vm_platform_run_handle *handle, vm_platform_run_event event)
{
    if (handle == STD_NULL) return;
    STD_ATOMIC_STORE(&handle->last_event, event);
    if (event == VM_PLATFORM_RUN_EVENT_STOP_REQUESTED ||
        event == VM_PLATFORM_RUN_EVENT_STARTUP_FAILED) {
        STD_ATOMIC_STORE(&handle->stop_reported, TYPE_TRUE);
    } else if (event == VM_PLATFORM_RUN_EVENT_PAUSE_REQUESTED) {
        STD_ATOMIC_STORE(&handle->pause_reported, TYPE_TRUE);
    }
}

vm_platform_run_event vm_platform_run_handle_get_last_event(
    const vm_platform_run_handle *handle)
{
    return handle == STD_NULL ? VM_PLATFORM_RUN_EVENT_NONE :
        (vm_platform_run_event)STD_ATOMIC_LOAD(&handle->last_event);
}

C_INT vm_platform_run_handle_take_stop_report(
    vm_platform_run_handle *handle)
{
    return handle != STD_NULL && STD_ATOMIC_EXCHANGE(&handle->stop_reported,
        TYPE_FALSE);
}

C_INT vm_platform_run_handle_take_pause_report(
    vm_platform_run_handle *handle)
{
    return handle != STD_NULL && STD_ATOMIC_EXCHANGE(&handle->pause_reported,
        TYPE_FALSE);
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
