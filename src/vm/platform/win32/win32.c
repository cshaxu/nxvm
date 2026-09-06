/* NXVM's Win32 product binding owns VM threads only.  lib/ux owns native UI. */

#include "type.h"

#include "lib/host/sync.h"
#include "lib/ux/win32/input.h"
#include "lib/ux/win32/runner.h"
#include "vm/platform/platform_internal.h"
#include "vm/platform/ux_binding.h"
#include "vm/platform/win32/win32.h"

#include <windows.h>

typedef struct vm_platform_win32_ux_handle {
    const vm_platform_run_context *context;
    vm_platform_run_handle *owner;
    host_sync_task *kernel_task;
    host_sync_task *presenter_task;
    host_sync_event *kernel_started;
} vm_platform_win32_ux_handle;

static void vm_platform_win32_signal_started(void *opaque)
{ host_sync_event_signal(opaque); }

static int vm_platform_win32_submit_ux_event(void *opaque, const ux_event *event)
{
    return vm_platform_ux_event_submit(opaque, event);
}

C_VOID vm_platform_win32_keyboard_make_key_for(
    const vm_platform_run_context *context, vm_platform_run_handle *owner,
    type_unsigned_16 scan_code, type_unsigned_16 virtual_key, C_INT pressed)
{
    (C_VOID)owner;
    (C_VOID)ux_win32_keyboard_submit_transition((C_VOID *)context,
        vm_platform_win32_submit_ux_event, scan_code, virtual_key, 0u, pressed);
}

C_VOID vm_platform_win32_keyboard_make_key_with_modifiers_for(
    const vm_platform_run_context *context, vm_platform_run_handle *owner,
    type_unsigned_16 scan_code, type_unsigned_16 virtual_key,
    type_unsigned_8 modifiers, C_INT pressed)
{
    (C_VOID)owner;
    (C_VOID)ux_win32_keyboard_submit_transition((C_VOID *)context,
        vm_platform_win32_submit_ux_event, scan_code, virtual_key, modifiers, pressed);
}

C_VOID vm_platform_win32_keyboard_make_character_for(
    const vm_platform_run_context *context, type_unsigned_32 scalar)
{
    ux_win32_keyboard_normalizer normalizer = {0};

    if (scalar <= 0xffffu) (C_VOID)ux_win32_keyboard_submit_utf16(&normalizer,
        (C_VOID *)context, vm_platform_win32_submit_ux_event, (WORD)scalar);
}

C_VOID vm_platform_win32_keyboard_make_utf16_for(
    ux_win32_keyboard_normalizer *state,
    const vm_platform_run_context *context, type_unsigned_16 code_unit)
{
    (C_VOID)ux_win32_keyboard_submit_utf16(state,
        (C_VOID *)context, vm_platform_win32_submit_ux_event, code_unit);
}

C_VOID vm_platform_win32_mouse_relative_for(
    const vm_platform_run_context *context, type_signed_16 delta_x,
    type_signed_16 delta_y, type_unsigned_8 buttons)
{
    core_machine_guest_input_event event;

    if (context == STD_NULL) return;
    STD_MEMSET(&event, 0, sizeof(event));
    event.kind = CORE_MACHINE_GUEST_INPUT_RELATIVE_MOUSE;
    event.data.relative_mouse.delta_x = delta_x;
    event.data.relative_mouse.delta_y = delta_y;
    event.data.relative_mouse.buttons = buttons;
    (C_VOID)vm_platform_host_input_sink_submit(&context->input_sink, &event);
}

static void vm_platform_win32_ux_presenter_task(void *opaque,
    const host_sync_task *task)
{
    vm_platform_win32_ux_handle *handle = opaque;
    ux_binding binding;

    (void)task;
    if (handle == STD_NULL || vm_platform_ux_binding_initialize(
            handle->context, handle->owner, &binding) != TYPE_STATUS_OK ||
        ux_win32_run(&binding) == UX_RUN_ERROR_RESULT) {
        if (handle != STD_NULL) vm_platform_run_handle_report(handle->owner,
            VM_PLATFORM_RUN_EVENT_STARTUP_FAILED);
    }
}

static void vm_platform_win32_ux_kernel_task(void *opaque,
    const host_sync_task *task)
{
    vm_platform_win32_ux_handle *handle = opaque;

    (void)task;
    if (handle == STD_NULL) return;
    lib_session_state_start(handle->context->execution->state);
    vm_platform_win32_signal_started(handle->kernel_started);
    handle->context->execution->run(handle->context->execution->context);
    vm_platform_run_handle_report(handle->owner,
        VM_PLATFORM_RUN_EVENT_KERNEL_COMPLETED);
}

type_status vm_platform_win32_run_handle_start(
    const vm_platform_run_context *context, vm_platform_run_handle *owner)
{
    vm_platform_win32_ux_handle *handle;

    if (context == STD_NULL || owner == STD_NULL || owner->active ||
        context->execution == STD_NULL || context->execution->state == STD_NULL ||
        context->execution->run == STD_NULL || context->execution->stop == STD_NULL ||
        context->input_sink.submit == STD_NULL)
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
    if (host_sync_event_create(&handle->kernel_started) != LIB_STATUS_OK ||
        host_sync_task_create(vm_platform_win32_ux_kernel_task, handle,
            &handle->kernel_task) != LIB_STATUS_OK) {
        vm_platform_win32_run_handle_request_stop(owner);
        vm_platform_win32_run_handle_join(owner);
        vm_platform_win32_run_handle_finalize(owner);
        return TYPE_STATUS_INVALID_STATE;
    }
    if (host_sync_event_wait(handle->kernel_started,
            VM_PLATFORM_START_TIMEOUT_MILLISECONDS) !=
        HOST_SYNC_WAIT_SIGNALED) {
        vm_platform_win32_run_handle_request_stop(owner);
        vm_platform_win32_run_handle_join(owner);
        vm_platform_win32_run_handle_finalize(owner);
        return TYPE_STATUS_INVALID_STATE;
    }
    if (host_sync_task_create(vm_platform_win32_ux_presenter_task, handle,
            &handle->presenter_task) != LIB_STATUS_OK) {
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

    if (handle != STD_NULL) handle->context->execution->stop(
        handle->context->execution->context);
    if (handle != STD_NULL) {
        ux_mailbox_wake(handle->context->ux_mailbox);
        host_sync_task_request_cancel(handle->kernel_task);
        host_sync_task_request_cancel(handle->presenter_task);
    }
}

C_VOID vm_platform_win32_run_handle_join(vm_platform_run_handle *owner)
{
    vm_platform_win32_ux_handle *handle = owner == STD_NULL ? STD_NULL :
        owner->backend;

    if (handle == STD_NULL) return;
    host_sync_task_join(handle->kernel_task);
    host_sync_task_join(handle->presenter_task);
}

C_VOID vm_platform_win32_run_handle_finalize(vm_platform_run_handle *owner)
{
    vm_platform_win32_ux_handle *handle = owner == STD_NULL ? STD_NULL :
        owner->backend;

    if (handle == STD_NULL) return;
    host_sync_task_destroy(handle->kernel_task);
    host_sync_task_destroy(handle->presenter_task);
    host_sync_event_destroy(handle->kernel_started);
    STD_FREE(handle);
    vm_platform_run_handle_initialize(owner);
}
