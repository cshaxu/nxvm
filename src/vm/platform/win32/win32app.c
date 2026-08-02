/* Copyright 2012-2014 Neko. */

/* WIN32APP provides win32 window i/o interface. */

#include <tchar.h>
#include <stdlib.h>

#include "core/product/utils.h"

#include "vm/platform/win32/win32.h"
#include "vm/platform/win32/w32adisp.h"
#include "vm/platform/win32/win32app.h"

HWND w32aHWnd = NULL; /* handler for window; if null, window is not yet ready */
#define TIMER_PAINT   0
/* #define TIMER_RTC     1 */

typedef struct win32app_run_context {
    const vm_platform_run_context *platform;
    HINSTANCE instance;
    LPCSTR window_class;
    LPCSTR title;
} win32app_run_context;

static LRESULT CALLBACK WndProc(HWND hWnd, UINT message,
                                WPARAM wParam, LPARAM lParam) {
    win32app_run_context *context;
    PAINTSTRUCT ps;
    INT wmId, wmEvent;
    UCHAR scanCode, virtualKey;

    if (message == WM_NCCREATE) {
        context = ((CREATESTRUCT *)lParam)->lpCreateParams;
        SetWindowLongPtr(hWnd, GWLP_USERDATA, (LONG_PTR)context);
        return TRUE;
    }
    context = (win32app_run_context *)GetWindowLongPtr(hWnd,
                                                        GWLP_USERDATA);
    if (context == NULL) return DefWindowProc(hWnd, message, wParam, lParam);

    switch (message) {
    case WM_CREATE:
        SetTimer(hWnd, TIMER_PAINT, 50, NULL);
        break;
    case WM_DESTROY:
        PostQuitMessage(0);
        break;
    case WM_COMMAND:
        wmId    = LOWORD(wParam);
        wmEvent = HIWORD(wParam);
        break;
    case WM_TIMER:
        switch (wParam) {
        case TIMER_PAINT:
            if (vm_platform_execution_is_running_for(
                    context->platform->execution)) {
                w32adispPaint(FALSE);
            }
            break;
        default:
            break;
        }
        break;
    case WM_PAINT:
        BeginPaint(hWnd, &ps);
        if (vm_platform_execution_is_running_for(context->platform->execution)) {
            w32adispPaint(TRUE);
        }
        EndPaint(hWnd, &ps);
        break;
    /*case WM_SIZE: break;
    case WM_SIZING: break;
    case WM_CHAR: win32KeyboardMakeChar(wParam, lParam);break;*/
    case WM_KEYDOWN:
    case WM_SYSKEYDOWN:
        scanCode = (UCHAR)((lParam >> 16) & 0x000000ff);
        virtualKey = (UCHAR)(wParam & 0x000000ff);
        win32KeyboardMakeKeyFor(context->platform, scanCode, virtualKey);
        break;
    case WM_KEYUP:
    case WM_SYSKEYUP:
    case WM_SETFOCUS:
        win32KeyboardMakeStatusFor(context->platform);
        break;
    default:
        return DefWindowProc(hWnd, message, wParam, lParam);
    }
    return (LRESULT) NULL;
}

static ATOM ThreadDisplayRegisterClass(const win32app_run_context *context) {
    WNDCLASSEX wcex;
    wcex.cbSize = sizeof(WNDCLASSEX);
    wcex.style            = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc    = (WNDPROC) WndProc;
    wcex.cbClsExtra        = 0;
    wcex.cbWndExtra        = 0;
    wcex.hInstance        = context->instance;
    wcex.hIcon            = NULL;
    wcex.hCursor        = LoadCursor(NULL, IDC_ARROW);
    wcex.hbrBackground    = (HBRUSH)GetStockObject(BLACK_BRUSH);
    wcex.lpszMenuName    = NULL;
    wcex.lpszClassName    = context->window_class;
    wcex.hIconSm        = NULL;
    return RegisterClassEx(&wcex);
}

static BOOL ThreadDisplayInitInstance(win32app_run_context *context,
                                      INT nCmdShow) {
    DWORD dwStyle = WS_THICKFRAME | WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU |
                    WS_MINIMIZEBOX | WS_MAXIMIZEBOX;
    w32aHWnd = CreateWindow(context->window_class, context->title, dwStyle,
                            CW_USEDEFAULT, 0, 888, 484, NULL, NULL,
                            context->instance, context);
    /* window size is 888 x 484 for "Courier New" */
    if (!w32aHWnd) {
        return FALSE;
    }
    ShowWindow(w32aHWnd, SW_SHOW);
    UpdateWindow(w32aHWnd);
    return TRUE;
}

static DWORD WINAPI ThreadDisplay(LPVOID lpParam) {
    win32app_run_context *context = lpParam;
    MSG msg;
    context->instance = GetModuleHandle(NULL);
    ThreadDisplayRegisterClass(context);
    if (!ThreadDisplayInitInstance(context, 0)) {
        free(context);
        return FALSE;
    }

    w32adispInit();
    while (GetMessage(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
    w32aHWnd = NULL;
    vm_platform_execution_stop_for(context->platform->execution);
    w32adispFinal();
    free(context);
    return 0;
}

static DWORD WINAPI ThreadKernel(LPVOID lpParam) {
    win32app_run_context *context = lpParam;

    vm_platform_execution_start_for(context->platform->execution);
    w32adispPaint(TRUE);
    return 0;
}

VOID win32appDisplaySetScreen() {
    w32adispSetScreen();
}
VOID win32appDisplayPaint() {
    w32adispPaint(TRUE);
}
VOID win32appStartMachine(const vm_platform_run_context *context) {
    win32app_run_context *run_context;
    BOOL oldDeviceFlip;
    DWORD thread_id;

    if (context == NULL || context->execution == NULL ||
        context->keyboard == NULL) return;
    run_context = calloc(1u, sizeof(*run_context));
    if (run_context == NULL) return;
    run_context->platform = context;
    run_context->window_class = _T("nxvm");
    run_context->title = _T("Neko's x86 Virtual Machine");
    oldDeviceFlip = vm_platform_execution_get_flip_for(context->execution);
    CreateThread(NULL, 0, ThreadKernel, run_context, 0, &thread_id);
    while (oldDeviceFlip ==
           vm_platform_execution_get_flip_for(context->execution)) {
        utilsSleep(100);
    }
    CreateThread(NULL, 0, ThreadDisplay, run_context, 0, &thread_id);
}
