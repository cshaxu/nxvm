#include "type.h"

#include <tchar.h>

#include "vm/platform/platform.h"
#include "vm/platform/win32/win32.h"
#include "vm/platform/win32/win32app.h"

typedef struct host_action_capture {
    core_platform_input_event events[12];
    C_UINT count;
} host_action_capture;

static type_status host_action_capture_submit(C_VOID *context,
    const core_platform_input_event *event)
{
    host_action_capture *capture = context;

    if (capture == STD_NULL || event == STD_NULL || capture->count ==
        (C_UINT)(sizeof(capture->events) / sizeof(capture->events[0]))) {
        return TYPE_STATUS_INVALID_STATE;
    }
    capture->events[capture->count++] = *event;
    return TYPE_STATUS_OK;
}

static C_INT host_action_capture_key(const host_action_capture *capture,
    C_UINT index, type_unsigned_16 virtual_key, C_INT pressed)
{
    return capture != STD_NULL && index < capture->count &&
        capture->events[index].kind == CORE_PLATFORM_INPUT_KEY &&
        capture->events[index].data.key.virtual_key == virtual_key &&
        capture->events[index].data.key.pressed == pressed;
}

int main(C_INT argc, C_CHAR **argv)
{
    vm_platform_run_context *context = STD_NULL;
    vm_platform_run_handle *handle = STD_NULL;
    host_action_capture capture = {0};
    vm_platform_host_input_sink sink = {host_action_capture_submit, &capture};

    (C_VOID)argc;
    (C_VOID)argv;
    if (vm_platform_run_context_create(STD_NULL, &sink, STD_NULL, STD_NULL,
            STD_NULL, &context) != TYPE_STATUS_OK ||
        vm_platform_run_handle_create(&handle) != TYPE_STATUS_OK) goto fail;
    if (_tcscmp(vm_platform_win32app_title_for_lifecycle(
            VM_PLATFORM_EXECUTION_RUNNING), _T("NXVM (Running)")) != 0 ||
        _tcscmp(vm_platform_win32app_title_for_lifecycle(
            VM_PLATFORM_EXECUTION_PAUSED), _T("NXVM (Paused)")) != 0 ||
        vm_platform_win32app_pointer_input_enabled(
            VM_PLATFORM_EXECUTION_RUNNING, 0) ||
        !vm_platform_win32app_pointer_input_enabled(
            VM_PLATFORM_EXECUTION_RUNNING, 1) ||
        vm_platform_win32app_pointer_input_enabled(
            VM_PLATFORM_EXECUTION_PAUSED, 1)) goto fail;

    vm_platform_win32_keyboard_make_key_with_modifiers_for(context, handle,
        0x003bu, VK_F1, VM_PLATFORM_WIN32_MODIFIER_NONE, 1);
    vm_platform_win32_keyboard_make_key_with_modifiers_for(context, handle,
        0x003bu, VK_F1, VM_PLATFORM_WIN32_MODIFIER_NONE, 0);
    vm_platform_win32_keyboard_make_key_with_modifiers_for(context, handle,
        0x0043u, VK_F9, VM_PLATFORM_WIN32_MODIFIER_NONE, 1);
    vm_platform_win32_keyboard_make_key_with_modifiers_for(context, handle,
        0x0043u, VK_F9, VM_PLATFORM_WIN32_MODIFIER_NONE, 0);
    if (capture.count != 4u || !host_action_capture_key(&capture, 0u, VK_F1, 1) ||
        !host_action_capture_key(&capture, 1u, VK_F1, 0) ||
        !host_action_capture_key(&capture, 2u, VK_F9, 1) ||
        !host_action_capture_key(&capture, 3u, VK_F9, 0) ||
        vm_platform_run_handle_take_stop_report(handle)) goto fail;

    vm_platform_win32_keyboard_make_key_with_modifiers_for(context, handle,
        0x001du, VK_CONTROL, VM_PLATFORM_WIN32_MODIFIER_CONTROL, 1);
    vm_platform_win32_keyboard_make_key_with_modifiers_for(context, handle,
        0x0038u, VK_MENU, VM_PLATFORM_WIN32_MODIFIER_CONTROL |
        VM_PLATFORM_WIN32_MODIFIER_ALT, 1);
    vm_platform_win32_keyboard_make_key_with_modifiers_for(context, handle,
        0x0019u, 'P', VM_PLATFORM_WIN32_MODIFIER_CONTROL |
        VM_PLATFORM_WIN32_MODIFIER_ALT, 1);
    vm_platform_win32_keyboard_make_key_with_modifiers_for(context, handle,
        0x0019u, 'P', VM_PLATFORM_WIN32_MODIFIER_CONTROL |
        VM_PLATFORM_WIN32_MODIFIER_ALT, 0);
    vm_platform_win32_keyboard_make_key_with_modifiers_for(context, handle,
        0x0038u, VK_MENU, VM_PLATFORM_WIN32_MODIFIER_CONTROL, 0);
    vm_platform_win32_keyboard_make_key_with_modifiers_for(context, handle,
        0x001du, VK_CONTROL, VM_PLATFORM_WIN32_MODIFIER_NONE, 0);
    if (capture.count != 4u || !vm_platform_run_handle_take_pause_report(handle) ||
        vm_platform_run_handle_take_pause_report(handle)) goto fail;

    vm_platform_win32_keyboard_make_key_with_modifiers_for(context, handle,
        0x0020u, 'D', VM_PLATFORM_WIN32_MODIFIER_CONTROL |
        VM_PLATFORM_WIN32_MODIFIER_ALT, 1);
    vm_platform_win32_keyboard_make_key_with_modifiers_for(context, handle,
        0x0020u, 'D', VM_PLATFORM_WIN32_MODIFIER_CONTROL |
        VM_PLATFORM_WIN32_MODIFIER_ALT, 0);
    vm_platform_win32_keyboard_make_key_with_modifiers_for(context, handle,
        0x0032u, 'M', VM_PLATFORM_WIN32_MODIFIER_CONTROL |
        VM_PLATFORM_WIN32_MODIFIER_ALT, 1);
    vm_platform_win32_keyboard_make_key_with_modifiers_for(context, handle,
        0x0032u, 'M', VM_PLATFORM_WIN32_MODIFIER_CONTROL |
        VM_PLATFORM_WIN32_MODIFIER_ALT, 0);
    if (capture.count != 4u || !vm_platform_run_handle_take_debug_report(handle) ||
        !vm_platform_run_handle_take_mouse_release_report(handle)) goto fail;

    vm_platform_win32_keyboard_make_key_with_modifiers_for(context, handle,
        0x001du, VK_CONTROL, VM_PLATFORM_WIN32_MODIFIER_CONTROL, 1);
    vm_platform_win32_keyboard_make_key_with_modifiers_for(context, handle,
        0x0038u, VK_MENU, VM_PLATFORM_WIN32_MODIFIER_CONTROL |
        VM_PLATFORM_WIN32_MODIFIER_ALT, 1);
    vm_platform_win32_keyboard_make_key_with_modifiers_for(context, handle,
        0x002du, 'X', VM_PLATFORM_WIN32_MODIFIER_CONTROL |
        VM_PLATFORM_WIN32_MODIFIER_ALT, 1);
    vm_platform_win32_keyboard_make_key_with_modifiers_for(context, handle,
        0x002du, 'X', VM_PLATFORM_WIN32_MODIFIER_CONTROL |
        VM_PLATFORM_WIN32_MODIFIER_ALT, 0);
    vm_platform_win32_keyboard_make_key_with_modifiers_for(context, handle,
        0x0038u, VK_MENU, VM_PLATFORM_WIN32_MODIFIER_CONTROL, 0);
    vm_platform_win32_keyboard_make_key_with_modifiers_for(context, handle,
        0x001du, VK_CONTROL, VM_PLATFORM_WIN32_MODIFIER_NONE, 0);
    if (capture.count != 10u || !host_action_capture_key(&capture, 4u,
            VK_CONTROL, 1) || !host_action_capture_key(&capture, 5u, VK_MENU, 1) ||
        !host_action_capture_key(&capture, 6u, 'X', 1) ||
        !host_action_capture_key(&capture, 7u, 'X', 0) ||
        !host_action_capture_key(&capture, 8u, VK_MENU, 0) ||
        !host_action_capture_key(&capture, 9u, VK_CONTROL, 0)) goto fail;

    vm_platform_run_handle_destroy(handle);
    vm_platform_run_context_destroy(context);
    puts("M5:T518:S2:HOST-ACTIONS:OK");
    return 0;

fail:
    vm_platform_run_handle_destroy(handle);
    vm_platform_run_context_destroy(context);
    return 1;
}
