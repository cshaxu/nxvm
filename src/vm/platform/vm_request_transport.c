#include "type.h"

#include "vm/platform/vm_request_transport.h"

static C_VOID vm_platform_request_transport_lock(
    vm_platform_request_transport *transport)
{
    while (atomic_exchange_explicit(&transport->locked, 1,
                                    memory_order_acquire)) {
    }
}

static C_VOID vm_platform_request_transport_unlock(
    vm_platform_request_transport *transport)
{
    atomic_store_explicit(&transport->locked, 0, memory_order_release);
}

C_VOID vm_platform_request_transport_initialize(
    vm_platform_request_transport *transport)
{
    if (transport == NULL) return;

    atomic_init(&transport->locked, 0);
    transport->accepting = 1;
    transport->execution_boundary_count = 0u;
    transport->consumer = NULL;
    transport->consumer_opaque = NULL;
    nxvm_platform_vm_request_bridge_initialize(&transport->ingress);
    nxvm_platform_vm_request_bridge_initialize(&transport->egress);
}

static ntvdm64_status vm_platform_request_transport_enqueue(
    vm_platform_request_transport *transport,
    nxvm_platform_vm_request_bridge *bridge,
    const nxvm_platform_vm_request *request)
{
    ntvdm64_status status;

    if (transport == NULL || request == NULL) {
        return NTVDM64_STATUS_INVALID_ARGUMENT;
    }

    vm_platform_request_transport_lock(transport);
    status = transport->accepting
        ? nxvm_platform_vm_request_bridge_enqueue(bridge, request)
        : NTVDM64_STATUS_INVALID_STATE;
    vm_platform_request_transport_unlock(transport);
    return status;
}

static ntvdm64_status vm_platform_request_transport_dequeue(
    vm_platform_request_transport *transport,
    nxvm_platform_vm_request_bridge *bridge,
    nxvm_platform_vm_request *out_request)
{
    ntvdm64_status status;

    if (transport == NULL || out_request == NULL) {
        return NTVDM64_STATUS_INVALID_ARGUMENT;
    }

    vm_platform_request_transport_lock(transport);
    status = nxvm_platform_vm_request_bridge_dequeue(bridge, out_request);
    vm_platform_request_transport_unlock(transport);
    return status;
}

ntvdm64_status vm_platform_request_transport_enqueue_ingress(
    vm_platform_request_transport *transport,
    const nxvm_platform_vm_request *request)
{
    return vm_platform_request_transport_enqueue(
        transport, transport != NULL ? &transport->ingress : NULL, request);
}

ntvdm64_status vm_platform_request_transport_dequeue_ingress(
    vm_platform_request_transport *transport,
    nxvm_platform_vm_request *out_request)
{
    return vm_platform_request_transport_dequeue(
        transport, transport != NULL ? &transport->ingress : NULL, out_request);
}

ntvdm64_status vm_platform_request_transport_enqueue_egress(
    vm_platform_request_transport *transport,
    const nxvm_platform_vm_request *request)
{
    return vm_platform_request_transport_enqueue(
        transport, transport != NULL ? &transport->egress : NULL, request);
}

ntvdm64_status vm_platform_request_transport_dequeue_egress(
    vm_platform_request_transport *transport,
    nxvm_platform_vm_request *out_request)
{
    return vm_platform_request_transport_dequeue(
        transport, transport != NULL ? &transport->egress : NULL, out_request);
}

C_VOID vm_platform_request_transport_close(
    vm_platform_request_transport *transport)
{
    if (transport == NULL) return;

    vm_platform_request_transport_lock(transport);
    transport->accepting = 0;
    vm_platform_request_transport_unlock(transport);
}

C_VOID vm_platform_request_transport_discard(
    vm_platform_request_transport *transport)
{
    if (transport == NULL) return;

    vm_platform_request_transport_lock(transport);
    transport->accepting = 0;
    nxvm_platform_vm_request_bridge_initialize(&transport->ingress);
    nxvm_platform_vm_request_bridge_initialize(&transport->egress);
    vm_platform_request_transport_unlock(transport);
}

C_VOID vm_platform_request_transport_bind_consumer(
    vm_platform_request_transport *transport,
    vm_platform_request_consumer consumer, C_VOID *opaque)
{
    if (transport == NULL) return;

    vm_platform_request_transport_lock(transport);
    transport->consumer = consumer;
    transport->consumer_opaque = opaque;
    vm_platform_request_transport_unlock(transport);
}

C_VOID vm_platform_request_transport_observe_execution_boundary(C_VOID *opaque)
{
    vm_platform_request_transport *transport =
        (vm_platform_request_transport *)opaque;
    nxvm_platform_vm_request request;
    vm_platform_request_consumer consumer;
    C_VOID *consumer_opaque;

    if (transport == NULL) return;
    vm_platform_request_transport_lock(transport);
    ++transport->execution_boundary_count;
    vm_platform_request_transport_unlock(transport);
    for (;;) {
        vm_platform_request_transport_lock(transport);
        if (nxvm_platform_vm_request_bridge_dequeue(&transport->ingress,
                                                    &request) !=
            NTVDM64_STATUS_OK) {
            vm_platform_request_transport_unlock(transport);
            return;
        }
        consumer = transport->consumer;
        consumer_opaque = transport->consumer_opaque;
        vm_platform_request_transport_unlock(transport);
        if (consumer != NULL) consumer(consumer_opaque, &request);
    }
}

C_UINT vm_platform_request_transport_execution_boundary_count(
    const vm_platform_request_transport *transport)
{
    return transport != NULL ? transport->execution_boundary_count : 0u;
}
