#include "type.h"

#include "vm/platform/platform.h"
#include "vm/platform/platform_internal.h"
#include "vm/platform/ux_binding.h"

typedef struct input_capture { core_machine_guest_input_event event; C_INT count; } input_capture;
static type_status submit(C_VOID *opaque, const core_machine_guest_input_event *event)
{ input_capture *capture = opaque; if (capture == STD_NULL || event == STD_NULL) return TYPE_STATUS_INVALID_ARGUMENT; capture->event = *event; ++capture->count; return TYPE_STATUS_OK; }

C_INT main(C_VOID)
{
    vm_platform_run_context *context = STD_NULL;
    vm_platform_run_handle *handle = STD_NULL;
    input_capture capture = { 0 };
    vm_platform_host_input_sink sink = { submit, &capture };
    ux_input_event event = { 0 };
    if (vm_platform_run_context_create(STD_NULL, &sink, STD_NULL, STD_NULL, &context) != TYPE_STATUS_OK) return 1;
    if (vm_platform_run_handle_create(&handle) != TYPE_STATUS_OK) goto fail;
    handle->context = context;
    context->run_handle = handle;
    event.type = UX_EVENT_KEY; event.data.key.scan_code = 0x3bu;
    event.data.key.key = UX_KEY_F1; event.data.key.pressed = LIB_TRUE;
    if (!vm_platform_ux_event_submit(context, &event) || capture.count != 1 ||
        capture.event.data.key.scan_code != 0x3bu || !capture.event.data.key.pressed) goto fail;
    event.type = UX_EVENT_MOUSE; event.data.mouse.delta_x = 3; event.data.mouse.delta_y = -2;
    event.data.mouse.buttons = UX_MOUSE_BUTTON_LEFT;
    if (!vm_platform_ux_event_submit(context, &event) || capture.count != 2 ||
        capture.event.kind != CORE_MACHINE_GUEST_INPUT_RELATIVE_MOUSE ||
        capture.event.data.relative_mouse.delta_x != 3 || capture.event.data.relative_mouse.delta_y != -2) goto fail;
    event.type = UX_EVENT_KEY;
    event.source_identity = 7u;
    event.data.key.scan_code = 0x1eu;
    event.data.key.key = (ux_key)0;
    event.data.key.pressed = LIB_TRUE;
    if (!vm_platform_run_context_handle_ux_input(context, &event) ||
        capture.count != 3 || !capture.event.data.key.pressed) goto fail;
    event.type = UX_EVENT_SOURCE_RETIRED;
    event.source_identity = 7u;
    if (!vm_platform_run_context_handle_ux_input(context, &event) ||
        capture.count != 4 || capture.event.data.key.pressed ||
        capture.event.data.key.scan_code != 0x1eu) goto fail;
    context->run_handle = STD_NULL;
    vm_platform_run_handle_destroy(handle);
    vm_platform_run_context_destroy(context); return 0;
fail:
    if (context != STD_NULL) context->run_handle = STD_NULL;
    vm_platform_run_handle_destroy(handle);
    vm_platform_run_context_destroy(context); return 1;
}
