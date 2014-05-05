/* Copyright 2012-2014 Neko. */

/* WIN32APP provides win32 window i/o interface. */

#include <tchar.h>

#include "../../utils.h"
#include "../../device/device.h"

#include "win32.h"
#include "w32adisp.h"
#include "win32app.h"

HWND w32aHWnd = NULL; /* handler for window; if null, window is not yet ready */
static DWORD ThreadIdDisplay = 0;
static DWORD ThreadIdKernel = 0;
static HINSTANCE hInstance = NULL;
static LPCSTR szWindowClass = _T("nxvm");
static LPCSTR szTitle = _T("Neko's x86 Virtual Machine");
#define TIMER_PAINT   0
/* #define TIMER_RTC     1 */

static LRESULT CALLBACK WndProc(HWND hWnd, UINT message,
                                WPARAM wParam, LPARAM lParam) {
    PAINTSTRUCT ps;
    INT wmId, wmEvent;
    UCHAR scanCode, virtualKey;
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
            if (device.flagRun) {
                w32adispPaint(FALSE);
            }
            break;
        default:
            break;
        }
        break;
    case WM_PAINT:
        BeginPaint(hWnd, &ps);
        if (device.flagRun) {
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
        win32KeyboardMakeKey(scanCode, virtualKey);
        break;
    case WM_KEYUP:
    case WM_SYSKEYUP:
    case WM_SETFOCUS:
        win32KeyboardMakeStatus();
        break;
    default:
        return DefWindowProc(hWnd, message, wParam, lParam);
    }
    return 0;
}

static ATOM ThreadDisplayRegisterClass(HINSTANCE hInstance) {
    WNDCLASSEX wcex;
    wcex.cbSize = sizeof(WNDCLASSEX);
    wcex.style            = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc    = (WNDPROC)WndProc;
    wcex.cbClsExtra        = 0;
    wcex.cbWndExtra        = 0;
    wcex.hInstance        = hInstance;
    wcex.hIcon            = NULL;
    wcex.hCursor        = LoadCursor(NULL, IDC_ARROW);
    wcex.hbrBackground    = (HBRUSH)GetStockObject(BLACK_BRUSH);
    wcex.lpszMenuName    = NULL;
    wcex.lpszClassName    = szWindowClass;
    wcex.hIconSm        = NULL;
    return RegisterClassEx(&wcex);
}

static BOOL ThreadDisplayInitInstance(HINSTANCE hInstance, int nCmdShow) {
    DWORD dwStyle = WS_THICKFRAME | WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU |
                    WS_MINIMIZEBOX | WS_MAXIMIZEBOX;
    w32aHWnd = CreateWindow(szWindowClass, szTitle, dwStyle, CW_USEDEFAULT,
                            0, 888, 484, NULL, NULL, hInstance, NULL);
    /* window size is 888 x 484 for "Courier New" */
    if (!w32aHWnd) {
        return FALSE;
    }
    ShowWindow(w32aHWnd, SW_SHOW);
    UpdateWindow(w32aHWnd);
    return TRUE;
}

static DWORD WINAPI ThreadDisplay(LPVOID lpParam) {
    MSG msg;
    hInstance = GetModuleHandle(NULL);
    ThreadDisplayRegisterClass(hInstance);
    if (!ThreadDisplayInitInstance(hInstance, 0)) {
        return FALSE;
    }

    w32adispInit();
    while (GetMessage(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
    w32aHWnd = NULL;
    deviceStop();
    w32adispFinal();
    ThreadIdDisplay = 0;
    return 0;
}

static DWORD WINAPI ThreadKernel(LPVOID lpParam) {
    deviceStart();
    w32adispPaint(TRUE);
    ThreadIdKernel = 0;
    return 0;
}

VOID win32appDisplaySetScreen() {
    w32adispSetScreen();
}
VOID win32appDisplayPaint() {
    w32adispPaint(TRUE);
}
VOID win32appStartMachine() {
    BOOL oldDeviceFlip = device.flagFlip;
    CreateThread(NULL, 0, ThreadKernel, NULL, 0, &ThreadIdKernel);
    while (oldDeviceFlip == device.flagFlip) {
        utilsSleep(100);
    }
    if (!ThreadIdDisplay) {
        CreateThread(NULL, 0, ThreadDisplay, NULL, 0, &ThreadIdDisplay);
    }
}
