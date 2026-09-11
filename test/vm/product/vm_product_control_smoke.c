#include "type.h"
#include "vm/product/control.h"

typedef struct vm_product_control_input_capture {
    ui_input_event events[4];
    STD_SIZE_T count;
} vm_product_control_input_capture;

static type_status vm_product_control_capture_input(C_VOID *opaque,
    const ui_input_event *event)
{
    vm_product_control_input_capture *capture = opaque;

    if (capture == STD_NULL || event == STD_NULL ||
        capture->count == sizeof(capture->events) / sizeof(capture->events[0]))
        return TYPE_STATUS_NO_MEMORY;
    capture->events[capture->count++] = *event;
    return TYPE_STATUS_OK;
}

int main(void)
{
    vm_product_control *control = STD_NULL;
    vm_product_control_fact fact = {0};
    vm_product_control_fact received = {0};
    core_machine_guest_display_frame display = {0};
    core_machine_guest_display_frame received_display = {0};
    vm_product_control_input_capture capture = {0};
    ui_input_event input = {0};

    if (vm_product_control_create(&control) != TYPE_STATUS_OK) return 1;
    fact.kind = VM_PRODUCT_CONTROL_FACT_LIFECYCLE;
    fact.value.lifecycle = VM_SESSION_RUNNING;
    display.generation = 7u;
    if (vm_product_control_publish_display(control, &display) != TYPE_STATUS_OK ||
        vm_product_control_publish(control, &fact) != TYPE_STATUS_OK ||
        vm_product_control_take(control, &received, &received_display, 0u) != TYPE_STATUS_OK ||
        received.kind != VM_PRODUCT_CONTROL_FACT_LIFECYCLE ||
        received.value.lifecycle != VM_SESSION_RUNNING ||
        STD_STRCMP(vm_product_control_note_lifecycle(control,
            received.value.lifecycle), "started") != 0 ||
        STD_STRCMP(vm_product_control_note_lifecycle(control,
            VM_SESSION_PAUSED), "paused") != 0 ||
        STD_STRCMP(vm_product_control_note_lifecycle(control,
            VM_SESSION_RUNNING), "resumed") != 0) {
        vm_product_control_destroy(control);
        return 1;
    }
    if (vm_product_control_take(control, &received, &received_display, 0u) != TYPE_STATUS_OK ||
        received.kind != VM_PRODUCT_CONTROL_FACT_DISPLAY ||
        received_display.generation != 7u) {
        vm_product_control_destroy(control);
        return 1;
    }
    display.generation = 8u;
    if (vm_product_control_publish_display(control, &display) != TYPE_STATUS_OK) {
        vm_product_control_destroy(control);
        return 1;
    }
    display.generation = 9u;
    if (vm_product_control_publish_display(control, &display) != TYPE_STATUS_OK ||
        vm_product_control_take(control, &received, &received_display, 0u) != TYPE_STATUS_OK ||
        received.kind != VM_PRODUCT_CONTROL_FACT_DISPLAY ||
        received_display.generation != 9u) {
        vm_product_control_destroy(control);
        return 1;
    }
    fact.kind = VM_PRODUCT_CONTROL_FACT_HOST_INPUT;
    fact.value.host_input.type = UI_EVENT_KEY;
    fact.value.host_input.data.key.scan_code = 0x3bu;
    if (vm_product_control_publish(control, &fact) != TYPE_STATUS_OK ||
        vm_product_control_begin_run(control) == 0u ||
        vm_product_control_take(control, &received, &received_display, 0u) ==
            TYPE_STATUS_OK) {
        vm_product_control_destroy(control);
        return 1;
    }
    if (vm_product_control_publish(control, &fact) != TYPE_STATUS_OK ||
        vm_product_control_take(control, &received, &received_display, 0u) != TYPE_STATUS_OK ||
        received.kind != VM_PRODUCT_CONTROL_FACT_HOST_INPUT ||
        received.value.host_input.data.key.scan_code != 0x3bu) {
        vm_product_control_destroy(control);
        return 1;
    }
    display.generation = 10u;
    if (vm_product_control_publish_display(control, &display) != TYPE_STATUS_OK ||
        vm_product_control_begin_run(control) == 0u ||
        vm_product_control_take(control, &received, &received_display, 0u) ==
            TYPE_STATUS_OK) {
        vm_product_control_destroy(control);
        return 1;
    }
    input.type = UI_EVENT_KEY;
    input.source_identity = 17u;
    input.data.key.scan_code = 0x1eu;
    input.data.key.key = 'A';
    input.data.key.pressed = TYPE_TRUE;
    if (vm_product_control_dispatch_host_input(control, &input, TYPE_TRUE,
            vm_product_control_capture_input, &capture) != TYPE_STATUS_OK ||
        capture.count != 1u || !capture.events[0].data.key.pressed) {
        vm_product_control_destroy(control);
        return 1;
    }
    input = (ui_input_event){0};
    input.type = UI_EVENT_SOURCE_RETIRED;
    input.source_identity = 17u;
    if (vm_product_control_dispatch_host_input(control, &input, TYPE_TRUE,
            vm_product_control_capture_input, &capture) != TYPE_STATUS_OK ||
        capture.count != 2u || capture.events[1].data.key.pressed) {
        vm_product_control_destroy(control);
        return 1;
    }
    input = capture.events[0];
    input.source_identity = 18u;
    if (vm_product_control_dispatch_host_input(control, &input, TYPE_TRUE,
            vm_product_control_capture_input, &capture) != TYPE_STATUS_OK ||
        capture.count != 3u) {
        vm_product_control_destroy(control);
        return 1;
    }
    input = (ui_input_event){0};
    input.type = UI_EVENT_SOURCE_RETIRED;
    input.source_identity = 18u;
    if (vm_product_control_dispatch_host_input(control, &input, TYPE_FALSE,
            vm_product_control_capture_input, &capture) != TYPE_STATUS_OK ||
        capture.count != 3u ||
        vm_product_control_dispatch_host_input(control, &input, TYPE_TRUE,
            vm_product_control_capture_input, &capture) != TYPE_STATUS_OK ||
        capture.count != 3u) {
        vm_product_control_destroy(control);
        return 1;
    }
    vm_product_control_close(control);
    if (vm_product_control_publish(control, &fact) == TYPE_STATUS_OK) {
        vm_product_control_destroy(control);
        return 1;
    }
    vm_product_control_destroy(control);
    return 0;
}
