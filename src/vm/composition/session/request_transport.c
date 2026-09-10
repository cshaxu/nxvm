#include "type.h"

#include "vm/composition/session/request_transport.h"

struct vm_session_request_transport {
    STD_ATOMIC_BOOL locked;
    C_INT accepting;
    C_UINT execution_boundary_count;
    vm_session_request_consumer consumer;
    C_VOID *consumer_opaque;
    vm_session_request_bridge ingress;
};

static C_VOID vm_session_request_transport_lock(
    vm_session_request_transport *transport)
{
    while (STD_ATOMIC_EXCHANGE_EXPLICIT(&transport->locked, 1,
                                    STD_MEMORY_ORDER_ACQUIRE)) {
    }
}

static C_VOID vm_session_request_transport_unlock(
    vm_session_request_transport *transport)
{
    STD_ATOMIC_STORE_EXPLICIT(&transport->locked, 0, STD_MEMORY_ORDER_RELEASE);
}

type_status vm_session_request_transport_create(
    vm_session_request_transport **out_transport)
{
    vm_session_request_transport *transport;

    if (out_transport == STD_NULL) return TYPE_STATUS_INVALID_ARGUMENT;
    *out_transport = STD_CALLOC(1u, sizeof(*transport));
    transport = *out_transport;
    if (transport == STD_NULL) return TYPE_STATUS_NO_MEMORY;

    STD_ATOMIC_INIT(&transport->locked, 0);
    transport->accepting = 1;
    transport->execution_boundary_count = 0u;
    transport->consumer = STD_NULL;
    transport->consumer_opaque = STD_NULL;
    vm_session_request_bridge_initialize(&transport->ingress);
    return TYPE_STATUS_OK;
}

C_VOID vm_session_request_transport_destroy(
    vm_session_request_transport *transport)
{
    if (transport == STD_NULL) return;
    vm_session_request_transport_discard(transport);
    STD_FREE(transport);
}

type_status vm_session_request_transport_enqueue_ingress(
    vm_session_request_transport *transport,
    const vm_session_request *request)
{
    type_status status;
    if (transport == STD_NULL || request == STD_NULL) {
        return TYPE_STATUS_INVALID_ARGUMENT;
    }
    vm_session_request_transport_lock(transport);
    if (!transport->accepting) {
        status = TYPE_STATUS_INVALID_STATE;
    } else {
        status = vm_session_request_bridge_enqueue(&transport->ingress,
            request);
    }
    vm_session_request_transport_unlock(transport);
    return status;
}

type_status vm_session_request_transport_dequeue_ingress(
    vm_session_request_transport *transport,
    vm_session_request *out_request)
{
    type_status status;

    if (transport == STD_NULL || out_request == STD_NULL) {
        return TYPE_STATUS_INVALID_ARGUMENT;
    }
    vm_session_request_transport_lock(transport);
    status = vm_session_request_bridge_dequeue(&transport->ingress,
        out_request);
    vm_session_request_transport_unlock(transport);
    return status;
}

C_VOID vm_session_request_transport_close(
    vm_session_request_transport *transport)
{
    if (transport == STD_NULL) return;

    vm_session_request_transport_lock(transport);
    transport->accepting = 0;
    vm_session_request_transport_unlock(transport);
}

C_VOID vm_session_request_transport_discard(
    vm_session_request_transport *transport)
{
    if (transport == STD_NULL) return;

    vm_session_request_transport_lock(transport);
    transport->accepting = 0;
    vm_session_request_bridge_initialize(&transport->ingress);
    vm_session_request_transport_unlock(transport);
}

C_VOID vm_session_request_transport_bind_consumer(
    vm_session_request_transport *transport,
    vm_session_request_consumer consumer, C_VOID *opaque)
{
    if (transport == STD_NULL) return;

    vm_session_request_transport_lock(transport);
    transport->consumer = consumer;
    transport->consumer_opaque = opaque;
    vm_session_request_transport_unlock(transport);
}

C_VOID vm_session_request_transport_observe_execution_boundary(C_VOID *opaque)
{
    vm_session_request_transport *transport =
        (vm_session_request_transport *)opaque;
    vm_session_request request;
    vm_session_request_consumer consumer;
    C_VOID *consumer_opaque;

    if (transport == STD_NULL) return;
    vm_session_request_transport_lock(transport);
    ++transport->execution_boundary_count;
    vm_session_request_transport_unlock(transport);
    /* One host event per guest instruction boundary preserves keyboard chord
     * ordering: an IRQ1 handler can observe Alt before a later host key-up
     * snapshot is accepted.  Draining the whole ingress queue collapsed a
     * physical chord into one guest instant. */
    vm_session_request_transport_lock(transport);
    if (vm_session_request_bridge_dequeue(&transport->ingress, &request) !=
        TYPE_STATUS_OK) {
        vm_session_request_transport_unlock(transport);
        return;
    }
    consumer = transport->consumer;
    consumer_opaque = transport->consumer_opaque;
    vm_session_request_transport_unlock(transport);
    if (consumer != STD_NULL) consumer(consumer_opaque, &request);
}

C_UINT vm_session_request_transport_execution_boundary_count(
    const vm_session_request_transport *transport)
{
    return transport != STD_NULL ? transport->execution_boundary_count : 0u;
}
