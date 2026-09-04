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

static C_VOID vm_platform_win32_keyboard_submit_guest(
    const vm_platform_run_context *context, type_unsigned_16 scan_code,
    type_unsigned_16 virtual_key, C_INT pressed)
{
    (C_VOID)core_platform_win32_keyboard_submit_key((C_VOID *)context,
        vm_platform_win32_keyboard_submit, scan_code, virtual_key, pressed);
}

static C_INT vm_platform_win32_keyboard_modifier_bit(type_unsigned_16 virtual_key)
{
    return virtual_key == VK_CONTROL || virtual_key == VK_LCONTROL ||
        virtual_key == VK_RCONTROL ? VM_PLATFORM_WIN32_MODIFIER_CONTROL :
        virtual_key == VK_MENU || virtual_key == VK_LMENU ||
        virtual_key == VK_RMENU ? VM_PLATFORM_WIN32_MODIFIER_ALT : 0;
}

static C_VOID vm_platform_win32_keyboard_flush_pending_modifiers(
    const vm_platform_run_context *context, vm_platform_run_handle *owner)
{
    C_INT keys = STD_ATOMIC_EXCHANGE(&owner->pending_modifier_keys, 0);

    if ((keys & VM_PLATFORM_WIN32_MODIFIER_CONTROL) != 0) {
        vm_platform_win32_keyboard_submit_guest(context, (type_unsigned_16)
            STD_ATOMIC_LOAD(&owner->pending_control_scan_code), VK_CONTROL, 1);
    }
    if ((keys & VM_PLATFORM_WIN32_MODIFIER_ALT) != 0) {
        vm_platform_win32_keyboard_submit_guest(context, (type_unsigned_16)
            STD_ATOMIC_LOAD(&owner->pending_alt_scan_code), VK_MENU, 1);
    }
}

static C_VOID vm_platform_win32_keyboard_submit_ctrl_alt_delete(
    const vm_platform_run_context *context)
{
    vm_platform_win32_keyboard_submit_guest(context, 0x001du, VK_CONTROL, 1);
    vm_platform_win32_keyboard_submit_guest(context, 0x0038u, VK_MENU, 1);
    vm_platform_win32_keyboard_submit_guest(context, 0x0153u, VK_DELETE, 1);
    vm_platform_win32_keyboard_submit_guest(context, 0x0153u, VK_DELETE, 0);
    vm_platform_win32_keyboard_submit_guest(context, 0x0038u, VK_MENU, 0);
    vm_platform_win32_keyboard_submit_guest(context, 0x001du, VK_CONTROL, 0);
}

static C_VOID vm_platform_win32_keyboard_submit_alt_enter(
    const vm_platform_run_context *context)
{
    vm_platform_win32_keyboard_submit_guest(context, 0x0038u, VK_MENU, 1);
    vm_platform_win32_keyboard_submit_guest(context, 0x001cu, VK_RETURN, 1);
    vm_platform_win32_keyboard_submit_guest(context, 0x001cu, VK_RETURN, 0);
    vm_platform_win32_keyboard_submit_guest(context, 0x0038u, VK_MENU, 0);
}

static C_INT vm_platform_win32_keyboard_classify(
    const vm_platform_run_context *context, vm_platform_run_handle *owner,
    type_unsigned_16 scan_code, type_unsigned_16 virtual_key,
    type_unsigned_8 modifiers, C_INT pressed)
{
    C_INT modifier_bit;
    vm_platform_run_event event = VM_PLATFORM_RUN_EVENT_NONE;

    if (owner == STD_NULL) return TYPE_FALSE;
    modifier_bit = vm_platform_win32_keyboard_modifier_bit(virtual_key);
    if (modifier_bit != 0) {
        C_INT pending = STD_ATOMIC_LOAD(&owner->pending_modifier_keys);

        if (pressed) {
            if ((pending & modifier_bit) != 0) return TYPE_TRUE;
            if (modifier_bit == VM_PLATFORM_WIN32_MODIFIER_CONTROL) {
                STD_ATOMIC_STORE(&owner->pending_control_scan_code, scan_code);
            } else {
                STD_ATOMIC_STORE(&owner->pending_alt_scan_code, scan_code);
            }
            STD_ATOMIC_STORE(&owner->pending_modifier_keys, pending | modifier_bit);
            return TYPE_TRUE;
        }
        if ((pending & modifier_bit) != 0) {
            vm_platform_win32_keyboard_flush_pending_modifiers(context, owner);
            vm_platform_win32_keyboard_submit_guest(context, scan_code,
                virtual_key, 0);
            return TYPE_TRUE;
        }
        if ((STD_ATOMIC_LOAD(&owner->suppressed_modifier_keys) &
                modifier_bit) != 0) {
            STD_ATOMIC_STORE(&owner->suppressed_modifier_keys,
                STD_ATOMIC_LOAD(&owner->suppressed_modifier_keys) & ~modifier_bit);
            return TYPE_TRUE;
        }
        return TYPE_FALSE;
    }
    if (!pressed) {
        C_INT reserved = STD_ATOMIC_LOAD(&owner->reserved_virtual_key);

        if (reserved != (C_INT)virtual_key) return TYPE_FALSE;
        STD_ATOMIC_STORE(&owner->reserved_virtual_key, 0);
        return TYPE_TRUE;
    }
    if ((modifiers & (VM_PLATFORM_WIN32_MODIFIER_CONTROL |
        VM_PLATFORM_WIN32_MODIFIER_ALT)) !=
        (VM_PLATFORM_WIN32_MODIFIER_CONTROL | VM_PLATFORM_WIN32_MODIFIER_ALT)) {
        vm_platform_win32_keyboard_flush_pending_modifiers(context, owner);
        return TYPE_FALSE;
    }
    if (virtual_key == 'P') event = VM_PLATFORM_RUN_EVENT_PAUSE_REQUESTED;
    else if (virtual_key == 'M') event = VM_PLATFORM_RUN_EVENT_MOUSE_RELEASE_REQUESTED;
    else if (virtual_key != 'D' && virtual_key != 'F') {
        vm_platform_win32_keyboard_flush_pending_modifiers(context, owner);
        return TYPE_FALSE;
    }
    {
        C_INT pending = STD_ATOMIC_EXCHANGE(&owner->pending_modifier_keys, 0);

        STD_ATOMIC_STORE(&owner->suppressed_modifier_keys,
            STD_ATOMIC_LOAD(&owner->suppressed_modifier_keys) | pending);
    }
    if (STD_ATOMIC_LOAD(&owner->reserved_virtual_key) == (C_INT)virtual_key) {
        return TYPE_TRUE;
    }
    STD_ATOMIC_STORE(&owner->reserved_virtual_key, virtual_key);
    if (virtual_key == 'D') {
        vm_platform_win32_keyboard_submit_ctrl_alt_delete(context);
    } else if (virtual_key == 'F') {
        vm_platform_win32_keyboard_submit_alt_enter(context);
    } else vm_platform_run_handle_report(owner, event);
    return TYPE_TRUE;
}

C_VOID vm_platform_win32_keyboard_make_key_with_modifiers_for(
    const vm_platform_run_context *context, vm_platform_run_handle *owner,
    type_unsigned_16 scan_code, type_unsigned_16 virtual_key,
    type_unsigned_8 modifiers, C_INT pressed)
{
    if (context == STD_NULL || vm_platform_win32_keyboard_classify(context,
            owner, scan_code, virtual_key, modifiers, pressed)) return;
    vm_platform_win32_keyboard_submit_guest(context, scan_code, virtual_key,
        pressed);
}

C_VOID vm_platform_win32_keyboard_make_key_for(
    const vm_platform_run_context *context, vm_platform_run_handle *owner,
    type_unsigned_16 scan_code, type_unsigned_16 virtual_key, C_INT pressed)
{
    vm_platform_win32_keyboard_make_key_with_modifiers_for(context, owner,
        scan_code, virtual_key, VM_PLATFORM_WIN32_MODIFIER_NONE, pressed);
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
