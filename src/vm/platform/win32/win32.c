/* Copyright 2012-2014 Neko. */

/* WIN32 provides win32 platform input and output interface. */

#include "type.h"

#include "core/platform/input_interface.h"
#include "core/platform/win32/keyboard.h"


#include "vm/platform/win32/win32con.h"

#include "vm/platform/win32/win32app.h"

#include "vm/platform/win32/win32.h"

#include "vm/platform/platform_internal.h"

static type_status vm_platform_win32_keyboard_submit(C_VOID *context,
    const core_platform_input_event *event)
{
    const vm_platform_run_context *run_context = context;

    return run_context == STD_NULL ? TYPE_STATUS_INVALID_ARGUMENT :
        vm_platform_host_input_sink_submit(&run_context->input_sink, event);
}

C_VOID vm_platform_win32_keyboard_make_key_for(
    const vm_platform_run_context *context, vm_platform_run_handle *owner,
    type_unsigned_16 scanCode, type_unsigned_16 virtualKey, C_INT pressed)
{
    if (context == STD_NULL) return;
    if (pressed && virtualKey == VK_F9) {
        vm_platform_run_handle_report(owner,
            VM_PLATFORM_RUN_EVENT_STOP_REQUESTED);
        return;
    }
    (C_VOID)core_platform_win32_keyboard_submit_key((C_VOID *)context,
        vm_platform_win32_keyboard_submit, scanCode, virtualKey, pressed);
}

C_VOID vm_platform_win32_keyboard_make_character_for(const vm_platform_run_context *context,
    type_unsigned_32 scalar)
{
    (C_VOID)core_platform_win32_keyboard_submit_character((C_VOID *)context,
        vm_platform_win32_keyboard_submit, scalar);
}

C_VOID vm_platform_win32_keyboard_make_utf16_for(
    core_platform_win32_keyboard_normalizer *state, const vm_platform_run_context *context,
    type_unsigned_16 code_unit)
{
    (C_VOID)core_platform_win32_keyboard_submit_utf16(state, (C_VOID *)context,
        vm_platform_win32_keyboard_submit, code_unit);
}

C_VOID vm_platform_win32_mouse_relative_for(
    const vm_platform_run_context *context, type_signed_16 delta_x, type_signed_16 delta_y,
    type_unsigned_8 buttons)
{
    core_platform_input_event event;

    if (context == STD_NULL) return;
    event.kind = CORE_PLATFORM_INPUT_RELATIVE_MOUSE;
    event.data.relative_mouse.delta_x = delta_x;
    event.data.relative_mouse.delta_y = delta_y;
    event.data.relative_mouse.buttons = buttons;
    (C_VOID)vm_platform_host_input_sink_submit(&context->input_sink, &event);
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
    if (vm_platform_run_context_get_display_mode(context) ==
        VM_PLATFORM_DISPLAY_WINDOW) {
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
