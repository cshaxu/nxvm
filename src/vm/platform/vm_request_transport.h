#ifndef VM_PLATFORM_REQUEST_TRANSPORT_H
#define VM_PLATFORM_REQUEST_TRANSPORT_H


#include "type.h"
#include "vm/platform/request_bridge.h"

typedef C_VOID (*vm_platform_request_consumer)(
    C_VOID *opaque, const vm_platform_request *request);

typedef struct vm_platform_request_transport vm_platform_request_transport;

type_status vm_platform_request_transport_create(
    vm_platform_request_transport **out_transport);
C_VOID vm_platform_request_transport_destroy(
    vm_platform_request_transport *transport);
type_status vm_platform_request_transport_enqueue_ingress(
    vm_platform_request_transport *transport,
    const vm_platform_request *request);
type_status vm_platform_request_transport_dequeue_ingress(
    vm_platform_request_transport *transport,
    vm_platform_request *out_request);
C_VOID vm_platform_request_transport_close(
    vm_platform_request_transport *transport);
C_VOID vm_platform_request_transport_discard(
    vm_platform_request_transport *transport);
C_VOID vm_platform_request_transport_bind_consumer(
    vm_platform_request_transport *transport,
    vm_platform_request_consumer consumer, C_VOID *opaque);
C_VOID vm_platform_request_transport_observe_execution_boundary(C_VOID *opaque);
C_UINT vm_platform_request_transport_execution_boundary_count(
    const vm_platform_request_transport *transport);

#endif
