#include "type.h"

#include "vm/machine/runtime/machine_interface.h"
#include "../support/rom/session_assets.h"
#include "vm/machine/runtime/machine_private.h"
#include "vm/machine/executor_fifo.h"

#define VM_HOST_CANCELLATION_F9_SCAN_CODE 0x43u
#define VM_HOST_CANCELLATION_F9_VIRTUAL_KEY 0x78u

C_INT main(C_VOID)
{
    vm_machine *session = STD_NULL;
    core_machine_guest_input_event event = { 0 };
    vm_machine_request request;

    if (vm_test_default_pc_at_session_create(STD_NULL, &session) != TYPE_STATUS_OK) goto fail;
    event.kind = CORE_MACHINE_GUEST_INPUT_KEY;
    event.data.key.scan_code = VM_HOST_CANCELLATION_F9_SCAN_CODE;
    event.data.key.virtual_key = VM_HOST_CANCELLATION_F9_VIRTUAL_KEY;
    event.data.key.pressed = TYPE_TRUE;
    if (vm_machine_submit_host_input(session, &event) != TYPE_STATUS_OK) goto fail;
    if (vm_machine_executor_fifo_dequeue_ingress(session->executor_fifo,
            &request) != TYPE_STATUS_OK ||
        request.kind != VM_MACHINE_REQUEST_INPUT ||
        request.input.kind != VM_MACHINE_INPUT_KEY_EVENT ||
        request.input.data.key_event.virtual_key != VM_HOST_CANCELLATION_F9_VIRTUAL_KEY ||
        !request.input.data.key_event.pressed) goto fail;
    vm_machine_destroy(session);
    STD_PRINTF("M5:T201:S3:HOST-CANCELLATION:OK\n");
    return 0;

fail:
    vm_machine_destroy(session);
    return 1;
}
