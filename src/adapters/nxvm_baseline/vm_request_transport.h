#ifndef NXVM_BASELINE_VM_REQUEST_TRANSPORT_H
#define NXVM_BASELINE_VM_REQUEST_TRANSPORT_H

#include <stdatomic.h>

#include "machine/core/status.h"
#include "platform/vm/request_bridge.h"

typedef struct nxvm_baseline_vm_request_transport {
    atomic_bool locked;
    int accepting;
    unsigned execution_boundary_count;
    nxvm_platform_vm_request_bridge ingress;
    nxvm_platform_vm_request_bridge egress;
} nxvm_baseline_vm_request_transport;

void nxvm_baseline_vm_request_transport_initialize(
    nxvm_baseline_vm_request_transport *transport);
nxvm_core_status nxvm_baseline_vm_request_transport_enqueue_ingress(
    nxvm_baseline_vm_request_transport *transport,
    const nxvm_platform_vm_request *request);
nxvm_core_status nxvm_baseline_vm_request_transport_dequeue_ingress(
    nxvm_baseline_vm_request_transport *transport,
    nxvm_platform_vm_request *out_request);
nxvm_core_status nxvm_baseline_vm_request_transport_enqueue_egress(
    nxvm_baseline_vm_request_transport *transport,
    const nxvm_platform_vm_request *request);
nxvm_core_status nxvm_baseline_vm_request_transport_dequeue_egress(
    nxvm_baseline_vm_request_transport *transport,
    nxvm_platform_vm_request *out_request);
void nxvm_baseline_vm_request_transport_close(
    nxvm_baseline_vm_request_transport *transport);
void nxvm_baseline_vm_request_transport_discard(
    nxvm_baseline_vm_request_transport *transport);
void nxvm_baseline_vm_request_transport_observe_execution_boundary(void *opaque);
unsigned nxvm_baseline_vm_request_transport_execution_boundary_count(
    const nxvm_baseline_vm_request_transport *transport);

#endif
