#include "type.h"

#include "vm/machine/executor_fifo.h"

struct vm_machine_executor_fifo {
    STD_ATOMIC_BOOL locked;
    C_INT accepting;
    C_UINT execution_boundary_count;
    vm_machine_executor_request_consumer consumer;
    C_VOID *consumer_opaque;
    vm_machine_executor_queue ingress;
};

static C_VOID vm_machine_executor_fifo_lock(
    vm_machine_executor_fifo *transport)
{
    while (STD_ATOMIC_EXCHANGE_EXPLICIT(&transport->locked, 1,
                                    STD_MEMORY_ORDER_ACQUIRE)) {
    }
}

static C_VOID vm_machine_executor_fifo_unlock(
    vm_machine_executor_fifo *transport)
{
    STD_ATOMIC_STORE_EXPLICIT(&transport->locked, 0, STD_MEMORY_ORDER_RELEASE);
}

type_status vm_machine_executor_fifo_create(
    vm_machine_executor_fifo **out_transport)
{
    vm_machine_executor_fifo *transport;

    if (out_transport == STD_NULL) return TYPE_STATUS_INVALID_ARGUMENT;
    *out_transport = STD_CALLOC(1u, sizeof(*transport));
    transport = *out_transport;
    if (transport == STD_NULL) return TYPE_STATUS_NO_MEMORY;

    STD_ATOMIC_INIT(&transport->locked, 0);
    transport->accepting = 1;
    transport->execution_boundary_count = 0u;
    transport->consumer = STD_NULL;
    transport->consumer_opaque = STD_NULL;
    vm_machine_executor_queue_initialize(&transport->ingress);
    return TYPE_STATUS_OK;
}

C_VOID vm_machine_executor_fifo_destroy(
    vm_machine_executor_fifo *transport)
{
    if (transport == STD_NULL) return;
    vm_machine_executor_fifo_discard(transport);
    STD_FREE(transport);
}

type_status vm_machine_executor_fifo_enqueue_ingress(
    vm_machine_executor_fifo *transport,
    const vm_machine_request *request)
{
    type_status status;
    if (transport == STD_NULL || request == STD_NULL) {
        return TYPE_STATUS_INVALID_ARGUMENT;
    }
    vm_machine_executor_fifo_lock(transport);
    if (!transport->accepting) {
        status = TYPE_STATUS_INVALID_STATE;
    } else {
        status = vm_machine_executor_queue_enqueue(&transport->ingress,
            request);
    }
    vm_machine_executor_fifo_unlock(transport);
    return status;
}

type_status vm_machine_executor_fifo_dequeue_ingress(
    vm_machine_executor_fifo *transport,
    vm_machine_request *out_request)
{
    type_status status;

    if (transport == STD_NULL || out_request == STD_NULL) {
        return TYPE_STATUS_INVALID_ARGUMENT;
    }
    vm_machine_executor_fifo_lock(transport);
    status = vm_machine_executor_queue_dequeue(&transport->ingress,
        out_request);
    vm_machine_executor_fifo_unlock(transport);
    return status;
}

C_VOID vm_machine_executor_fifo_close(
    vm_machine_executor_fifo *transport)
{
    if (transport == STD_NULL) return;

    vm_machine_executor_fifo_lock(transport);
    transport->accepting = 0;
    vm_machine_executor_fifo_unlock(transport);
}

C_VOID vm_machine_executor_fifo_discard(
    vm_machine_executor_fifo *transport)
{
    if (transport == STD_NULL) return;

    vm_machine_executor_fifo_lock(transport);
    transport->accepting = 0;
    vm_machine_executor_queue_initialize(&transport->ingress);
    vm_machine_executor_fifo_unlock(transport);
}

C_VOID vm_machine_executor_fifo_bind_consumer(
    vm_machine_executor_fifo *transport,
    vm_machine_executor_request_consumer consumer, C_VOID *opaque)
{
    if (transport == STD_NULL) return;

    vm_machine_executor_fifo_lock(transport);
    transport->consumer = consumer;
    transport->consumer_opaque = opaque;
    vm_machine_executor_fifo_unlock(transport);
}

C_VOID vm_machine_executor_fifo_observe_execution_boundary(C_VOID *opaque)
{
    vm_machine_executor_fifo *transport =
        (vm_machine_executor_fifo *)opaque;
    vm_machine_request request;
    vm_machine_executor_request_consumer consumer;
    C_VOID *consumer_opaque;

    if (transport == STD_NULL) return;
    vm_machine_executor_fifo_lock(transport);
    ++transport->execution_boundary_count;
    vm_machine_executor_fifo_unlock(transport);
    /* One host event per guest instruction boundary preserves keyboard chord
     * ordering: an IRQ1 handler can observe Alt before a later host key-up
     * snapshot is accepted.  Draining the whole ingress queue collapsed a
     * physical chord into one guest instant. */
    vm_machine_executor_fifo_lock(transport);
    if (vm_machine_executor_queue_dequeue(&transport->ingress, &request) !=
        TYPE_STATUS_OK) {
        vm_machine_executor_fifo_unlock(transport);
        return;
    }
    consumer = transport->consumer;
    consumer_opaque = transport->consumer_opaque;
    vm_machine_executor_fifo_unlock(transport);
    if (consumer != STD_NULL) consumer(consumer_opaque, &request);
}

C_UINT vm_machine_executor_fifo_execution_boundary_count(
    const vm_machine_executor_fifo *transport)
{
    return transport != STD_NULL ? transport->execution_boundary_count : 0u;
}
