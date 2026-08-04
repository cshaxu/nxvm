/* Copyright 2012-2014 Neko. */

/* WIN32 provides win32 platform input and output interface. */

#include "type.h"

#include "vm/platform/input.h"


#include "vm/platform/win32/win32con.h"

#include "vm/platform/win32/win32app.h"

#include "vm/platform/win32/win32.h"

static uint32_t vm_platform_win32_keyboard_get_async_state(C_VOID)
{
    uint32_t state = 0u;

    if (GetAsyncKeyState(VK_RSHIFT) & 0x8000) state |= CORE_MACHINE_KEYBOARD_ASYNC_RIGHT_SHIFT;
    if (GetAsyncKeyState(VK_LSHIFT) & 0x8000) state |= CORE_MACHINE_KEYBOARD_ASYNC_LEFT_SHIFT;
    if (GetAsyncKeyState(VK_CONTROL) & 0x8000) state |= CORE_MACHINE_KEYBOARD_ASYNC_CONTROL;
    if (GetAsyncKeyState(VK_MENU) & 0x8000) state |= CORE_MACHINE_KEYBOARD_ASYNC_ALT;
    if (GetAsyncKeyState(VK_SCROLL) & 0x8000) state |= CORE_MACHINE_KEYBOARD_ASYNC_SCROLL_LOCK;
    if (GetAsyncKeyState(VK_NUMLOCK) & 0x8000) state |= CORE_MACHINE_KEYBOARD_ASYNC_NUM_LOCK;
    if (GetAsyncKeyState(VK_CAPITAL) & 0x8000) state |= CORE_MACHINE_KEYBOARD_ASYNC_CAPS_LOCK;
    if (GetAsyncKeyState(VK_INSERT) & 0x8000) state |= CORE_MACHINE_KEYBOARD_ASYNC_INSERT;
    return state;
}

static uint32_t vm_platform_win32_keyboard_get_toggle_state(C_VOID)
{
    uint32_t state = 0u;

    if (GetKeyState(VK_SCROLL) & 0x0001) state |= CORE_MACHINE_KEYBOARD_TOGGLE_SCROLL_LOCK;
    if (GetKeyState(VK_NUMLOCK) & 0x0001) state |= CORE_MACHINE_KEYBOARD_TOGGLE_NUM_LOCK;
    if (GetKeyState(VK_CAPITAL) & 0x0001) state |= CORE_MACHINE_KEYBOARD_TOGGLE_CAPS_LOCK;
    if (GetKeyState(VK_INSERT) & 0x0001) state |= CORE_MACHINE_KEYBOARD_TOGGLE_INSERT;
    if (GetKeyState(VK_PAUSE) & 0x0001) state |= CORE_MACHINE_KEYBOARD_TOGGLE_PAUSE;
    return state;
}

C_VOID vm_platform_win32_keyboard_make_status_for(const vm_platform_run_context *context) {
    uint32_t asynchronous_keys = vm_platform_win32_keyboard_get_async_state();
    uint32_t toggle_keys = vm_platform_win32_keyboard_get_toggle_state();

    (C_VOID)vm_platform_run_context_submit_keyboard_state(context,
        asynchronous_keys, toggle_keys);
}
C_VOID vm_platform_win32_keyboard_make_key_for(
    const vm_platform_run_context *context, vm_platform_run_handle *owner,
    C_UCHAR scanCode, C_UCHAR virtualKey)
{
    vm_platform_win32_keyboard_make_status_for(context);
    if (context == STD_NULL) return;
    if (virtualKey == VK_F9) {
        vm_platform_run_handle_report(owner, VM_PLATFORM_RUN_EVENT_STOP_REQUESTED);
    }
    vm_platform_keyboard_receive_key_press_for(context->keyboard, scanCode,
        virtualKey);
}

C_VOID vm_platform_win32_display_set_screen(WIN32_BOOL flagWindow,
                           const vm_platform_run_context *context) {
    if (flagWindow) {
        vm_platform_win32app_display_set_screen(context);
    } else {
        vm_platform_win32con_display_set_screen(context);
    }
}

C_VOID vm_platform_win32_display_paint(WIN32_BOOL flagWindow,
                       const vm_platform_run_context *context) {
    if (flagWindow) {
        vm_platform_win32app_display_paint(context);
    } else {
        vm_platform_win32con_display_paint(context);
    }
}

type_status vm_platform_win32_run_handle_start(
    const vm_platform_run_context *context, vm_platform_run_handle *handle) {
    if (vm_platform_run_context_get_window_display(context)) {
        return vm_platform_win32app_run_handle_start(context, handle);
    }
    return vm_platform_win32con_run_handle_start(context, handle);
}

C_VOID vm_platform_win32_run_handle_request_stop(vm_platform_run_handle *handle) {
    if (vm_platform_run_handle_is_window_display(handle)) {
        vm_platform_win32app_run_handle_request_stop(handle);
    } else {
        vm_platform_win32con_run_handle_request_stop(handle);
    }
}

C_VOID vm_platform_win32_run_handle_join(vm_platform_run_handle *handle) {
    if (vm_platform_run_handle_is_window_display(handle)) {
        vm_platform_win32app_run_handle_join(handle);
    } else {
        vm_platform_win32con_run_handle_join(handle);
    }
}

C_VOID vm_platform_win32_run_handle_finalize(vm_platform_run_handle *handle) {
    if (vm_platform_run_handle_is_window_display(handle)) {
        vm_platform_win32app_run_handle_finalize(handle);
    } else {
        vm_platform_win32con_run_handle_finalize(handle);
    }
}
