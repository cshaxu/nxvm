#ifndef VM_MACHINE_EXECUTOR_FIFO_H
#define VM_MACHINE_EXECUTOR_FIFO_H


#include "type.h"
#include "vm/machine/executor_queue.h"

typedef C_VOID (*vm_machine_executor_request_consumer)(
    C_VOID *opaque, const vm_machine_request *request);

typedef struct vm_machine_executor_fifo vm_machine_executor_fifo;

type_status vm_machine_executor_fifo_create(
    vm_machine_executor_fifo **out_transport);
C_VOID vm_machine_executor_fifo_destroy(
    vm_machine_executor_fifo *transport);
type_status vm_machine_executor_fifo_enqueue_ingress(
    vm_machine_executor_fifo *transport,
    const vm_machine_request *request);
type_status vm_machine_executor_fifo_dequeue_ingress(
    vm_machine_executor_fifo *transport,
    vm_machine_request *out_request);
C_VOID vm_machine_executor_fifo_close(
    vm_machine_executor_fifo *transport);
C_VOID vm_machine_executor_fifo_discard(
    vm_machine_executor_fifo *transport);
C_VOID vm_machine_executor_fifo_bind_consumer(
    vm_machine_executor_fifo *transport,
    vm_machine_executor_request_consumer consumer, C_VOID *opaque);
C_VOID vm_machine_executor_fifo_observe_execution_boundary(C_VOID *opaque);
C_UINT vm_machine_executor_fifo_execution_boundary_count(
    const vm_machine_executor_fifo *transport);

#endif
