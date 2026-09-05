#include "type.h"

#include "lib/ux/presenter.h"
#include "vm/platform/platform.h"
#include "vm/platform/ux_binding.h"

#include <string.h>

typedef struct host_action_capture {
    core_platform_input_event events[24];
    type_unsigned_32 count;
} host_action_capture;

static type_status host_action_capture_submit(C_VOID *context,
    const core_platform_input_event *event)
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
    ux_event event = { 0 };
    C_CHAR title[32];

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
    if (binding.handle_action(binding.context, UX_ACTION_PAUSE_TOGGLE,
            binding.input_sink) != UX_RUN_CONTINUE ||
        !vm_platform_run_handle_take_pause_report(handle) || capture.count != 3u ||
        capture.events[1].data.key.pressed ||
        capture.events[2].data.key.pressed) goto fail;
    if (binding.handle_action(binding.context, UX_ACTION_SEND_CTRL_ALT_DEL,
            binding.input_sink) != UX_RUN_CONTINUE || capture.count != 9u ||
        capture.events[5].data.key.scan_code != 0x0153u ||
        !capture.events[5].data.key.pressed ||
        capture.events[5].data.key.virtual_key != 0x2eu) goto fail;
    event.type = UX_EVENT_TEXT;
    event.data.text.scalar = 'a';
    if (!binding.input_sink(binding.context, &event) || capture.count != 11u ||
        capture.events[9].data.key.virtual_key != 'A' ||
        !capture.events[9].data.key.pressed ||
        capture.events[10].data.key.virtual_key != 'A' ||
        capture.events[10].data.key.pressed) goto fail;
    if (binding.handle_action(binding.context, UX_ACTION_SEND_ALT_ENTER,
            binding.input_sink) != UX_RUN_CONTINUE || capture.count != 17u ||
        capture.events[13].data.key.scan_code != 0x1cu ||
        !capture.events[13].data.key.pressed ||
        capture.events[13].data.key.virtual_key != 0x0du) goto fail;
    binding.get_title(binding.context, title, sizeof(title));
    if (strcmp(title, "NXVM (Stopped)") != 0) goto fail;
    vm_platform_run_handle_destroy(handle);
    vm_platform_run_context_destroy(context);
    puts("M5:T522:S4:UX-BINDING:OK");
    return 0;

fail:
    vm_platform_run_handle_destroy(handle);
    vm_platform_run_context_destroy(context);
    return 1;
}
