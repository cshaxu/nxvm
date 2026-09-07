#include "type.h"

#include "lib/ux/presenter.h"
#include "lib/ux/router.h"
#include "lib/ux/win32/input.h"
#include "vm/platform/platform.h"
#include "vm/platform/platform_internal.h"
#include "vm/platform/ux_binding.h"

#include <string.h>

typedef struct host_action_capture {
    core_machine_guest_input_event events[24];
    type_unsigned_32 count;
} host_action_capture;

static type_status host_action_capture_submit(C_VOID *context,
    const core_machine_guest_input_event *event)
{
    host_action_capture *capture = context;

    if (capture == STD_NULL || event == STD_NULL || capture->count >= 24u)
        return TYPE_STATUS_INVALID_STATE;
    capture->events[capture->count++] = *event;
    return TYPE_STATUS_OK;
}

int main(C_INT argc, C_CHAR **argv)
{
    vm_platform_run_context *context = STD_NULL;
    vm_platform_run_handle *handle = STD_NULL;
    vm_platform_host_input_sink sink;
    host_action_capture capture = { 0 };
    ux_binding binding;
    ux_win32_keyboard_normalizer normalizer = { 0 };
    ux_event event = { 0 };
    C_CHAR title[UX_WINDOW_TITLE_CAPACITY];
    type_unsigned_32 title_generation = 0u;

    (C_VOID)argc;
    (C_VOID)argv;
    sink.submit = host_action_capture_submit;
    sink.context = &capture;
    if (vm_platform_run_context_create(STD_NULL, &sink, STD_NULL, STD_NULL,
            &context) != TYPE_STATUS_OK || vm_platform_run_handle_create(
            &handle) != TYPE_STATUS_OK || vm_platform_ux_binding_initialize(
            context, handle, &binding) != TYPE_STATUS_OK) goto fail;

    event.type = UX_EVENT_KEY;
    event.data.key.scan_code = 0x3bu;
    event.data.key.virtual_key = 0x70u;
    event.data.key.pressed = TYPE_TRUE;
    if (!binding.input_sink(binding.context, &event) || capture.count != 1u ||
        capture.events[0].data.key.scan_code != 0x3bu ||
        capture.events[0].data.key.virtual_key != 0x70u) goto fail;
    if (ux_binding_invoke_action(&binding, VM_PLATFORM_UX_ACTION_PAUSE_TOGGLE) !=
            UX_RUN_CONTINUE ||
        !vm_platform_run_handle_take_pause_report(handle) || capture.count != 3u ||
        capture.events[1].kind != CORE_MACHINE_GUEST_INPUT_RELATIVE_MOUSE ||
        capture.events[1].data.relative_mouse.buttons != 0u ||
        capture.events[2].data.key.scan_code != 0x3bu ||
        capture.events[2].data.key.pressed) goto fail;
    if (ux_binding_invoke_action(&binding, VM_PLATFORM_UX_ACTION_SEND_CTRL_ALT_DEL) !=
            UX_RUN_CONTINUE ||
        capture.count != 10u ||
        capture.events[3].kind != CORE_MACHINE_GUEST_INPUT_RELATIVE_MOUSE ||
        capture.events[3].data.relative_mouse.buttons != 0u ||
        capture.events[6].data.key.scan_code != 0x0153u ||
        !capture.events[6].data.key.pressed ||
        capture.events[6].data.key.virtual_key != 0x2eu) goto fail;
    if (!ux_win32_keyboard_submit_utf16(&normalizer, binding.context,
            binding.input_sink, 'a') || capture.count != 12u ||
        capture.events[10].data.key.virtual_key != 'A' ||
        !capture.events[10].data.key.pressed ||
        capture.events[11].data.key.virtual_key != 'A' ||
        capture.events[11].data.key.pressed) goto fail;
    if (ux_binding_invoke_action(&binding, VM_PLATFORM_UX_ACTION_SEND_ALT_ENTER) !=
            UX_RUN_CONTINUE ||
        capture.count != 17u ||
        capture.events[12].kind != CORE_MACHINE_GUEST_INPUT_RELATIVE_MOUSE ||
        capture.events[12].data.relative_mouse.buttons != 0u ||
        capture.events[14].data.key.scan_code != 0x1cu ||
        !capture.events[14].data.key.pressed ||
        capture.events[14].data.key.virtual_key != 0x0du) goto fail;
    if (!ux_win32_keyboard_submit_utf16(&normalizer, binding.context,
            binding.input_sink, 'b') || capture.count != 19u ||
        capture.events[17].data.key.virtual_key != 'B' ||
        !capture.events[17].data.key.pressed ||
        capture.events[18].data.key.virtual_key != 'B' ||
        capture.events[18].data.key.pressed) goto fail;
    if (strcmp(binding.window_initial_title, "NXVM (Running)") != 0) goto fail;
    vm_platform_run_context_set_window_title(context, "ignored");
    if (ux_router_capture_window_title(&context->ux_router, title, sizeof(title),
            &title_generation) != LIB_STATUS_OK || title_generation != 0u) goto fail;
    vm_platform_run_context_set_display_mode(context, VM_PLATFORM_DISPLAY_WINDOW);
    ux_router_set_active_target(&context->ux_router, UX_TARGET_WINDOW);
    vm_platform_run_context_set_window_title(context, "NXVM (Paused)");
    if (ux_router_capture_window_title(&context->ux_router, title, sizeof(title),
            &title_generation) != LIB_STATUS_OK || title_generation == 0u ||
        strcmp(title, "NXVM (Paused)") != 0) goto fail;
    ux_router_set_active_target(&context->ux_router, UX_TARGET_CONSOLE);
    vm_platform_run_context_set_window_title(context, "ignored");
    if (ux_router_capture_window_title(&context->ux_router, title, sizeof(title),
            &title_generation) != LIB_STATUS_OK || title_generation != 0u) goto fail;
    vm_platform_run_handle_destroy(handle);
    vm_platform_run_context_destroy(context);
    puts("M5:T522:S4:UX-BINDING:OK");
    return 0;

fail:
    vm_platform_run_handle_destroy(handle);
    vm_platform_run_context_destroy(context);
    return 1;
}
