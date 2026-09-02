/* Copyright 2012-2014 Neko. */

#include <tchar.h>

#include "type.h"

#include "core/platform/wait_interface.h"

#include "vm/platform/win32/win32.h"
#include "vm/platform/win32/w32adisp.h"
#include "vm/platform/win32/win32app.h"

#include "vm/platform/platform_internal.h"

#define TIMER_PAINT 0
#define WIN32APP_DISPLAY_READY_TIMEOUT_MILLISECONDS 5000u

typedef struct win32app_run_handle {
    vm_platform_run_handle *owner;
    const vm_platform_run_context *platform;
    HANDLE kernel_thread;
    HANDLE display_thread;
    HWND window;
    HINSTANCE instance;
    C_INT initial_flip;
    C_INT mouse_position_valid;
    type_signed_16 mouse_x;
    type_signed_16 mouse_y;
    volatile LONG display_ready;
    volatile LONG display_failed;
    volatile LONG stop_requested;
    core_platform_win32_keyboard_normalizer keyboard_normalizer;
} win32app_run_handle;

static type_unsigned_8 win32app_mouse_buttons(WPARAM w_param)
{
    type_unsigned_8 buttons = 0u;

    if ((w_param & MK_LBUTTON) != 0u) buttons |= 0x01u;
    if ((w_param & MK_RBUTTON) != 0u) buttons |= 0x02u;
    if ((w_param & MK_MBUTTON) != 0u) buttons |= 0x04u;
    return buttons;
}

static C_VOID win32app_submit_mouse_event(win32app_run_handle *handle,
    WPARAM w_param, LPARAM l_param, C_INT force)
{
    type_signed_16 x;
    type_signed_16 y;
    type_signed_16 delta_x = 0;
    type_signed_16 delta_y = 0;

    if (handle == STD_NULL) return;
    x = (type_signed_16)(short)LOWORD(l_param);
    y = (type_signed_16)(short)HIWORD(l_param);
    if (handle->mouse_position_valid) {
        delta_x = (type_signed_16)(x - handle->mouse_x);
        delta_y = (type_signed_16)(y - handle->mouse_y);
    }
    handle->mouse_x = x;
    handle->mouse_y = y;
    handle->mouse_position_valid = 1;
    if (force || delta_x != 0 || delta_y != 0) {
        vm_platform_win32_mouse_relative_for(handle->platform, delta_x, delta_y,
            win32app_mouse_buttons(w_param));
    }
}

static C_INT win32app_atomic_read(const volatile LONG *value)
{
    return (C_INT)InterlockedCompareExchange((volatile LONG *)value, 0, 0);
}

static C_INT win32app_start_wait_cancelled(C_VOID *context)
{
    const win32app_run_handle *handle = context;

    return handle == STD_NULL || win32app_atomic_read(&handle->stop_requested) ||
        handle->initial_flip != vm_platform_execution_get_flip_for(
            handle->platform->execution);
}

static C_INT win32app_display_ready_wait_cancelled(C_VOID *context)
{
    const win32app_run_handle *handle = context;

    return handle == STD_NULL || win32app_atomic_read(&handle->display_ready) ||
        win32app_atomic_read(&handle->display_failed);
}

static C_INT win32app_test_should_fail(C_INT stage)
{
#if defined(VM_PLATFORM_TEST_FAILURE_STAGE)
    return VM_PLATFORM_TEST_FAILURE_STAGE == stage;
#else
    (C_VOID)stage;
    return 0;
#endif
}

static C_VOID win32app_display_renderer_finalize(vm_platform_run_context *platform)
{
    if (platform->window_renderer == STD_NULL) return;
    w32adispFinal((w32adisp_context *)platform->window_renderer);
    w32adisp_context_destroy((w32adisp_context *)platform->window_renderer);
    platform->window_renderer = STD_NULL;
}

static LRESULT CALLBACK win32app_window_procedure(HWND window, UINT message,
    WPARAM wParam, LPARAM lParam)
{
    win32app_run_handle *handle;
    PAINTSTRUCT paint;
    type_unsigned_16 scan_code;
    UCHAR virtual_key;

    if (message == WM_NCCREATE) {
        handle = ((CREATESTRUCT *)lParam)->lpCreateParams;
        SetWindowLongPtr(window, GWLP_USERDATA, (LONG_PTR)handle);
        return TRUE;
    }
    handle = (win32app_run_handle *)GetWindowLongPtr(window, GWLP_USERDATA);
    if (handle == STD_NULL) return DefWindowProc(window, message, wParam, lParam);

    switch (message) {
    case WM_CREATE:
        SetTimer(window, TIMER_PAINT, 50u, STD_NULL);
        return 0;
    case WM_CLOSE:
        win32app_display_renderer_finalize((vm_platform_run_context *)handle->platform);
        return DefWindowProc(window, message, wParam, lParam);
    case WM_DESTROY:
        vm_platform_run_handle_report(handle->owner,
            vm_platform_run_handle_get_last_event(handle->owner) ==
            VM_PLATFORM_RUN_EVENT_KERNEL_COMPLETED ?
            VM_PLATFORM_RUN_EVENT_DISPLAY_COMPLETED :
            VM_PLATFORM_RUN_EVENT_STOP_REQUESTED);
        PostQuitMessage(0);
        return 0;
    case WM_TIMER:
        if (wParam == TIMER_PAINT && vm_platform_execution_is_running_for(
                handle->platform->execution)) {
            w32adispPaint((w32adisp_context *)handle->platform->window_renderer,
                window, handle->platform->presentation, FALSE);
        }
        return 0;
    case WM_PAINT:
        BeginPaint(window, &paint);
        if (vm_platform_execution_is_running_for(handle->platform->execution)) {
            w32adispPaint((w32adisp_context *)handle->platform->window_renderer,
                window, handle->platform->presentation, TRUE);
        }
        EndPaint(window, &paint);
        return 0;
    case WM_KEYDOWN:
    case WM_SYSKEYDOWN:
        scan_code = vm_platform_win32app_decode_scan_code(lParam);
        virtual_key = (type_unsigned_16)(wParam & 0xffff);
        if (scan_code == 0u) core_platform_win32_keyboard_note_recovered_key(
            &handle->keyboard_normalizer, virtual_key);
        vm_platform_win32_keyboard_make_key_for(handle->platform,
            handle->owner, scan_code, virtual_key, 1);
        return 0;
    case WM_KEYUP:
    case WM_SYSKEYUP:
        scan_code = vm_platform_win32app_decode_scan_code(lParam);
        virtual_key = (type_unsigned_16)(wParam & 0xffff);
        if (scan_code == 0u) core_platform_win32_keyboard_release_recovered_key(
            &handle->keyboard_normalizer, virtual_key);
        vm_platform_win32_keyboard_make_key_for(handle->platform,
            handle->owner, scan_code, virtual_key, 0);
        return 0;
    case WM_CHAR:
        /* TranslateMessage carries the originating scan in WM_CHAR. A normal
         * physical key was already sent above; RDP soft keyboards may supply
         * only a character and therefore leave that field clear. */
        if (((type_unsigned_32)lParam >> 16u & 0xffu) == 0u) {
            if (core_platform_win32_keyboard_consume_duplicate_character(
                    &handle->keyboard_normalizer, (type_unsigned_16)(wParam & 0xffffu))) {
                return 0;
            }
            vm_platform_win32_keyboard_make_utf16_for(&handle->keyboard_normalizer,
                handle->platform, (type_unsigned_16)(wParam & 0xffffu));
        }
        return 0;
    case WM_UNICHAR:
        if (wParam != UNICODE_NOCHAR) vm_platform_win32_keyboard_make_character_for(
            handle->platform, (type_unsigned_32)wParam);
        return TRUE;
    case WM_MOUSEMOVE:
        win32app_submit_mouse_event(handle, wParam, lParam, 0);
        return 0;
    case WM_LBUTTONDOWN:
    case WM_LBUTTONUP:
    case WM_RBUTTONDOWN:
    case WM_RBUTTONUP:
    case WM_MBUTTONDOWN:
    case WM_MBUTTONUP:
        /* T229 reports relative motion and buttons only; pointer capture and
         * absolute host coordinates remain explicitly unsupported. */
        win32app_submit_mouse_event(handle, wParam, lParam, 1);
        return 0;
    case WM_SYSCHAR:
    case WM_SYSDEADCHAR:
        /* Guest system-key chords must not fall through to the host menu. */
        return 0;
    case WM_SETFOCUS:
        return 0;
    default:
        return DefWindowProc(window, message, wParam, lParam);
    }
}

type_unsigned_16 vm_platform_win32app_decode_scan_code(LPARAM l_param)
{
    type_unsigned_16 scan_code = (type_unsigned_16)((l_param >> 16) & 0xff);

    if ((l_param & (1L << 24)) != 0) scan_code |= 0x0100u;
    return scan_code;
}

static C_INT win32app_register_class(const win32app_run_handle *handle)
{
    WNDCLASSEX window_class = {0};

    window_class.cbSize = sizeof(window_class);
    window_class.style = CS_HREDRAW | CS_VREDRAW;
    window_class.lpfnWndProc = win32app_window_procedure;
    window_class.hInstance = handle->instance;
    window_class.hCursor = LoadCursor(STD_NULL, IDC_ARROW);
    window_class.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);
    window_class.lpszClassName = _T("nxvm");
    return RegisterClassEx(&window_class) != 0 || GetLastError() == ERROR_CLASS_ALREADY_EXISTS;
}

static DWORD WINAPI win32app_display_thread(LPVOID opaque)
{
    win32app_run_handle *handle = opaque;
    vm_platform_run_context *platform = (vm_platform_run_context *)handle->platform;
    MSG message;
    DWORD style = WS_THICKFRAME | WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU |
        WS_MINIMIZEBOX | WS_MAXIMIZEBOX;

    handle->instance = GetModuleHandle(STD_NULL);
    if (win32app_test_should_fail(2) || !win32app_register_class(handle)) {
        InterlockedExchange((volatile LONG *)&handle->display_failed, 1);
        vm_platform_run_handle_report(handle->owner,
            VM_PLATFORM_RUN_EVENT_STARTUP_FAILED);
        return 1;
    }
    handle->window = win32app_test_should_fail(3) ? STD_NULL :
        CreateWindow(_T("nxvm"), _T("Neko's x86 Virtual Machine"), style,
            CW_USEDEFAULT, 0, 888, 484, STD_NULL, STD_NULL, handle->instance,
            handle);
    if (handle->window == STD_NULL) {
        InterlockedExchange((volatile LONG *)&handle->display_failed, 1);
        vm_platform_run_handle_report(handle->owner,
            VM_PLATFORM_RUN_EVENT_STARTUP_FAILED);
        return 1;
    }
    platform->window_surface.native_handle = handle->window;
    platform->window_renderer = win32app_test_should_fail(4) ? STD_NULL :
        w32adisp_context_create();
    if (platform->window_renderer == STD_NULL) {
        DestroyWindow(handle->window);
        InterlockedExchange((volatile LONG *)&handle->display_failed, 1);
        vm_platform_run_handle_report(handle->owner,
            VM_PLATFORM_RUN_EVENT_STARTUP_FAILED);
        return 1;
    }
    w32adispInit((w32adisp_context *)platform->window_renderer, handle->window,
        platform->presentation, platform->font_path);
    InterlockedExchange((volatile LONG *)&handle->display_ready, 1);
    while (!win32app_atomic_read(&handle->stop_requested) && handle->initial_flip ==
            vm_platform_execution_get_flip_for(handle->platform->execution)) {
        (C_VOID)core_platform_wait_milliseconds(100u,
            win32app_start_wait_cancelled, handle);
    }
    if (win32app_atomic_read(&handle->stop_requested)) {
        win32app_display_renderer_finalize(platform);
        DestroyWindow(handle->window);
        return 0;
    }
    ShowWindow(handle->window, SW_SHOW);
    UpdateWindow(handle->window);
    /* A Window session owns guest input once its display becomes visible. */
    SetForegroundWindow(handle->window);
    SetFocus(handle->window);
    while (GetMessage(&message, STD_NULL, 0, 0) > 0) {
        TranslateMessage(&message);
        DispatchMessage(&message);
    }
    win32app_display_renderer_finalize(platform);
    return 0;
}

static DWORD WINAPI win32app_kernel_thread(LPVOID opaque)
{
    win32app_run_handle *handle = opaque;

    vm_platform_execution_start_for(handle->platform->execution);
    vm_platform_run_handle_report(handle->owner,
        VM_PLATFORM_RUN_EVENT_KERNEL_COMPLETED);
    if (handle->window != STD_NULL) PostMessage(handle->window, WM_CLOSE, 0, 0);
    return 0;
}

type_status vm_platform_win32app_run_handle_start(
    const vm_platform_run_context *context, vm_platform_run_handle *owner)
{
    win32app_run_handle *handle;
    DWORD thread_id;

    if (context == STD_NULL || owner == STD_NULL || owner->active ||
        context->execution == STD_NULL || context->input_sink.submit == STD_NULL) {
        return TYPE_STATUS_INVALID_ARGUMENT;
    }
    handle = (win32app_run_handle *)STD_CALLOC(1u, sizeof(*handle));
    if (handle == STD_NULL) return TYPE_STATUS_NO_MEMORY;
    handle->owner = owner;
    handle->platform = context;
    handle->initial_flip = vm_platform_execution_get_flip_for(context->execution);
    owner->context = context;
    owner->backend = handle;
    owner->window_display = 1;
    owner->active = 1;
    handle->display_thread = win32app_test_should_fail(1) ? STD_NULL :
        CreateThread(STD_NULL, 0, win32app_display_thread, handle, 0, &thread_id);
    if (handle->display_thread == STD_NULL) {
        vm_platform_win32app_run_handle_finalize(owner);
        return TYPE_STATUS_INVALID_STATE;
    }
    (C_VOID)core_platform_wait_milliseconds(
        WIN32APP_DISPLAY_READY_TIMEOUT_MILLISECONDS,
        win32app_display_ready_wait_cancelled, handle);
    if (!win32app_atomic_read(&handle->display_ready)) {
        vm_platform_win32app_run_handle_request_stop(owner);
        vm_platform_win32app_run_handle_join(owner);
        vm_platform_win32app_run_handle_finalize(owner);
        return TYPE_STATUS_INVALID_STATE;
    }
    handle->kernel_thread = win32app_test_should_fail(5) ? STD_NULL :
        CreateThread(STD_NULL, 0, win32app_kernel_thread, handle, 0, &thread_id);
    if (handle->kernel_thread == STD_NULL) {
        vm_platform_win32app_run_handle_request_stop(owner);
        vm_platform_win32app_run_handle_join(owner);
        vm_platform_win32app_run_handle_finalize(owner);
        return TYPE_STATUS_INVALID_STATE;
    }
    return TYPE_STATUS_OK;
}

C_VOID vm_platform_win32app_run_handle_request_stop(vm_platform_run_handle *owner)
{
    win32app_run_handle *handle = owner == STD_NULL ? STD_NULL : owner->backend;

    if (handle == STD_NULL) return;
    InterlockedExchange((volatile LONG *)&handle->stop_requested, 1);
    vm_platform_execution_stop_for(handle->platform->execution);
    if (handle->window != STD_NULL) PostMessage(handle->window, WM_CLOSE, 0, 0);
}

C_VOID vm_platform_win32app_run_handle_join(vm_platform_run_handle *owner)
{
    win32app_run_handle *handle = owner == STD_NULL ? STD_NULL : owner->backend;

    if (handle == STD_NULL) return;
    if (handle->kernel_thread != STD_NULL) WaitForSingleObject(handle->kernel_thread, INFINITE);
    if (handle->display_thread != STD_NULL) WaitForSingleObject(handle->display_thread, INFINITE);
    owner->active = 0;
}

C_VOID vm_platform_win32app_run_handle_finalize(vm_platform_run_handle *owner)
{
    win32app_run_handle *handle = owner == STD_NULL ? STD_NULL : owner->backend;
    vm_platform_run_context *platform;

    if (handle == STD_NULL) return;
    platform = (vm_platform_run_context *)handle->platform;
    if (handle->kernel_thread != STD_NULL) CloseHandle(handle->kernel_thread);
    if (handle->display_thread != STD_NULL) CloseHandle(handle->display_thread);
    platform->window_surface.native_handle = STD_NULL;
    STD_FREE(handle);
    vm_platform_run_handle_initialize(owner);
}

C_VOID vm_platform_win32app_display_set_screen(const vm_platform_run_context *context)
{
    if (context != STD_NULL && context->window_renderer != STD_NULL) {
        w32adispSetScreen((w32adisp_context *)context->window_renderer,
            (HWND)context->window_surface.native_handle, context->presentation);
    }
}

C_VOID vm_platform_win32app_display_paint(const vm_platform_run_context *context)
{
    if (context != STD_NULL && context->window_renderer != STD_NULL) {
        w32adispPaint((w32adisp_context *)context->window_renderer,
            (HWND)context->window_surface.native_handle, context->presentation, TRUE);
    }
}
