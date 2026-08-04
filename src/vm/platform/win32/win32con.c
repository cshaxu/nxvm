/* Copyright 2012-2014 Neko. */

#include "type.h"

#include "core/product/wait.h"

#include "core/product/utils.h"

#include "vm/platform/win32/win32.h"
#include "vm/platform/win32/w32cdisp.h"
#include "vm/platform/win32/win32con.h"

typedef struct win32con_run_handle {
    vm_platform_run_handle *owner;
    const vm_platform_run_context *platform;
    HANDLE input;
    HANDLE output;
    HANDLE kernel_thread;
    HANDLE display_thread;
} win32con_run_handle;

static core_platform_host_surface_lease win32_console_lease = {
    ATOMIC_VAR_INIT(0)
};

static C_INT win32con_test_should_fail(C_INT stage)
{
#if defined(NTVDM64_VM_PLATFORM_TEST_FAILURE_STAGE)
    return NTVDM64_VM_PLATFORM_TEST_FAILURE_STAGE == stage;
#else
    (C_VOID)stage;
    return 0;
#endif
}

static C_VOID win32con_process_input(const win32con_run_handle *handle)
{
    DWORD count;
    INPUT_RECORD input;
    UCHAR scan_code;
    UCHAR virtual_key;

    GetNumberOfConsoleInputEvents(handle->input, &count);
    if (count == 0u || !ReadConsoleInput(handle->input, &input, 1u, &count)) return;
    switch (input.EventType) {
    case KEY_EVENT:
        scan_code = (UCHAR)input.Event.KeyEvent.wVirtualScanCode;
        virtual_key = (UCHAR)input.Event.KeyEvent.wVirtualKeyCode;
        if (input.Event.KeyEvent.bKeyDown) {
            vm_platform_win32_keyboard_make_key_for(handle->platform, scan_code,
                virtual_key);
        } else {
            vm_platform_win32_keyboard_make_status_for(handle->platform);
        }
        break;
    case FOCUS_EVENT:
        vm_platform_win32_keyboard_make_status_for(handle->platform);
        break;
    default:
        break;
    }
}

static DWORD WINAPI win32con_display_thread(LPVOID opaque)
{
    win32con_run_handle *handle = opaque;

    w32cdispInit((w32cdisp_context *)handle->platform->console_renderer,
        handle->output, handle->platform->presentation);
    while (vm_platform_execution_is_running_for(handle->platform->execution)) {
        w32cdispPaint((w32cdisp_context *)handle->platform->console_renderer,
            handle->output, handle->platform->presentation, FALSE);
        win32con_process_input(handle);
        core_product_wait_milliseconds(handle->platform->wait_scope, 20u);
    }
    return 0;
}

static DWORD WINAPI win32con_kernel_thread(LPVOID opaque)
{
    win32con_run_handle *handle = opaque;

    vm_platform_execution_start_for(handle->platform->execution);
    vm_platform_run_handle_report(handle->owner,
        VM_PLATFORM_RUN_EVENT_KERNEL_COMPLETED);
    return 0;
}

type_status vm_platform_win32con_run_handle_start(
    const vm_platform_run_context *context, vm_platform_run_handle *owner)
{
    win32con_run_handle *handle;
    DWORD thread_id;
    C_INT old_flip;
    vm_platform_run_context *platform;

    if (context == STD_NULL || owner == STD_NULL || owner->active ||
        context->execution == STD_NULL || context->keyboard == STD_NULL) {
        return TYPE_STATUS_INVALID_ARGUMENT;
    }
    if (core_platform_host_surface_lease_acquire(&win32_console_lease,
            context) != TYPE_STATUS_OK) return TYPE_STATUS_INVALID_STATE;
    handle = (win32con_run_handle *)STD_CALLOC(1u, sizeof(*handle));
    if (handle == STD_NULL) {
        core_platform_host_surface_lease_release(&win32_console_lease, context);
        return TYPE_STATUS_NO_MEMORY;
    }
    handle->owner = owner;
    handle->platform = context;
    handle->input = GetStdHandle(STD_INPUT_HANDLE);
    handle->output = GetStdHandle(STD_OUTPUT_HANDLE);
    platform = (vm_platform_run_context *)context;
    owner->context = context;
    owner->backend = handle;
    owner->window_display = 0;
    owner->active = 1;
    platform->console_surface.native_handle = handle->output;
    platform->console_renderer = win32con_test_should_fail(6) ? STD_NULL :
        w32cdisp_context_create();
    if (platform->console_renderer == STD_NULL) {
        vm_platform_win32con_run_handle_finalize(owner);
        return TYPE_STATUS_NO_MEMORY;
    }
    old_flip = vm_platform_execution_get_flip_for(context->execution);
    handle->kernel_thread = win32con_test_should_fail(7) ? STD_NULL :
        CreateThread(STD_NULL, 0, win32con_kernel_thread, handle, 0, &thread_id);
    if (handle->kernel_thread == STD_NULL) {
        vm_platform_win32con_run_handle_finalize(owner);
        return TYPE_STATUS_INVALID_STATE;
    }
    while (old_flip == vm_platform_execution_get_flip_for(context->execution)) {
        core_product_wait_milliseconds(context->wait_scope, 100u);
    }
    handle->display_thread = win32con_test_should_fail(8) ? STD_NULL :
        CreateThread(STD_NULL, 0, win32con_display_thread, handle, 0, &thread_id);
    if (handle->display_thread == STD_NULL) {
        vm_platform_win32con_run_handle_request_stop(owner);
        vm_platform_win32con_run_handle_join(owner);
        vm_platform_win32con_run_handle_finalize(owner);
        return TYPE_STATUS_INVALID_STATE;
    }
    return TYPE_STATUS_OK;
}

C_VOID vm_platform_win32con_run_handle_request_stop(vm_platform_run_handle *owner)
{
    win32con_run_handle *handle = owner == STD_NULL ? STD_NULL : owner->backend;

    if (handle != STD_NULL) vm_platform_execution_stop_for(handle->platform->execution);
}

C_VOID vm_platform_win32con_run_handle_join(vm_platform_run_handle *owner)
{
    win32con_run_handle *handle = owner == STD_NULL ? STD_NULL : owner->backend;

    if (handle == STD_NULL) return;
    if (handle->kernel_thread != STD_NULL) WaitForSingleObject(handle->kernel_thread, INFINITE);
    if (handle->display_thread != STD_NULL) WaitForSingleObject(handle->display_thread, INFINITE);
    owner->active = 0;
}

C_VOID vm_platform_win32con_run_handle_finalize(vm_platform_run_handle *owner)
{
    win32con_run_handle *handle = owner == STD_NULL ? STD_NULL : owner->backend;
    vm_platform_run_context *platform;

    if (handle == STD_NULL) return;
    platform = (vm_platform_run_context *)handle->platform;
    if (handle->kernel_thread != STD_NULL) CloseHandle(handle->kernel_thread);
    if (handle->display_thread != STD_NULL) CloseHandle(handle->display_thread);
    if (platform->console_renderer != STD_NULL) {
        w32cdispFinal((w32cdisp_context *)platform->console_renderer,
            handle->output);
        w32cdisp_context_destroy((w32cdisp_context *)platform->console_renderer);
    }
    platform->console_renderer = STD_NULL;
    platform->console_surface.native_handle = STD_NULL;
    core_platform_host_surface_lease_release(&win32_console_lease, handle->platform);
    STD_FREE(handle);
    vm_platform_run_handle_initialize(owner);
}

C_VOID vm_platform_win32con_display_set_screen(const vm_platform_run_context *context)
{
    if (context != STD_NULL && context->console_renderer != STD_NULL) {
        w32cdispSetScreen((w32cdisp_context *)context->console_renderer,
            (HANDLE)context->console_surface.native_handle, context->presentation);
    }
}

C_VOID vm_platform_win32con_display_paint(const vm_platform_run_context *context)
{
    if (context != STD_NULL && context->console_renderer != STD_NULL) {
        w32cdispPaint((w32cdisp_context *)context->console_renderer,
            (HANDLE)context->console_surface.native_handle, context->presentation, TRUE);
    }
}
