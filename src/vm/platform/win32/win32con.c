/* Copyright 2012-2014 Neko. */

/* WIN32CON provides win32 console i/o interface. */

#include "core/product/utils.h"
#include "vm/platform/win32/win32.h"
#include "vm/platform/win32/w32cdisp.h"
#include "vm/platform/win32/win32con.h"

HANDLE hOut;

typedef struct win32con_run_context {
    const vm_platform_run_context *platform;
    HANDLE input;
} win32con_run_context;

static DWORD WINAPI ThreadDisplay(LPVOID lpParam) {
    const win32con_run_context *context = lpParam;

    w32cdispInit(context->platform->presentation);
    w32cdispPaint(context->platform->presentation, TRUE);
    while (vm_platform_execution_is_running_for(context->platform->execution)) {
        w32cdispPaint(context->platform->presentation, FALSE);
        utilsSleep(100);
    }
    w32cdispFinal();
    return 0;
}

static DWORD WINAPI ThreadKernel(LPVOID lpParam) {
    const win32con_run_context *context = lpParam;

    vm_platform_execution_start_for(context->platform->execution);
    return 0;
}

static VOID w32ckeybProcess(const win32con_run_context *context) {
    DWORD res;
    INPUT_RECORD inRec;
    UCHAR scanCode, virtualKey;

    GetNumberOfConsoleInputEvents(context->input, &res);
    if (!res) {
        return;
    }
    ReadConsoleInput(context->input, &inRec, 1, &res);
    switch (inRec.EventType) {
    case KEY_EVENT:
        scanCode = (UCHAR) inRec.Event.KeyEvent.wVirtualScanCode;
        virtualKey = (UCHAR) inRec.Event.KeyEvent.wVirtualKeyCode;
        if (inRec.Event.KeyEvent.bKeyDown) {
            win32KeyboardMakeKeyFor(context->platform, scanCode, virtualKey);
        } else {
            win32KeyboardMakeStatusFor(context->platform);
        }
        break;
    case FOCUS_EVENT:
        win32KeyboardMakeStatusFor(context->platform);
        break;
    default:
        break;
    }
}

VOID win32conDisplaySetScreen(const vm_platform_run_context *context) {
    w32cdispSetScreen(context->presentation);
}

VOID win32conDisplayPaint(const vm_platform_run_context *context) {
    w32cdispPaint(context->presentation, TRUE);
}

VOID win32conStartMachine(const vm_platform_run_context *context) {
    DWORD ThreadIdDisplay;
    DWORD ThreadIdKernel;
    win32con_run_context run_context;
    BOOL oldDeviceFlip;

    if (context == NULL || context->execution == NULL ||
        context->keyboard == NULL) return;
    run_context.platform = context;
    run_context.input = GetStdHandle(STD_INPUT_HANDLE);
    oldDeviceFlip = vm_platform_execution_get_flip_for(context->execution);
    hOut = GetStdHandle(STD_OUTPUT_HANDLE);
    CreateThread(NULL, 0, ThreadKernel, &run_context, 0, &ThreadIdKernel);
    while (oldDeviceFlip == vm_platform_execution_get_flip_for(context->execution)) {
        utilsSleep(100);
    }
    CreateThread(NULL, 0, ThreadDisplay, &run_context, 0, &ThreadIdDisplay);
    while (vm_platform_execution_is_running_for(context->execution)) {
        utilsSleep(20);
        w32ckeybProcess(&run_context);
    }
}
