/* NXVM's Win32 product binding owns VM threads only.  lib/ux owns native UI. */

#include "type.h"

#include "core/platform/input_interface.h"
#include "core/platform/win32/keyboard.h"
#include "lib/ux/win32/runner.h"
#include "vm/platform/platform_internal.h"
#include "vm/platform/ux_binding.h"
#include "vm/platform/win32/win32.h"

#include <windows.h>

typedef struct vm_platform_win32_ux_handle {
    const vm_platform_run_context *context;
    vm_platform_run_handle *owner;
    HANDLE kernel_thread;
    HANDLE presenter_thread;
} vm_platform_win32_ux_handle;

static type_status vm_platform_win32_submit_event(C_VOID *opaque,
    const core_platform_input_event *event)
{
    const vm_platform_run_context *context = opaque;

    return context == STD_NULL ? TYPE_STATUS_INVALID_ARGUMENT :
        vm_platform_host_input_sink_submit(&context->input_sink, event);
}

C_VOID vm_platform_win32_keyboard_make_key_for(
    const vm_platform_run_context *context, vm_platform_run_handle *owner,
    type_unsigned_16 scan_code, type_unsigned_16 virtual_key, C_INT pressed)
{
    (C_VOID)owner;
    (C_VOID)core_platform_win32_keyboard_submit_key((C_VOID *)context,
        vm_platform_win32_submit_event, scan_code, virtual_key, pressed);
}

C_VOID vm_platform_win32_keyboard_make_key_with_modifiers_for(
    const vm_platform_run_context *context, vm_platform_run_handle *owner,
    type_unsigned_16 scan_code, type_unsigned_16 virtual_key,
    type_unsigned_8 modifiers, C_INT pressed)
{
    (C_VOID)modifiers;
    vm_platform_win32_keyboard_make_key_for(context, owner, scan_code,
        virtual_key, pressed);
}

C_VOID vm_platform_win32_keyboard_make_character_for(
    const vm_platform_run_context *context, type_unsigned_32 scalar)
{
    (C_VOID)core_platform_win32_keyboard_submit_character((C_VOID *)context,
        vm_platform_win32_submit_event, scalar);
}

C_VOID vm_platform_win32_keyboard_make_utf16_for(
    core_platform_win32_keyboard_normalizer *state,
    const vm_platform_run_context *context, type_unsigned_16 code_unit)
{
    (C_VOID)core_platform_win32_keyboard_submit_utf16(state,
        (C_VOID *)context, vm_platform_win32_submit_event, code_unit);
}

C_VOID vm_platform_win32_mouse_relative_for(
    const vm_platform_run_context *context, type_signed_16 delta_x,
    type_signed_16 delta_y, type_unsigned_8 buttons)
{
    core_platform_input_event event;

    if (context == STD_NULL) return;
    STD_MEMSET(&event, 0, sizeof(event));
    event.kind = CORE_PLATFORM_INPUT_RELATIVE_MOUSE;
    event.data.relative_mouse.delta_x = delta_x;
    event.data.relative_mouse.delta_y = delta_y;
    event.data.relative_mouse.buttons = buttons;
    (C_VOID)vm_platform_host_input_sink_submit(&context->input_sink, &event);
}

static DWORD WINAPI vm_platform_win32_ux_presenter_thread(LPVOID opaque)
{
    vm_platform_win32_ux_handle *handle = opaque;
    ux_binding binding;

    if (handle == STD_NULL || vm_platform_ux_binding_initialize(
            handle->context, handle->owner, &binding) != TYPE_STATUS_OK ||
        ux_win32_run(&binding) == UX_RUN_ERROR_RESULT) {
        if (handle != STD_NULL) vm_platform_run_handle_report(handle->owner,
            VM_PLATFORM_RUN_EVENT_STARTUP_FAILED);
    }
    return 0;
}

static DWORD WINAPI vm_platform_win32_ux_kernel_thread(LPVOID opaque)
{
    vm_platform_win32_ux_handle *handle = opaque;

    if (handle == STD_NULL) return 0;
    vm_platform_execution_start_for(handle->context->execution);
    vm_platform_run_handle_report(handle->owner,
        VM_PLATFORM_RUN_EVENT_KERNEL_COMPLETED);
    return 0;
}

type_status vm_platform_win32_run_handle_start(
    const vm_platform_run_context *context, vm_platform_run_handle *owner)
{
    vm_platform_win32_ux_handle *handle;
    DWORD thread_id;
    C_INT old_flip;

    if (context == STD_NULL || owner == STD_NULL || owner->active ||
        context->execution == STD_NULL || context->input_sink.submit == STD_NULL)
        return TYPE_STATUS_INVALID_ARGUMENT;
    handle = STD_CALLOC(1u, sizeof(*handle));
    if (handle == STD_NULL) return TYPE_STATUS_NO_MEMORY;
    handle->context = context;
    handle->owner = owner;
    owner->context = context;
    owner->backend = handle;
    owner->window_display = vm_platform_run_context_get_display_mode(context) ==
        VM_PLATFORM_DISPLAY_WINDOW;
    owner->active = TYPE_TRUE;
    ux_router_initialize((ux_router *)&context->ux_router,
        owner->window_display ? UX_DISPLAY_WINDOW : UX_DISPLAY_CONSOLE);
    old_flip = vm_platform_execution_get_flip_for(context->execution);
    handle->kernel_thread = CreateThread(STD_NULL, 0,
        vm_platform_win32_ux_kernel_thread, handle, 0, &thread_id);
    if (handle->kernel_thread == STD_NULL ||
        !vm_platform_execution_wait_for_flip_for(context->execution, old_flip,
            VM_PLATFORM_EXECUTION_FLIP_TIMEOUT_MILLISECONDS)) {
        vm_platform_win32_run_handle_request_stop(owner);
        vm_platform_win32_run_handle_join(owner);
        vm_platform_win32_run_handle_finalize(owner);
        return TYPE_STATUS_INVALID_STATE;
    }
    handle->presenter_thread = CreateThread(STD_NULL, 0,
        vm_platform_win32_ux_presenter_thread, handle, 0, &thread_id);
    if (handle->presenter_thread == STD_NULL) {
        vm_platform_win32_run_handle_request_stop(owner);
        vm_platform_win32_run_handle_join(owner);
        vm_platform_win32_run_handle_finalize(owner);
        return TYPE_STATUS_INVALID_STATE;
    }
    return TYPE_STATUS_OK;
}

C_VOID vm_platform_win32_run_handle_request_stop(vm_platform_run_handle *owner)
{
    vm_platform_win32_ux_handle *handle = owner == STD_NULL ? STD_NULL :
        owner->backend;

    if (handle != STD_NULL) vm_platform_execution_stop_for(
        handle->context->execution);
}

C_VOID vm_platform_win32_run_handle_join(vm_platform_run_handle *owner)
{
    vm_platform_win32_ux_handle *handle = owner == STD_NULL ? STD_NULL :
        owner->backend;

    if (handle == STD_NULL) return;
    if (handle->kernel_thread != STD_NULL) WaitForSingleObject(
        handle->kernel_thread, INFINITE);
    if (handle->presenter_thread != STD_NULL) WaitForSingleObject(
        handle->presenter_thread, INFINITE);
}

C_VOID vm_platform_win32_run_handle_finalize(vm_platform_run_handle *owner)
{
    vm_platform_win32_ux_handle *handle = owner == STD_NULL ? STD_NULL :
        owner->backend;

    if (handle == STD_NULL) return;
    if (handle->kernel_thread != STD_NULL) CloseHandle(handle->kernel_thread);
    if (handle->presenter_thread != STD_NULL) CloseHandle(handle->presenter_thread);
    STD_FREE(handle);
    vm_platform_run_handle_initialize(owner);
}
