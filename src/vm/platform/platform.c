/* Copyright 2012-2014 Neko. */

/* PLATFORM is the hub that connects all platform-specific modules */

#include "type.h"



#include "core/machine/guest_display_frame.h"

#include "vm/platform/platform_internal.h"
#include "vm/platform/ux_binding.h"
#include "vm/platform/ux_frame.h"

type_status vm_platform_run_context_create(
    const vm_platform_execution *execution,
    const vm_platform_host_input_sink *input_sink,
    const core_machine_guest_presentation_mailbox *presentation,
    const core_utils_wait_scope *wait_scope,
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
    context->core_frame = STD_MALLOC(sizeof(*context->core_frame));
    context->ux_frame = STD_MALLOC(sizeof(*context->ux_frame));
    if (context->core_frame == STD_NULL || context->ux_frame == STD_NULL ||
        ux_mailbox_create(&context->ux_mailbox) != LIB_STATUS_OK) {
        STD_FREE(context->ux_frame);
        STD_FREE(context->core_frame);
        STD_FREE(context);
        return TYPE_STATUS_NO_MEMORY;
    }
    ux_actions_initialize(&context->ux_actions);
    (C_VOID)ux_actions_register(&context->ux_actions, 'P',
        UX_MODIFIER_CONTROL | UX_MODIFIER_ALT, VM_PLATFORM_UX_ACTION_PAUSE_TOGGLE);
    (C_VOID)ux_actions_register(&context->ux_actions, 'D',
        UX_MODIFIER_CONTROL | UX_MODIFIER_ALT, VM_PLATFORM_UX_ACTION_SEND_CTRL_ALT_DEL);
    (C_VOID)ux_actions_register(&context->ux_actions, 'F',
        UX_MODIFIER_CONTROL | UX_MODIFIER_ALT, VM_PLATFORM_UX_ACTION_SEND_ALT_ENTER);
    (C_VOID)ux_actions_register(&context->ux_actions, 'M',
        UX_MODIFIER_CONTROL | UX_MODIFIER_ALT, VM_PLATFORM_UX_ACTION_RELEASE_MOUSE);
    ux_router_initialize(&context->ux_router, UX_TARGET_CONSOLE);
    context->console_text_frames = 0u;
    context->display_mode = VM_PLATFORM_DISPLAY_CONSOLE;
    *out_context = context;
    return TYPE_STATUS_OK;
}

C_VOID vm_platform_run_context_destroy(vm_platform_run_context *context)
{
    if (context != STD_NULL) ux_mailbox_destroy(context->ux_mailbox);
    if (context != STD_NULL) STD_FREE(context->ux_frame);
    if (context != STD_NULL) STD_FREE(context->core_frame);
    STD_FREE(context);
}

type_status vm_platform_run_context_publish_ux_frame(
    vm_platform_run_context *context)
{
    type_status status;

    if (context == STD_NULL || context->presentation == STD_NULL ||
        context->core_frame == STD_NULL || context->ux_frame == STD_NULL)
        return TYPE_STATUS_INVALID_ARGUMENT;
    status = core_machine_guest_presentation_mailbox_capture(context->presentation,
        context->core_frame);
    if (status != TYPE_STATUS_OK) return status;
    status = vm_platform_ux_frame_from_core(context->core_frame, context->ux_frame);
    if (status != TYPE_STATUS_OK) return status;
    if (context->display_mode == VM_PLATFORM_DISPLAY_WINDOW) {
        ux_router_request(&context->ux_router, UX_TARGET_WINDOW);
    } else if (context->ux_frame->graphics != 0u) {
        context->console_text_frames = 0u;
        ux_router_request(&context->ux_router, UX_TARGET_WINDOW);
    } else if (ux_router_target(&context->ux_router) == UX_TARGET_WINDOW &&
        ++context->console_text_frames >= 3u) {
        context->console_text_frames = 0u;
        ux_router_request(&context->ux_router, UX_TARGET_CONSOLE);
    }
    return ux_mailbox_publish(context->ux_mailbox, context->ux_frame);
}

type_status vm_platform_host_input_sink_submit(
    const vm_platform_host_input_sink *sink,
    const core_machine_guest_input_event *event)
{
    if (sink == STD_NULL || sink->submit == STD_NULL) {
        return TYPE_STATUS_INVALID_STATE;
    }
    return sink->submit(sink->context, event);
}

C_INT vm_platform_run_context_get_window_display(
    const vm_platform_run_context *context)
{
    return context != STD_NULL && context->display_mode ==
        VM_PLATFORM_DISPLAY_WINDOW;
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
    context->console_text_frames = 0u;
    ux_router_request(&context->ux_router, mode == VM_PLATFORM_DISPLAY_WINDOW ?
        UX_TARGET_WINDOW : UX_TARGET_CONSOLE);
}

C_VOID vm_platform_run_context_set_window_display(
    vm_platform_run_context *context, C_INT enabled)
{
    vm_platform_run_context_set_display_mode(context, enabled ?
        VM_PLATFORM_DISPLAY_WINDOW : VM_PLATFORM_DISPLAY_CONSOLE);
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
    STD_ATOMIC_INIT(&handle->mouse_release_reported, TYPE_FALSE);
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
    return handle != STD_NULL && handle->context != STD_NULL &&
        vm_platform_run_context_get_display_mode(handle->context) ==
            VM_PLATFORM_DISPLAY_WINDOW;
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
    } else if (event == VM_PLATFORM_RUN_EVENT_MOUSE_RELEASE_REQUESTED) {
        STD_ATOMIC_STORE(&handle->mouse_release_reported, TYPE_TRUE);
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

C_INT vm_platform_run_handle_take_mouse_release_report(
    vm_platform_run_handle *handle)
{
    return handle != STD_NULL && STD_ATOMIC_EXCHANGE(
        &handle->mouse_release_reported, TYPE_FALSE);
}

#if GLOBAL_PLATFORM == GLOBAL_VAR_WIN32

#include "vm/platform/win32/win32.h"
type_status vm_platform_start(vm_platform_run_context *context,
    vm_platform_run_handle *handle) {
    return vm_platform_win32_run_handle_start(context, handle);
}
C_VOID vm_platform_run_handle_request_stop(vm_platform_run_handle *handle) {
    vm_platform_win32_run_handle_request_stop(handle);
}
C_VOID vm_platform_run_handle_request_presenter_stop(
    vm_platform_run_handle *handle)
{
    if (handle == STD_NULL || handle->context == STD_NULL) return;
    ux_router_request(&handle->context->ux_router, UX_TARGET_NONE);
    ux_mailbox_wake(handle->context->ux_mailbox);
}
C_VOID vm_platform_run_handle_join(vm_platform_run_handle *handle) {
    vm_platform_win32_run_handle_join(handle);
}
C_VOID vm_platform_run_handle_finalize(vm_platform_run_handle *handle) {
    vm_platform_win32_run_handle_finalize(handle);
}
#elif GLOBAL_PLATFORM == GLOBAL_VAR_LINUX

#include "vm/platform/linux/linux.h"
type_status vm_platform_start(vm_platform_run_context *context,
    vm_platform_run_handle *handle) {
    return vm_platform_linux_run_handle_start(context, handle);
}
C_VOID vm_platform_run_handle_request_stop(vm_platform_run_handle *handle) {
    vm_platform_linux_run_handle_request_stop(handle);
}
C_VOID vm_platform_run_handle_request_presenter_stop(
    vm_platform_run_handle *handle)
{
    if (handle == STD_NULL || handle->context == STD_NULL) return;
    ux_router_request(&handle->context->ux_router, UX_TARGET_NONE);
    ux_mailbox_wake(handle->context->ux_mailbox);
}
C_VOID vm_platform_run_handle_join(vm_platform_run_handle *handle) {
    vm_platform_linux_run_handle_join(handle);
}
C_VOID vm_platform_run_handle_finalize(vm_platform_run_handle *handle) {
    vm_platform_linux_run_handle_finalize(handle);
}
#endif
