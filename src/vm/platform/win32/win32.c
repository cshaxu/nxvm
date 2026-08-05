/* Copyright 2012-2014 Neko. */

/* WIN32 provides win32 platform input and output interface. */

#include "type.h"

#include "vm/platform/input.h"


#include "vm/platform/win32/win32con.h"

#include "vm/platform/win32/win32app.h"

#include "vm/platform/win32/win32.h"

C_VOID vm_platform_win32_keyboard_make_key_for(
    const vm_platform_run_context *context, vm_platform_run_handle *owner,
    uint16_t scanCode, uint16_t virtualKey, C_INT pressed)
{
    if (context == STD_NULL) return;
    if (pressed && virtualKey == VK_F9) {
        vm_platform_run_handle_report(owner, VM_PLATFORM_RUN_EVENT_STOP_REQUESTED);
    }
    vm_platform_keyboard_receive_key_event_for(context->keyboard, scanCode,
        virtualKey, pressed);
}

C_VOID vm_platform_win32_mouse_relative_for(
    const vm_platform_run_context *context, int16_t delta_x, int16_t delta_y,
    uint8_t buttons)
{
    if (context == STD_NULL) return;
    vm_platform_mouse_receive_relative_event_for(context->mouse, delta_x,
        delta_y, buttons);
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
