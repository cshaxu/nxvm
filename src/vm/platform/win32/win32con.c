/* Copyright 2012-2014 Neko. */

#include "type.h"

#include "core/platform/wait_interface.h"

#include "vm/platform/win32/win32.h"
#include "vm/platform/win32/w32cdisp.h"
#include "vm/platform/win32/win32con.h"

#include "vm/platform/platform_internal.h"

typedef struct win32con_run_handle {
    vm_platform_run_handle *owner;
    const vm_platform_run_context *platform;
    HANDLE input;
    HANDLE output;
    HANDLE kernel_thread;
    HANDLE display_thread;
} win32con_run_handle;

static vm_platform_host_surface_lease win32_console_lease = {
    ATOMIC_VAR_INIT(0)
};

static C_INT win32con_test_should_fail(C_INT stage)
{
#if defined(VM_PLATFORM_TEST_FAILURE_STAGE)
    return VM_PLATFORM_TEST_FAILURE_STAGE == stage;
#else
    (C_VOID)stage;
    return 0;
#endif
}

static C_INT win32con_display_wait_cancelled(C_VOID *context)
{
    const win32con_run_handle *handle = context;

    return handle == STD_NULL || !vm_platform_execution_is_running_for(
        handle->platform->execution);
}

static C_VOID win32con_process_input(const win32con_run_handle *handle)
{
    DWORD count;
    INPUT_RECORD input;
    type_unsigned_16 scan_code;
    UCHAR virtual_key;

    GetNumberOfConsoleInputEvents(handle->input, &count);
    if (count == 0u || !ReadConsoleInput(handle->input, &input, 1u, &count)) return;
    switch (input.EventType) {
    case KEY_EVENT:
        scan_code = vm_platform_win32con_decode_scan_code(
            input.Event.KeyEvent.wVirtualScanCode,
            input.Event.KeyEvent.dwControlKeyState);
        virtual_key = (UCHAR)input.Event.KeyEvent.wVirtualKeyCode;
        if (scan_code == 0u && input.Event.KeyEvent.bKeyDown != 0 &&
            input.Event.KeyEvent.uChar.UnicodeChar != L'\0') {
            vm_platform_win32_keyboard_make_character_for(handle->platform,
                input.Event.KeyEvent.uChar.UnicodeChar);
        } else vm_platform_win32_keyboard_make_key_for(handle->platform, handle->owner,
            scan_code, virtual_key, input.Event.KeyEvent.bKeyDown != 0);
        break;
    case FOCUS_EVENT:
        break;
    default:
        break;
    }
}

type_unsigned_16 vm_platform_win32con_decode_scan_code(type_unsigned_16 raw_scan_code,
    DWORD control_key_state)
{
    if ((control_key_state & ENHANCED_KEY) != 0u) raw_scan_code |= 0x0100u;
    return raw_scan_code;
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
        (C_VOID)core_platform_wait_milliseconds(20u,
            win32con_display_wait_cancelled, handle);
    }
    return 0;
}

static DWORD WINAPI win32con_kernel_thread(LPVOID opaque)
{
    win32con_run_handle *handle = opaque;

    if (!win32con_test_should_fail(9)) {
        vm_platform_execution_start_for(handle->platform->execution);
    }
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
        context->execution == STD_NULL || context->input_sink.submit == STD_NULL) {
        return TYPE_STATUS_INVALID_ARGUMENT;
    }
    if (vm_platform_host_surface_lease_acquire(&win32_console_lease,
            context) != TYPE_STATUS_OK) return TYPE_STATUS_INVALID_STATE;
    handle = (win32con_run_handle *)STD_CALLOC(1u, sizeof(*handle));
    if (handle == STD_NULL) {
        vm_platform_host_surface_lease_release(&win32_console_lease, context);
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
    if (!vm_platform_execution_wait_for_flip_for(context->execution, old_flip,
            VM_PLATFORM_EXECUTION_FLIP_TIMEOUT_MILLISECONDS)) {
        vm_platform_win32con_run_handle_request_stop(owner);
        vm_platform_win32con_run_handle_join(owner);
        vm_platform_win32con_run_handle_finalize(owner);
        return TYPE_STATUS_INVALID_STATE;
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
    vm_platform_host_surface_lease_release(&win32_console_lease, handle->platform);
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
