#ifndef VM_MACHINE_EXECUTOR_QUEUE_H
#define VM_MACHINE_EXECUTOR_QUEUE_H


#include "type.h"
#include "vm/events/machine_event.h"

#define VM_MACHINE_EXECUTOR_QUEUE_CAPACITY 32u

typedef struct vm_machine_executor_queue {
    vm_machine_request entries[VM_MACHINE_EXECUTOR_QUEUE_CAPACITY];
    STD_SIZE_T head;
    STD_SIZE_T count;
} vm_machine_executor_queue;

C_VOID vm_machine_executor_queue_initialize(
    vm_machine_executor_queue *bridge);
type_status vm_machine_executor_queue_enqueue(
    vm_machine_executor_queue *bridge,
    const vm_machine_request *request);
type_status vm_machine_executor_queue_dequeue(
    vm_machine_executor_queue *bridge,
    vm_machine_request *out_request);

#endif
