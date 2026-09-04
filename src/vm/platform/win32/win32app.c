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

struct vm_platform_win32_window_presenter {
    vm_platform_run_handle *owner;
    const vm_platform_run_context *platform;
    HANDLE display_thread;
    HWND window;
    HINSTANCE instance;
    C_INT initial_flip;
    C_INT wait_for_execution_start;
    C_INT mouse_position_valid;
    type_signed_16 mouse_x;
    type_signed_16 mouse_y;
    volatile LONG display_ready;
    volatile LONG display_failed;
    volatile LONG stop_requested;
    volatile LONG hidden_for_pause;
    volatile LONG pause_acknowledged;
    core_platform_win32_keyboard_normalizer keyboard_normalizer;
};

typedef struct win32app_run_handle {
    vm_platform_run_handle *owner;
    const vm_platform_run_context *platform;
    HANDLE kernel_thread;
    vm_platform_win32_window_presenter *presenter;
} win32app_run_handle;

static type_unsigned_8 win32app_mouse_buttons(WPARAM w_param)
{
    type_unsigned_8 buttons = 0u;

    if ((w_param & MK_LBUTTON) != 0u) buttons |= 0x01u;
    if ((w_param & MK_RBUTTON) != 0u) buttons |= 0x02u;
    if ((w_param & MK_MBUTTON) != 0u) buttons |= 0x04u;
    return buttons;
}

static C_INT win32app_atomic_read(const volatile LONG *value)
{
    return (C_INT)InterlockedCompareExchange((volatile LONG *)value, 0, 0);
}

static C_VOID win32app_submit_mouse_event(
    vm_platform_win32_window_presenter *presenter, WPARAM w_param,
    LPARAM l_param, C_INT force)
{
    type_signed_16 x;
    type_signed_16 y;
    type_signed_16 delta_x = 0;
    type_signed_16 delta_y = 0;

    if (presenter == STD_NULL) return;
    x = (type_signed_16)(short)LOWORD(l_param);
    y = (type_signed_16)(short)HIWORD(l_param);
    if (presenter->mouse_position_valid) {
        delta_x = (type_signed_16)(x - presenter->mouse_x);
        delta_y = (type_signed_16)(y - presenter->mouse_y);
    }
    presenter->mouse_x = x;
    presenter->mouse_y = y;
    presenter->mouse_position_valid = 1;
    if (force || delta_x != 0 || delta_y != 0) {
        vm_platform_win32_mouse_relative_for(presenter->platform, delta_x,
            delta_y, win32app_mouse_buttons(w_param));
    }
}

static C_INT win32app_start_wait_cancelled(C_VOID *context)
{
    const vm_platform_win32_window_presenter *presenter = context;

    return presenter == STD_NULL || win32app_atomic_read(
        &presenter->stop_requested) || presenter->initial_flip !=
        vm_platform_execution_get_flip_for(presenter->platform->execution);
}

static C_INT win32app_display_ready_wait_cancelled(C_VOID *context)
{
    const vm_platform_win32_window_presenter *presenter = context;

    return presenter == STD_NULL || win32app_atomic_read(
        &presenter->display_ready) || win32app_atomic_read(
        &presenter->display_failed);
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
    WPARAM w_param, LPARAM l_param)
{
    vm_platform_win32_window_presenter *presenter;
    PAINTSTRUCT paint;
    type_unsigned_16 scan_code;
    UCHAR virtual_key;

    if (message == WM_NCCREATE) {
        presenter = ((CREATESTRUCT *)l_param)->lpCreateParams;
        SetWindowLongPtr(window, GWLP_USERDATA, (LONG_PTR)presenter);
        return TRUE;
    }
    presenter = (vm_platform_win32_window_presenter *)GetWindowLongPtr(
        window, GWLP_USERDATA);
    if (presenter == STD_NULL) return DefWindowProc(window, message, w_param,
        l_param);

    switch (message) {
    case WM_CREATE:
        SetTimer(window, TIMER_PAINT, 50u, STD_NULL);
        return 0;
    case WM_CLOSE:
        if (win32app_atomic_read(&presenter->stop_requested)) {
            DestroyWindow(window);
        } else if (!win32app_atomic_read(&presenter->hidden_for_pause)) {
            InterlockedExchange(&presenter->hidden_for_pause, 1);
            vm_platform_run_handle_report(presenter->owner,
                VM_PLATFORM_RUN_EVENT_PAUSE_REQUESTED);
            ShowWindow(window, SW_HIDE);
        }
        return 0;
    case WM_DESTROY:
        if (!win32app_atomic_read(&presenter->stop_requested) &&
            vm_platform_run_handle_get_last_event(presenter->owner) !=
            VM_PLATFORM_RUN_EVENT_PAUSE_REQUESTED) {
            vm_platform_run_handle_report(presenter->owner,
                VM_PLATFORM_RUN_EVENT_DISPLAY_COMPLETED);
        }
        PostQuitMessage(0);
        return 0;
    case WM_TIMER:
        if (win32app_atomic_read(&presenter->hidden_for_pause)) {
            if (!vm_platform_execution_is_running_for(presenter->platform->execution)) {
                InterlockedExchange(&presenter->pause_acknowledged, 1);
            } else if (win32app_atomic_read(&presenter->pause_acknowledged)) {
                InterlockedExchange(&presenter->hidden_for_pause, 0);
                InterlockedExchange(&presenter->pause_acknowledged, 0);
                ShowWindow(window, SW_SHOW);
                UpdateWindow(window);
                SetForegroundWindow(window);
                SetFocus(window);
            }
        }
        if (w_param == TIMER_PAINT && vm_platform_execution_is_running_for(
                presenter->platform->execution)) {
            w32adispPaint((w32adisp_context *)presenter->platform->window_renderer,
                window, presenter->platform->presentation, FALSE);
        }
        return 0;
    case WM_PAINT:
        BeginPaint(window, &paint);
        if (vm_platform_execution_is_running_for(presenter->platform->execution)) {
            w32adispPaint((w32adisp_context *)presenter->platform->window_renderer,
                window, presenter->platform->presentation, TRUE);
        }
        EndPaint(window, &paint);
        return 0;
    case WM_KEYDOWN:
    case WM_SYSKEYDOWN:
        scan_code = vm_platform_win32app_decode_scan_code(l_param);
        virtual_key = (type_unsigned_16)(w_param & 0xffffu);
        if (scan_code == 0u) core_platform_win32_keyboard_note_recovered_key(
            &presenter->keyboard_normalizer, virtual_key);
        vm_platform_win32_keyboard_make_key_for(presenter->platform,
            presenter->owner, scan_code, virtual_key, 1);
        return 0;
    case WM_KEYUP:
    case WM_SYSKEYUP:
        scan_code = vm_platform_win32app_decode_scan_code(l_param);
        virtual_key = (type_unsigned_16)(w_param & 0xffffu);
        if (scan_code == 0u) core_platform_win32_keyboard_release_recovered_key(
            &presenter->keyboard_normalizer, virtual_key);
        vm_platform_win32_keyboard_make_key_for(presenter->platform,
            presenter->owner, scan_code, virtual_key, 0);
        return 0;
    case WM_CHAR:
        if (((type_unsigned_32)l_param >> 16u & 0xffu) == 0u &&
            !core_platform_win32_keyboard_consume_duplicate_character(
                &presenter->keyboard_normalizer,
                (type_unsigned_16)(w_param & 0xffffu))) {
            vm_platform_win32_keyboard_make_utf16_for(
                &presenter->keyboard_normalizer, presenter->platform,
                (type_unsigned_16)(w_param & 0xffffu));
        }
        return 0;
    case WM_UNICHAR:
        if (w_param != UNICODE_NOCHAR) vm_platform_win32_keyboard_make_character_for(
            presenter->platform, (type_unsigned_32)w_param);
        return TRUE;
    case WM_MOUSEMOVE:
        win32app_submit_mouse_event(presenter, w_param, l_param, 0);
        return 0;
    case WM_LBUTTONDOWN:
    case WM_LBUTTONUP:
    case WM_RBUTTONDOWN:
    case WM_RBUTTONUP:
    case WM_MBUTTONDOWN:
    case WM_MBUTTONUP:
        win32app_submit_mouse_event(presenter, w_param, l_param, 1);
        return 0;
    case WM_SYSCHAR:
    case WM_SYSDEADCHAR:
        return 0;
    default:
        return DefWindowProc(window, message, w_param, l_param);
    }
}

type_unsigned_16 vm_platform_win32app_decode_scan_code(LPARAM l_param)
{
    type_unsigned_16 scan_code = (type_unsigned_16)((l_param >> 16) & 0xff);

    if ((l_param & (1L << 24)) != 0) scan_code |= 0x0100u;
    return scan_code;
}

static C_INT win32app_register_class(
    const vm_platform_win32_window_presenter *presenter)
{
    WNDCLASSEX window_class = {0};

    window_class.cbSize = sizeof(window_class);
    window_class.style = CS_HREDRAW | CS_VREDRAW;
    window_class.lpfnWndProc = win32app_window_procedure;
    window_class.hInstance = presenter->instance;
    window_class.hCursor = LoadCursor(STD_NULL, IDC_ARROW);
    window_class.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);
    window_class.lpszClassName = _T("nxvm");
    return RegisterClassEx(&window_class) != 0 ||
        GetLastError() == ERROR_CLASS_ALREADY_EXISTS;
}

static DWORD WINAPI win32app_display_thread(LPVOID opaque)
{
    vm_platform_win32_window_presenter *presenter = opaque;
    vm_platform_run_context *platform = (vm_platform_run_context *)presenter->platform;
    MSG message;
    DWORD style = WS_THICKFRAME | WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU |
        WS_MINIMIZEBOX | WS_MAXIMIZEBOX;

    presenter->instance = GetModuleHandle(STD_NULL);
    if (win32app_test_should_fail(2) || !win32app_register_class(presenter)) {
        InterlockedExchange(&presenter->display_failed, 1);
        vm_platform_run_handle_report(presenter->owner,
            VM_PLATFORM_RUN_EVENT_STARTUP_FAILED);
        return 1;
    }
    presenter->window = win32app_test_should_fail(3) ? STD_NULL : CreateWindow(
        _T("nxvm"), _T("Neko's x86 Virtual Machine"), style, CW_USEDEFAULT,
        0, 888, 484, STD_NULL, STD_NULL, presenter->instance, presenter);
    if (presenter->window == STD_NULL) {
        InterlockedExchange(&presenter->display_failed, 1);
        vm_platform_run_handle_report(presenter->owner,
            VM_PLATFORM_RUN_EVENT_STARTUP_FAILED);
        return 1;
    }
    platform->window_surface.native_handle = presenter->window;
    platform->window_renderer = win32app_test_should_fail(4) ? STD_NULL :
        w32adisp_context_create();
    if (platform->window_renderer == STD_NULL) {
        DestroyWindow(presenter->window);
        InterlockedExchange(&presenter->display_failed, 1);
        vm_platform_run_handle_report(presenter->owner,
            VM_PLATFORM_RUN_EVENT_STARTUP_FAILED);
        return 1;
    }
    w32adispInit((w32adisp_context *)platform->window_renderer, presenter->window,
        platform->presentation, platform->font_path);
    InterlockedExchange(&presenter->display_ready, 1);
    if (presenter->wait_for_execution_start) {
        while (!win32app_atomic_read(&presenter->stop_requested) &&
            presenter->initial_flip == vm_platform_execution_get_flip_for(
                presenter->platform->execution)) {
            (C_VOID)core_platform_wait_milliseconds(100u,
                win32app_start_wait_cancelled, presenter);
        }
    }
    if (win32app_atomic_read(&presenter->stop_requested)) {
        win32app_display_renderer_finalize(platform);
        platform->window_surface.native_handle = STD_NULL;
        DestroyWindow(presenter->window);
        return 0;
    }
    ShowWindow(presenter->window, SW_SHOW);
    UpdateWindow(presenter->window);
    SetForegroundWindow(presenter->window);
    SetFocus(presenter->window);
    while (GetMessage(&message, STD_NULL, 0, 0) > 0) {
        TranslateMessage(&message);
        DispatchMessage(&message);
    }
    win32app_display_renderer_finalize(platform);
    platform->window_surface.native_handle = STD_NULL;
    return 0;
}

type_status vm_platform_win32_window_presenter_start(
    const vm_platform_run_context *context, vm_platform_run_handle *owner,
    C_INT wait_for_execution_start,
    vm_platform_win32_window_presenter **out_presenter)
{
    vm_platform_win32_window_presenter *presenter;
    DWORD thread_id;

    if (out_presenter == STD_NULL) return TYPE_STATUS_INVALID_ARGUMENT;
    *out_presenter = STD_NULL;
    if (context == STD_NULL || owner == STD_NULL || context->execution == STD_NULL ||
        context->input_sink.submit == STD_NULL || context->window_renderer != STD_NULL) {
        return TYPE_STATUS_INVALID_ARGUMENT;
    }
    presenter = STD_CALLOC(1u, sizeof(*presenter));
    if (presenter == STD_NULL) return TYPE_STATUS_NO_MEMORY;
    presenter->owner = owner;
    presenter->platform = context;
    presenter->initial_flip = vm_platform_execution_get_flip_for(context->execution);
    presenter->wait_for_execution_start = wait_for_execution_start;
    presenter->display_thread = win32app_test_should_fail(1) ? STD_NULL :
        CreateThread(STD_NULL, 0, win32app_display_thread, presenter, 0,
            &thread_id);
    if (presenter->display_thread == STD_NULL) {
        STD_FREE(presenter);
        return TYPE_STATUS_INVALID_STATE;
    }
    (C_VOID)core_platform_wait_milliseconds(
        WIN32APP_DISPLAY_READY_TIMEOUT_MILLISECONDS,
        win32app_display_ready_wait_cancelled, presenter);
    if (!win32app_atomic_read(&presenter->display_ready)) {
        vm_platform_win32_window_presenter_request_stop(presenter);
        vm_platform_win32_window_presenter_join(presenter);
        vm_platform_win32_window_presenter_destroy(presenter);
        return TYPE_STATUS_INVALID_STATE;
    }
    *out_presenter = presenter;
    return TYPE_STATUS_OK;
}

C_VOID vm_platform_win32_window_presenter_request_stop(
    vm_platform_win32_window_presenter *presenter)
{
    if (presenter == STD_NULL) return;
    InterlockedExchange(&presenter->stop_requested, 1);
    if (presenter->window != STD_NULL) PostMessage(presenter->window, WM_CLOSE,
        0, 0);
}

C_VOID vm_platform_win32_window_presenter_join(
    vm_platform_win32_window_presenter *presenter)
{
    if (presenter != STD_NULL && presenter->display_thread != STD_NULL) {
        WaitForSingleObject(presenter->display_thread, INFINITE);
    }
}

C_VOID vm_platform_win32_window_presenter_destroy(
    vm_platform_win32_window_presenter *presenter)
{
    vm_platform_run_context *platform;

    if (presenter == STD_NULL) return;
    platform = (vm_platform_run_context *)presenter->platform;
    if (presenter->display_thread != STD_NULL) CloseHandle(presenter->display_thread);
    win32app_display_renderer_finalize(platform);
    platform->window_surface.native_handle = STD_NULL;
    STD_FREE(presenter);
}

static DWORD WINAPI win32app_kernel_thread(LPVOID opaque)
{
    win32app_run_handle *handle = opaque;

    vm_platform_execution_start_for(handle->platform->execution);
    vm_platform_run_handle_report(handle->owner,
        VM_PLATFORM_RUN_EVENT_KERNEL_COMPLETED);
    vm_platform_win32_window_presenter_request_stop(handle->presenter);
    return 0;
}

type_status vm_platform_win32app_run_handle_start(
    const vm_platform_run_context *context, vm_platform_run_handle *owner)
{
    win32app_run_handle *handle;
    type_status status;
    DWORD thread_id;

    if (context == STD_NULL || owner == STD_NULL || owner->active) {
        return TYPE_STATUS_INVALID_ARGUMENT;
    }
    handle = STD_CALLOC(1u, sizeof(*handle));
    if (handle == STD_NULL) return TYPE_STATUS_NO_MEMORY;
    handle->owner = owner;
    handle->platform = context;
    owner->context = context;
    owner->backend = handle;
    owner->window_display = 1;
    owner->active = 1;
    status = vm_platform_win32_window_presenter_start(context, owner, TYPE_TRUE,
        &handle->presenter);
    if (status != TYPE_STATUS_OK) {
        vm_platform_win32app_run_handle_finalize(owner);
        return status;
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
    vm_platform_execution_stop_for(handle->platform->execution);
    vm_platform_win32_window_presenter_request_stop(handle->presenter);
}

C_VOID vm_platform_win32app_run_handle_join(vm_platform_run_handle *owner)
{
    win32app_run_handle *handle = owner == STD_NULL ? STD_NULL : owner->backend;

    if (handle == STD_NULL) return;
    if (handle->kernel_thread != STD_NULL) WaitForSingleObject(handle->kernel_thread,
        INFINITE);
    vm_platform_win32_window_presenter_join(handle->presenter);
    owner->active = 0;
}

C_VOID vm_platform_win32app_run_handle_finalize(vm_platform_run_handle *owner)
{
    win32app_run_handle *handle = owner == STD_NULL ? STD_NULL : owner->backend;

    if (handle == STD_NULL) return;
    if (handle->kernel_thread != STD_NULL) CloseHandle(handle->kernel_thread);
    vm_platform_win32_window_presenter_destroy(handle->presenter);
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
