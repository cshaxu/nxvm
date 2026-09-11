#include "type.h"

#include "vm/machine/executor_queue.h"

C_INT main(C_VOID)
{
    vm_machine_executor_queue bridge;
    vm_machine_request request = {0};
    vm_machine_request copy;
    STD_SIZE_T index;

    vm_machine_executor_queue_initialize(&bridge);
    request.kind = VM_MACHINE_REQUEST_INPUT;
    request.input.kind = VM_MACHINE_INPUT_KEY_EVENT;
    request.input.data.key_event.scan_code = 0x1eu;
    request.input.data.key_event.virtual_key = 0x41u;
    request.input.data.key_event.pressed = TYPE_TRUE;
    if (vm_machine_executor_queue_enqueue(&bridge, &request) !=
        TYPE_STATUS_OK) return 1;
    request.input.data.key_event.scan_code = 0x30u;
    if (vm_machine_executor_queue_dequeue(&bridge, &copy) !=
        TYPE_STATUS_OK ||
        copy.kind != VM_MACHINE_REQUEST_INPUT ||
        copy.input.kind != VM_MACHINE_INPUT_KEY_EVENT ||
        copy.input.data.key_event.scan_code != 0x1eu ||
        copy.input.data.key_event.virtual_key != 0x41u ||
        copy.input.data.key_event.pressed != TYPE_TRUE) return 1;

    for (index = 0u; index < VM_MACHINE_EXECUTOR_QUEUE_CAPACITY; ++index) {
        if (vm_machine_executor_queue_enqueue(&bridge, &request) !=
            TYPE_STATUS_OK) return 1;
    }
    if (vm_machine_executor_queue_enqueue(&bridge, &request) !=
        TYPE_STATUS_NO_MEMORY ||
        vm_machine_executor_queue_dequeue(&bridge, &copy) !=
        TYPE_STATUS_OK ||
        vm_machine_executor_queue_enqueue(&bridge, &request) !=
        TYPE_STATUS_OK ||
        vm_machine_executor_queue_dequeue(&bridge, STD_NULL) !=
        TYPE_STATUS_INVALID_ARGUMENT) return 1;

    return 0;
}
