/* Copyright 2012-2014 Neko. */

/* WIN32CON provides win32 console i/o interface. */

#include "core/product/utils.h"
#include "type.h"
#include "vm/platform/win32/win32.h"
#include "vm/platform/win32/w32cdisp.h"
#include "vm/platform/win32/win32con.h"

typedef struct win32con_run_context {
    const vm_platform_run_context *platform;
    HANDLE input;
    HANDLE output;
} win32con_run_context;

static core_platform_host_surface_lease win32_console_lease = {
    ATOMIC_VAR_INIT(0)
};

static DWORD WINAPI ThreadDisplay(LPVOID lpParam) {
    const win32con_run_context *context = lpParam;

    w32cdispInit((w32cdisp_context *)context->platform->console_renderer,
                 context->output, context->platform->presentation);
    w32cdispPaint((w32cdisp_context *)context->platform->console_renderer,
                  context->output, context->platform->presentation, TRUE);
    while (vm_platform_execution_is_running_for(context->platform->execution)) {
        w32cdispPaint((w32cdisp_context *)context->platform->console_renderer,
                      context->output, context->platform->presentation, FALSE);
        core_product_utils_sleep(context->platform->wait_scope, 100);
    }
    w32cdispFinal((w32cdisp_context *)context->platform->console_renderer,
                  context->output);
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
            vm_platform_win32_keyboard_make_key_for(context->platform, scanCode, virtualKey);
        } else {
            vm_platform_win32_keyboard_make_status_for(context->platform);
        }
        break;
    case FOCUS_EVENT:
        vm_platform_win32_keyboard_make_status_for(context->platform);
        break;
    default:
        break;
    }
}

VOID vm_platform_win32con_display_set_screen(const vm_platform_run_context *context) {
    w32cdispSetScreen((w32cdisp_context *)context->console_renderer,
                      (HANDLE)context->console_surface.native_handle,
                      context->presentation);
}

VOID vm_platform_win32con_display_paint(const vm_platform_run_context *context) {
    w32cdispPaint((w32cdisp_context *)context->console_renderer,
                  (HANDLE)context->console_surface.native_handle,
                  context->presentation, TRUE);
}

VOID vm_platform_win32con_start_machine(const vm_platform_run_context *context) {
    DWORD ThreadIdDisplay;
    DWORD ThreadIdKernel;
    win32con_run_context *run_context;
    BOOL oldDeviceFlip;
    HANDLE kernel_thread;
    HANDLE display_thread;

    if (context == STD_NULL || context->execution == STD_NULL ||
        context->keyboard == STD_NULL) return;
    if (core_platform_host_surface_lease_acquire(&win32_console_lease,
            context) != NTVDM64_STATUS_OK) return;
    run_context = STD_CALLOC(1u, sizeof(*run_context));
    if (run_context == STD_NULL) {
        core_platform_host_surface_lease_release(&win32_console_lease, context);
        return;
    }
    run_context->platform = context;
    run_context->input = GetStdHandle(STD_INPUT_HANDLE);
    oldDeviceFlip = vm_platform_execution_get_flip_for(context->execution);
    run_context->output = GetStdHandle(STD_OUTPUT_HANDLE);
    ((vm_platform_run_context *)context)->console_surface.native_handle =
        run_context->output;
    ((vm_platform_run_context *)context)->console_renderer =
        w32cdisp_context_create();
    if (context->console_renderer == STD_NULL) goto final;
    kernel_thread = CreateThread(STD_NULL, 0, ThreadKernel, run_context, 0,
                                 &ThreadIdKernel);
    if (kernel_thread == STD_NULL) goto final;
    while (oldDeviceFlip == vm_platform_execution_get_flip_for(context->execution)) {
        core_product_utils_sleep(context->wait_scope, 100);
    }
    display_thread = CreateThread(STD_NULL, 0, ThreadDisplay, run_context, 0,
                                  &ThreadIdDisplay);
    if (display_thread == STD_NULL) {
        vm_platform_execution_stop_for(context->execution);
        WaitForSingleObject(kernel_thread, INFINITE);
        CloseHandle(kernel_thread);
        goto final;
    }
    while (vm_platform_execution_is_running_for(context->execution)) {
        core_product_utils_sleep(context->wait_scope, 20);
        w32ckeybProcess(run_context);
    }
    WaitForSingleObject(kernel_thread, INFINITE);
    WaitForSingleObject(display_thread, INFINITE);
    CloseHandle(kernel_thread);
    CloseHandle(display_thread);
final:
    w32cdisp_context_destroy((w32cdisp_context *)context->console_renderer);
    ((vm_platform_run_context *)context)->console_renderer = STD_NULL;
    ((vm_platform_run_context *)context)->console_surface.native_handle = STD_NULL;
    STD_FREE(run_context);
    core_platform_host_surface_lease_release(&win32_console_lease, context);
}
