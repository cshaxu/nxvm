#ifndef NXVM_BASELINE_VM_REQUEST_TRANSPORT_H
#define NXVM_BASELINE_VM_REQUEST_TRANSPORT_H

#include <stdatomic.h>

#include "type.h"
#include "vm/platform/request_bridge.h"

typedef void (*vm_platform_request_consumer)(
    void *opaque, const nxvm_platform_vm_request *request);

typedef struct vm_platform_request_transport {
    atomic_bool locked;
    int accepting;
    unsigned execution_boundary_count;
    vm_platform_request_consumer consumer;
    void *consumer_opaque;
    nxvm_platform_vm_request_bridge ingress;
    nxvm_platform_vm_request_bridge egress;
} vm_platform_request_transport;

void vm_platform_request_transport_initialize(
    vm_platform_request_transport *transport);
ntvdm64_status vm_platform_request_transport_enqueue_ingress(
    vm_platform_request_transport *transport,
    const nxvm_platform_vm_request *request);
ntvdm64_status vm_platform_request_transport_dequeue_ingress(
    vm_platform_request_transport *transport,
    nxvm_platform_vm_request *out_request);
ntvdm64_status vm_platform_request_transport_enqueue_egress(
    vm_platform_request_transport *transport,
    const nxvm_platform_vm_request *request);
ntvdm64_status vm_platform_request_transport_dequeue_egress(
    vm_platform_request_transport *transport,
    nxvm_platform_vm_request *out_request);
void vm_platform_request_transport_close(
    vm_platform_request_transport *transport);
void vm_platform_request_transport_discard(
    vm_platform_request_transport *transport);
void vm_platform_request_transport_bind_consumer(
    vm_platform_request_transport *transport,
    vm_platform_request_consumer consumer, void *opaque);
void vm_platform_request_transport_observe_execution_boundary(void *opaque);
unsigned vm_platform_request_transport_execution_boundary_count(
    const vm_platform_request_transport *transport);

#endif
