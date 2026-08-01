#include "adapters/nxvm_baseline/vm_request_transport.h"

static void nxvm_baseline_vm_request_transport_lock(
    nxvm_baseline_vm_request_transport *transport)
{
    while (atomic_exchange_explicit(&transport->locked, 1,
                                    memory_order_acquire)) {
    }
}

static void nxvm_baseline_vm_request_transport_unlock(
    nxvm_baseline_vm_request_transport *transport)
{
    atomic_store_explicit(&transport->locked, 0, memory_order_release);
}

void nxvm_baseline_vm_request_transport_initialize(
    nxvm_baseline_vm_request_transport *transport)
{
    if (transport == NULL) return;

    atomic_init(&transport->locked, 0);
    transport->accepting = 1;
    nxvm_platform_vm_request_bridge_initialize(&transport->ingress);
    nxvm_platform_vm_request_bridge_initialize(&transport->egress);
}

static nxvm_core_status nxvm_baseline_vm_request_transport_enqueue(
    nxvm_baseline_vm_request_transport *transport,
    nxvm_platform_vm_request_bridge *bridge,
    const nxvm_platform_vm_request *request)
{
    nxvm_core_status status;

    if (transport == NULL || request == NULL) {
        return NXVM_CORE_STATUS_INVALID_ARGUMENT;
    }

    nxvm_baseline_vm_request_transport_lock(transport);
    status = transport->accepting
        ? nxvm_platform_vm_request_bridge_enqueue(bridge, request)
        : NXVM_CORE_STATUS_INVALID_STATE;
    nxvm_baseline_vm_request_transport_unlock(transport);
    return status;
}

static nxvm_core_status nxvm_baseline_vm_request_transport_dequeue(
    nxvm_baseline_vm_request_transport *transport,
    nxvm_platform_vm_request_bridge *bridge,
    nxvm_platform_vm_request *out_request)
{
    nxvm_core_status status;

    if (transport == NULL || out_request == NULL) {
        return NXVM_CORE_STATUS_INVALID_ARGUMENT;
    }

    nxvm_baseline_vm_request_transport_lock(transport);
    status = nxvm_platform_vm_request_bridge_dequeue(bridge, out_request);
    nxvm_baseline_vm_request_transport_unlock(transport);
    return status;
}

nxvm_core_status nxvm_baseline_vm_request_transport_enqueue_ingress(
    nxvm_baseline_vm_request_transport *transport,
    const nxvm_platform_vm_request *request)
{
    return nxvm_baseline_vm_request_transport_enqueue(
        transport, transport != NULL ? &transport->ingress : NULL, request);
}

nxvm_core_status nxvm_baseline_vm_request_transport_dequeue_ingress(
    nxvm_baseline_vm_request_transport *transport,
    nxvm_platform_vm_request *out_request)
{
    return nxvm_baseline_vm_request_transport_dequeue(
        transport, transport != NULL ? &transport->ingress : NULL, out_request);
}

nxvm_core_status nxvm_baseline_vm_request_transport_enqueue_egress(
    nxvm_baseline_vm_request_transport *transport,
    const nxvm_platform_vm_request *request)
{
    return nxvm_baseline_vm_request_transport_enqueue(
        transport, transport != NULL ? &transport->egress : NULL, request);
}

nxvm_core_status nxvm_baseline_vm_request_transport_dequeue_egress(
    nxvm_baseline_vm_request_transport *transport,
    nxvm_platform_vm_request *out_request)
{
    return nxvm_baseline_vm_request_transport_dequeue(
        transport, transport != NULL ? &transport->egress : NULL, out_request);
}

void nxvm_baseline_vm_request_transport_close(
    nxvm_baseline_vm_request_transport *transport)
{
    if (transport == NULL) return;

    nxvm_baseline_vm_request_transport_lock(transport);
    transport->accepting = 0;
    nxvm_baseline_vm_request_transport_unlock(transport);
}

void nxvm_baseline_vm_request_transport_discard(
    nxvm_baseline_vm_request_transport *transport)
{
    if (transport == NULL) return;

    nxvm_baseline_vm_request_transport_lock(transport);
    transport->accepting = 0;
    nxvm_platform_vm_request_bridge_initialize(&transport->ingress);
    nxvm_platform_vm_request_bridge_initialize(&transport->egress);
    nxvm_baseline_vm_request_transport_unlock(transport);
}
