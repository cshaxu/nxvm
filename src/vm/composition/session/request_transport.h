#ifndef VM_SESSION_REQUEST_TRANSPORT_H
#define VM_SESSION_REQUEST_TRANSPORT_H


#include "type.h"
#include "vm/composition/session/request_bridge.h"

typedef C_VOID (*vm_session_request_consumer)(
    C_VOID *opaque, const vm_session_request *request);

typedef struct vm_session_request_transport vm_session_request_transport;

type_status vm_session_request_transport_create(
    vm_session_request_transport **out_transport);
C_VOID vm_session_request_transport_destroy(
    vm_session_request_transport *transport);
type_status vm_session_request_transport_enqueue_ingress(
    vm_session_request_transport *transport,
    const vm_session_request *request);
type_status vm_session_request_transport_dequeue_ingress(
    vm_session_request_transport *transport,
    vm_session_request *out_request);
C_VOID vm_session_request_transport_close(
    vm_session_request_transport *transport);
C_VOID vm_session_request_transport_discard(
    vm_session_request_transport *transport);
C_VOID vm_session_request_transport_bind_consumer(
    vm_session_request_transport *transport,
    vm_session_request_consumer consumer, C_VOID *opaque);
C_VOID vm_session_request_transport_observe_execution_boundary(C_VOID *opaque);
C_UINT vm_session_request_transport_execution_boundary_count(
    const vm_session_request_transport *transport);

#endif
