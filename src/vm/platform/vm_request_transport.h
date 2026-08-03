#ifndef NXVM_BASELINE_VM_REQUEST_TRANSPORT_H
#define NXVM_BASELINE_VM_REQUEST_TRANSPORT_H

#include <stdatomic.h>

#include "type.h"
#include "vm/platform/request_bridge.h"

typedef void (*nxvm_vm_request_consumer)(
    void *opaque, const nxvm_platform_vm_request *request);

typedef struct nxvm_vm_request_transport {
    atomic_bool locked;
    int accepting;
    unsigned execution_boundary_count;
    nxvm_vm_request_consumer consumer;
    void *consumer_opaque;
    nxvm_platform_vm_request_bridge ingress;
    nxvm_platform_vm_request_bridge egress;
} nxvm_vm_request_transport;

void nxvm_vm_request_transport_initialize(
    nxvm_vm_request_transport *transport);
ntvdm64_status nxvm_vm_request_transport_enqueue_ingress(
    nxvm_vm_request_transport *transport,
    const nxvm_platform_vm_request *request);
ntvdm64_status nxvm_vm_request_transport_dequeue_ingress(
    nxvm_vm_request_transport *transport,
    nxvm_platform_vm_request *out_request);
ntvdm64_status nxvm_vm_request_transport_enqueue_egress(
    nxvm_vm_request_transport *transport,
    const nxvm_platform_vm_request *request);
ntvdm64_status nxvm_vm_request_transport_dequeue_egress(
    nxvm_vm_request_transport *transport,
    nxvm_platform_vm_request *out_request);
void nxvm_vm_request_transport_close(
    nxvm_vm_request_transport *transport);
void nxvm_vm_request_transport_discard(
    nxvm_vm_request_transport *transport);
void nxvm_vm_request_transport_bind_consumer(
    nxvm_vm_request_transport *transport,
    nxvm_vm_request_consumer consumer, void *opaque);
void nxvm_vm_request_transport_observe_execution_boundary(void *opaque);
unsigned nxvm_vm_request_transport_execution_boundary_count(
    const nxvm_vm_request_transport *transport);

#endif
