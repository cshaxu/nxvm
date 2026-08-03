/* Copyright 2012-2014 Neko. */

#include <tchar.h>

#include "type.h"

#include "core/product/utils.h"

#include "vm/platform/win32/win32.h"
#include "vm/platform/win32/w32adisp.h"
#include "vm/platform/win32/win32app.h"

#define TIMER_PAINT 0

typedef struct win32app_run_handle {
    vm_platform_run_handle *owner;
    const vm_platform_run_context *platform;
    HANDLE kernel_thread;
    HANDLE display_thread;
    HWND window;
    HINSTANCE instance;
    C_INT initial_flip;
    C_INT display_ready;
    C_INT display_failed;
    C_INT stop_requested;
} win32app_run_handle;

static LRESULT CALLBACK win32app_window_procedure(HWND window, UINT message,
    WPARAM wParam, LPARAM lParam)
{
    win32app_run_handle *handle;
    PAINTSTRUCT paint;
    UCHAR scan_code;
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
    case WM_DESTROY:
        vm_platform_execution_stop_for(handle->platform->execution);
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
        scan_code = (UCHAR)((lParam >> 16) & 0xff);
        virtual_key = (UCHAR)(wParam & 0xff);
        vm_platform_win32_keyboard_make_key_for(handle->platform, scan_code,
            virtual_key);
        return 0;
    case WM_KEYUP:
    case WM_SYSKEYUP:
    case WM_SETFOCUS:
        vm_platform_win32_keyboard_make_status_for(handle->platform);
        return 0;
    default:
        return DefWindowProc(window, message, wParam, lParam);
    }
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
    if (!win32app_register_class(handle)) {
        InterlockedExchange((volatile LONG *)&handle->display_failed, 1);
        return 1;
    }
    handle->window = CreateWindow(_T("nxvm"), _T("Neko's x86 Virtual Machine"),
        style, CW_USEDEFAULT, 0, 888, 484, STD_NULL, STD_NULL, handle->instance,
        handle);
    if (handle->window == STD_NULL) {
        InterlockedExchange((volatile LONG *)&handle->display_failed, 1);
        return 1;
    }
    platform->window_surface.native_handle = handle->window;
    platform->window_renderer = w32adisp_context_create();
    if (platform->window_renderer == STD_NULL) {
        DestroyWindow(handle->window);
        InterlockedExchange((volatile LONG *)&handle->display_failed, 1);
        return 1;
    }
    w32adispInit((w32adisp_context *)platform->window_renderer, handle->window,
        platform->presentation);
    InterlockedExchange((volatile LONG *)&handle->display_ready, 1);
    while (!handle->stop_requested && handle->initial_flip ==
            vm_platform_execution_get_flip_for(handle->platform->execution)) {
        core_product_utils_sleep(handle->platform->wait_scope, 100u);
    }
    if (handle->stop_requested) {
        DestroyWindow(handle->window);
        return 0;
    }
    ShowWindow(handle->window, SW_SHOW);
    UpdateWindow(handle->window);
    while (GetMessage(&message, STD_NULL, 0, 0) > 0) {
        TranslateMessage(&message);
        DispatchMessage(&message);
    }
    return 0;
}

static DWORD WINAPI win32app_kernel_thread(LPVOID opaque)
{
    win32app_run_handle *handle = opaque;

    vm_platform_execution_start_for(handle->platform->execution);
    if (handle->window != STD_NULL) PostMessage(handle->window, WM_CLOSE, 0, 0);
    return 0;
}

ntvdm64_status vm_platform_win32app_run_handle_start(
    const vm_platform_run_context *context, vm_platform_run_handle *owner)
{
    win32app_run_handle *handle;
    DWORD thread_id;

    if (context == STD_NULL || owner == STD_NULL || owner->active ||
        context->execution == STD_NULL || context->keyboard == STD_NULL) {
        return NTVDM64_STATUS_INVALID_ARGUMENT;
    }
    handle = (win32app_run_handle *)STD_CALLOC(1u, sizeof(*handle));
    if (handle == STD_NULL) return NTVDM64_STATUS_NO_MEMORY;
    handle->owner = owner;
    handle->platform = context;
    handle->initial_flip = vm_platform_execution_get_flip_for(context->execution);
    owner->context = context;
    owner->backend = handle;
    owner->window_display = 1;
    owner->active = 1;
    handle->display_thread = CreateThread(STD_NULL, 0, win32app_display_thread,
        handle, 0, &thread_id);
    if (handle->display_thread == STD_NULL) {
        vm_platform_win32app_run_handle_finalize(owner);
        return NTVDM64_STATUS_INVALID_STATE;
    }
    while (!handle->display_ready && !handle->display_failed) {
        core_product_utils_sleep(context->wait_scope, 1u);
    }
    if (!handle->display_ready) {
        vm_platform_win32app_run_handle_request_stop(owner);
        vm_platform_win32app_run_handle_join(owner);
        vm_platform_win32app_run_handle_finalize(owner);
        return NTVDM64_STATUS_INVALID_STATE;
    }
    handle->kernel_thread = CreateThread(STD_NULL, 0, win32app_kernel_thread,
        handle, 0, &thread_id);
    if (handle->kernel_thread == STD_NULL) {
        vm_platform_win32app_run_handle_request_stop(owner);
        vm_platform_win32app_run_handle_join(owner);
        vm_platform_win32app_run_handle_finalize(owner);
        return NTVDM64_STATUS_INVALID_STATE;
    }
    return NTVDM64_STATUS_OK;
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
    if (platform->window_renderer != STD_NULL) {
        w32adispFinal((w32adisp_context *)platform->window_renderer);
        w32adisp_context_destroy((w32adisp_context *)platform->window_renderer);
    }
    platform->window_renderer = STD_NULL;
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
