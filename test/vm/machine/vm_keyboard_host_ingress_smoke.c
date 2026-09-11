#include "type.h"

#include "vm/machine/runtime/machine_interface.h"
#include "vm/machine/runtime/machine_private.h"
#include "vm/machine/executor_fifo.h"
#include "../support/rom/session_assets.h"

C_INT main(C_VOID)
{
    vm_machine *session = STD_NULL;
    core_machine_guest_input_event event = {0};
    vm_machine_request request;
    C_INT saw_a_press = 0;
    C_INT saw_a_release = 0;

    if (vm_test_default_pc_at_session_create(STD_NULL, &session) != TYPE_STATUS_OK) goto fail;
    event.kind = CORE_MACHINE_GUEST_INPUT_KEY;
    event.data.key.scan_code = 0x2au;
    event.data.key.virtual_key = 0x10u;
    event.data.key.pressed = TYPE_TRUE;
    if (vm_machine_submit_host_input(session, &event) != TYPE_STATUS_OK) goto fail;
    if (vm_machine_executor_fifo_dequeue_ingress(session->executor_fifo,
            &request) != TYPE_STATUS_OK ||
        request.kind != VM_MACHINE_REQUEST_INPUT ||
        request.input.kind != VM_MACHINE_INPUT_KEY_EVENT ||
        request.input.data.key_event.scan_code != 0x2au ||
        !request.input.data.key_event.pressed) goto fail;
    if (vm_machine_submit_host_input(session, &event) != TYPE_STATUS_OK) goto fail;
    if (vm_machine_executor_fifo_dequeue_ingress(session->executor_fifo,
            &request) != TYPE_STATUS_OK ||
        request.kind != VM_MACHINE_REQUEST_INPUT ||
        request.input.kind != VM_MACHINE_INPUT_KEY_EVENT ||
        request.input.data.key_event.scan_code != 0x2au ||
        !request.input.data.key_event.pressed) goto fail;
    event.data.key.scan_code = 0x3bu;
    event.data.key.virtual_key = 0x70u;
    if (vm_machine_submit_host_input(session, &event) != TYPE_STATUS_OK) goto fail;
    if (vm_machine_executor_fifo_dequeue_ingress(session->executor_fifo,
            &request) != TYPE_STATUS_OK ||
        request.kind != VM_MACHINE_REQUEST_INPUT ||
        request.input.kind != VM_MACHINE_INPUT_KEY_EVENT ||
        request.input.data.key_event.scan_code != 0x3bu ||
        !request.input.data.key_event.pressed) goto fail;
    event.data.key.scan_code = 0x1eu;
    event.data.key.virtual_key = 'A';
    if (vm_machine_submit_host_input(session, &event) != TYPE_STATUS_OK) goto fail;
    event.data.key.pressed = TYPE_FALSE;
    if (vm_machine_submit_host_input(session, &event) != TYPE_STATUS_OK) goto fail;
    while (vm_machine_executor_fifo_dequeue_ingress(session->executor_fifo,
            &request) == TYPE_STATUS_OK) {
        if (request.kind != VM_MACHINE_REQUEST_INPUT ||
            request.input.kind != VM_MACHINE_INPUT_KEY_EVENT ||
            request.input.data.key_event.virtual_key != 'A') continue;
        if (request.input.data.key_event.pressed) saw_a_press = 1;
        else saw_a_release = 1;
    }
    if (!saw_a_press || !saw_a_release) goto fail;
    event.kind = (core_machine_guest_input_kind)2;
    if (vm_machine_submit_host_input(session, &event) != TYPE_STATUS_INVALID_ARGUMENT ||
        vm_machine_executor_fifo_dequeue_ingress(session->executor_fifo,
            &request) == TYPE_STATUS_OK) goto fail;
    vm_machine_destroy(session);
    STD_PRINTF("M5:T226:S2:HOST-INGRESS:OK\n");
    return 0;

fail:
    event.kind = (core_machine_guest_input_kind)2;
    if (vm_machine_submit_host_input(session, &event) != TYPE_STATUS_INVALID_ARGUMENT ||
        vm_machine_executor_fifo_dequeue_ingress(session->executor_fifo,
            &request) == TYPE_STATUS_OK) goto fail;
    vm_machine_destroy(session);
    return 1;
}
