#include "type.h"

#include "vm/machine/executor_fifo.h"

typedef struct executor_observer {
    C_UINT count;
    vm_machine_request requests[3u];
} executor_observer;

static C_VOID observe_request(C_VOID *opaque,
    const vm_machine_request *request)
{
    executor_observer *observer = opaque;

    if (observer == STD_NULL || request == STD_NULL) return;
    if (observer->count < 3u) observer->requests[observer->count] = *request;
    ++observer->count;
}

int main(void)
{
    vm_machine_executor_fifo *fifo = STD_NULL;
    vm_machine_request input = {0};
    vm_machine_request copy;
    executor_observer observer = {0};

    input.kind = VM_MACHINE_REQUEST_INPUT;
    input.input.kind = VM_MACHINE_INPUT_KEY_EVENT;
    input.input.data.key_event.scan_code = 0x1eu;
    input.input.data.key_event.virtual_key = 0x41u;
    input.input.data.key_event.pressed = TYPE_TRUE;
    if (vm_machine_executor_fifo_create(&fifo) != TYPE_STATUS_OK ||
        vm_machine_executor_fifo_enqueue_ingress(fifo, &input) != TYPE_STATUS_OK ||
        vm_machine_executor_fifo_dequeue_ingress(fifo, &copy) != TYPE_STATUS_OK ||
        copy.kind != VM_MACHINE_REQUEST_INPUT ||
        copy.input.data.key_event.scan_code != 0x1eu) return 1;

    vm_machine_executor_fifo_bind_consumer(fifo, observe_request, &observer);
    if (vm_machine_executor_fifo_enqueue_ingress(fifo, &input) != TYPE_STATUS_OK ||
        vm_machine_executor_fifo_enqueue_ingress(fifo,
            &(vm_machine_request){.kind = VM_MACHINE_REQUEST_PAUSE}) != TYPE_STATUS_OK ||
        vm_machine_executor_fifo_enqueue_ingress(fifo,
            &(vm_machine_request){.kind = VM_MACHINE_REQUEST_STOP}) != TYPE_STATUS_OK) return 1;
    vm_machine_executor_fifo_observe_execution_boundary(fifo);
    vm_machine_executor_fifo_observe_execution_boundary(fifo);
    vm_machine_executor_fifo_observe_execution_boundary(fifo);
    if (observer.count != 3u ||
        observer.requests[0u].kind != VM_MACHINE_REQUEST_INPUT ||
        observer.requests[1u].kind != VM_MACHINE_REQUEST_PAUSE ||
        observer.requests[2u].kind != VM_MACHINE_REQUEST_STOP ||
        vm_machine_executor_fifo_execution_boundary_count(fifo) != 3u) return 1;

    vm_machine_executor_fifo_close(fifo);
    if (vm_machine_executor_fifo_enqueue_ingress(fifo, &input) !=
        TYPE_STATUS_INVALID_STATE) return 1;
    vm_machine_executor_fifo_destroy(fifo);
    return 0;
}
