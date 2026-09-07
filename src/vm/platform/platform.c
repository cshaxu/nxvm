/* Copyright 2012-2014 Neko. */

/* PLATFORM is the hub that connects all platform-specific modules */

#include "type.h"



#include "core/machine/guest_display_frame.h"

#include "vm/platform/platform_internal.h"
#include "vm/platform/ux_binding.h"
#include "vm/platform/ux_frame.h"

static type_status vm_platform_run_context_request_ux_target(
    vm_platform_run_context *context, ux_target target);

static C_INT vm_platform_run_context_target_is_mouse_capturable(
    const vm_platform_run_context *context, ux_target target)
{
    return context != STD_NULL && target == UX_TARGET_WINDOW &&
        context->execution != STD_NULL && context->execution->state != STD_NULL &&
        vm_session_state_lifecycle(context->execution->state) == VM_SESSION_RUNNING;
}

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
        ux_presenter_create(&context->ux_presenter) != LIB_STATUS_OK) {
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
    context->requested_target = UX_TARGET_NONE;
    context->console_text_frames = 0u;
    context->display_mode = VM_PLATFORM_DISPLAY_CONSOLE;
    if (vm_platform_run_context_request_ux_target(context, UX_TARGET_CONSOLE) !=
        TYPE_STATUS_OK) {
        vm_platform_run_context_destroy(context);
        return TYPE_STATUS_INVALID_STATE;
    }
    *out_context = context;
    return TYPE_STATUS_OK;
}

C_VOID vm_platform_run_context_destroy(vm_platform_run_context *context)
{
    if (context != STD_NULL) ux_presenter_destroy(context->ux_presenter);
    if (context != STD_NULL) STD_FREE(context->ux_frame);
    if (context != STD_NULL) STD_FREE(context->core_frame);
    STD_FREE(context);
}

static type_status vm_platform_run_context_request_ux_target(
    vm_platform_run_context *context, ux_target target)
{
    lib_status status;
    type_status capture_status;

    if (context == STD_NULL) return TYPE_STATUS_INVALID_ARGUMENT;
    capture_status = vm_platform_run_context_set_mouse_capturable(context,
        vm_platform_run_context_target_is_mouse_capturable(context, target));
    if (capture_status != TYPE_STATUS_OK) return capture_status;
    if (context->requested_target == target) return TYPE_STATUS_OK;
    status = ux_presenter_set_target(context->ux_presenter, target);
    if (status != LIB_STATUS_OK) return (type_status)status;
    context->requested_target = target;
    return TYPE_STATUS_OK;
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
        status = vm_platform_run_context_request_ux_target(context, UX_TARGET_WINDOW);
    } else if (context->ux_frame->graphics != 0u) {
        context->console_text_frames = 0u;
        status = vm_platform_run_context_request_ux_target(context, UX_TARGET_WINDOW);
    } else if (context->requested_target == UX_TARGET_WINDOW &&
        ++context->console_text_frames >= 3u) {
        context->console_text_frames = 0u;
        status = vm_platform_run_context_request_ux_target(context, UX_TARGET_CONSOLE);
    }
    if (status != TYPE_STATUS_OK) return status;
    return (type_status)ux_presenter_publish_frame(context->ux_presenter,
        context->ux_frame);
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
    return context != STD_NULL && context->requested_target == UX_TARGET_WINDOW;
}

C_INT vm_platform_run_context_get_display_mode(
    const vm_platform_run_context *context)
{
    return context == STD_NULL ? VM_PLATFORM_DISPLAY_CONSOLE :
        context->display_mode;
}

type_status vm_platform_run_context_set_display_mode(
    vm_platform_run_context *context, vm_platform_display_mode mode)
{
    type_status status;

    if (context == STD_NULL || mode < VM_PLATFORM_DISPLAY_CONSOLE ||
        mode > VM_PLATFORM_DISPLAY_WINDOW) return TYPE_STATUS_INVALID_ARGUMENT;
    context->display_mode = mode;
    context->console_text_frames = 0u;
    status = vm_platform_run_context_request_ux_target(context,
        mode == VM_PLATFORM_DISPLAY_WINDOW ? UX_TARGET_WINDOW : UX_TARGET_CONSOLE);
    if (status != TYPE_STATUS_OK) return status;
    return TYPE_STATUS_OK;
}

type_status vm_platform_run_context_set_window_display(
    vm_platform_run_context *context, C_INT enabled)
{
    return vm_platform_run_context_set_display_mode(context, enabled ?
        VM_PLATFORM_DISPLAY_WINDOW : VM_PLATFORM_DISPLAY_CONSOLE);
}

type_status vm_platform_run_context_set_window_title(
    vm_platform_run_context *context, const C_CHAR *title)
{
    if (context == STD_NULL || title == STD_NULL) return TYPE_STATUS_INVALID_ARGUMENT;
    return (type_status)ux_presenter_set_window_title(context->ux_presenter, title);
}

type_status vm_platform_run_context_set_mouse_capturable(
    vm_platform_run_context *context, C_INT capturable)
{
    if (context == STD_NULL) return TYPE_STATUS_INVALID_ARGUMENT;
    return (type_status)ux_presenter_set_mouse_capturable(context->ux_presenter,
        capturable ? LIB_TRUE : LIB_FALSE);
}

type_status vm_platform_run_context_release_mouse(vm_platform_run_context *context)
{
    if (context == STD_NULL) return TYPE_STATUS_INVALID_ARGUMENT;
    return (type_status)ux_presenter_release_mouse(context->ux_presenter);
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
    return handle != STD_NULL && handle->context != STD_NULL &&
        vm_platform_run_context_get_window_display(handle->context);
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

C_VOID vm_platform_run_handle_request_presenter_stop(
    vm_platform_run_handle *handle)
{
    if (handle == STD_NULL || handle->context == STD_NULL) return;
    (C_VOID)vm_platform_run_context_set_mouse_capturable(handle->context,
        TYPE_FALSE);
    (C_VOID)ux_presenter_set_target(handle->context->ux_presenter, UX_TARGET_NONE);
}
