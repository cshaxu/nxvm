#include "type.h"

#include "vm/platform/vm_request_transport.h"

static C_VOID vm_platform_request_transport_lock(
    vm_platform_request_transport *transport)
{
    while (STD_ATOMIC_EXCHANGE_EXPLICIT(&transport->locked, 1,
                                    STD_MEMORY_ORDER_ACQUIRE)) {
    }
}

static C_VOID vm_platform_request_transport_unlock(
    vm_platform_request_transport *transport)
{
    STD_ATOMIC_STORE_EXPLICIT(&transport->locked, 0, STD_MEMORY_ORDER_RELEASE);
}

C_VOID vm_platform_request_transport_initialize(
    vm_platform_request_transport *transport)
{
    if (transport == STD_NULL) return;

    STD_ATOMIC_INIT(&transport->locked, 0);
    transport->accepting = 1;
    transport->execution_boundary_count = 0u;
    transport->consumer = STD_NULL;
    transport->consumer_opaque = STD_NULL;
    vm_platform_request_bridge_initialize(&transport->ingress);
    vm_platform_request_bridge_initialize(&transport->egress);
}

static type_status vm_platform_request_transport_enqueue(
    vm_platform_request_transport *transport,
    vm_platform_request_bridge *bridge,
    const vm_platform_request *request)
{
    type_status status;

    if (transport == STD_NULL || request == STD_NULL) {
        return TYPE_STATUS_INVALID_ARGUMENT;
    }

    vm_platform_request_transport_lock(transport);
    status = transport->accepting
        ? vm_platform_request_bridge_enqueue(bridge, request)
        : TYPE_STATUS_INVALID_STATE;
    vm_platform_request_transport_unlock(transport);
    return status;
}

static type_status vm_platform_request_transport_dequeue(
    vm_platform_request_transport *transport,
    vm_platform_request_bridge *bridge,
    vm_platform_request *out_request)
{
    type_status status;

    if (transport == STD_NULL || out_request == STD_NULL) {
        return TYPE_STATUS_INVALID_ARGUMENT;
    }

    vm_platform_request_transport_lock(transport);
    status = vm_platform_request_bridge_dequeue(bridge, out_request);
    vm_platform_request_transport_unlock(transport);
    return status;
}

type_status vm_platform_request_transport_enqueue_ingress(
    vm_platform_request_transport *transport,
    const vm_platform_request *request)
{
    return vm_platform_request_transport_enqueue(
        transport, transport != STD_NULL ? &transport->ingress : STD_NULL, request);
}

type_status vm_platform_request_transport_dequeue_ingress(
    vm_platform_request_transport *transport,
    vm_platform_request *out_request)
{
    return vm_platform_request_transport_dequeue(
        transport, transport != STD_NULL ? &transport->ingress : STD_NULL, out_request);
}

type_status vm_platform_request_transport_enqueue_egress(
    vm_platform_request_transport *transport,
    const vm_platform_request *request)
{
    return vm_platform_request_transport_enqueue(
        transport, transport != STD_NULL ? &transport->egress : STD_NULL, request);
}

type_status vm_platform_request_transport_dequeue_egress(
    vm_platform_request_transport *transport,
    vm_platform_request *out_request)
{
    return vm_platform_request_transport_dequeue(
        transport, transport != STD_NULL ? &transport->egress : STD_NULL, out_request);
}

C_VOID vm_platform_request_transport_close(
    vm_platform_request_transport *transport)
{
    if (transport == STD_NULL) return;

    vm_platform_request_transport_lock(transport);
    transport->accepting = 0;
    vm_platform_request_transport_unlock(transport);
}

C_VOID vm_platform_request_transport_discard(
    vm_platform_request_transport *transport)
{
    if (transport == STD_NULL) return;

    vm_platform_request_transport_lock(transport);
    transport->accepting = 0;
    vm_platform_request_bridge_initialize(&transport->ingress);
    vm_platform_request_bridge_initialize(&transport->egress);
    vm_platform_request_transport_unlock(transport);
}

C_VOID vm_platform_request_transport_bind_consumer(
    vm_platform_request_transport *transport,
    vm_platform_request_consumer consumer, C_VOID *opaque)
{
    if (transport == STD_NULL) return;

    vm_platform_request_transport_lock(transport);
    transport->consumer = consumer;
    transport->consumer_opaque = opaque;
    vm_platform_request_transport_unlock(transport);
}

C_VOID vm_platform_request_transport_observe_execution_boundary(C_VOID *opaque)
{
    vm_platform_request_transport *transport =
        (vm_platform_request_transport *)opaque;
    vm_platform_request request;
    vm_platform_request_consumer consumer;
    C_VOID *consumer_opaque;

    if (transport == STD_NULL) return;
    vm_platform_request_transport_lock(transport);
    ++transport->execution_boundary_count;
    vm_platform_request_transport_unlock(transport);
    for (;;) {
        vm_platform_request_transport_lock(transport);
        if (vm_platform_request_bridge_dequeue(&transport->ingress,
                                                    &request) !=
            TYPE_STATUS_OK) {
            vm_platform_request_transport_unlock(transport);
            return;
        }
        consumer = transport->consumer;
        consumer_opaque = transport->consumer_opaque;
        vm_platform_request_transport_unlock(transport);
        if (consumer != STD_NULL) consumer(consumer_opaque, &request);
    }
}

C_UINT vm_platform_request_transport_execution_boundary_count(
    const vm_platform_request_transport *transport)
{
    return transport != STD_NULL ? transport->execution_boundary_count : 0u;
}
