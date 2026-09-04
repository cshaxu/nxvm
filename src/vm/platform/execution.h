#ifndef VM_PLATFORM_EXECUTION_H
#define VM_PLATFORM_EXECUTION_H


#include "type.h"

#define VM_PLATFORM_EXECUTION_FLIP_TIMEOUT_MILLISECONDS 5000u
typedef enum vm_platform_execution_lifecycle {
    VM_PLATFORM_EXECUTION_STOPPED,
    VM_PLATFORM_EXECUTION_RUNNING,
    VM_PLATFORM_EXECUTION_PAUSED
} vm_platform_execution_lifecycle;

typedef struct vm_platform_execution_sink {
    vm_platform_execution_lifecycle (*get_lifecycle)(C_VOID *context);
    C_INT (*get_flip)(C_VOID *context);
    C_VOID (*start)(C_VOID *context);
    C_VOID (*stop)(C_VOID *context);
} vm_platform_execution_sink;

typedef struct vm_platform_execution_transport vm_platform_execution_transport;

type_status vm_platform_execution_transport_create(
    const vm_platform_execution_sink *sink, C_VOID *context,
    vm_platform_execution_transport **out_transport);
C_VOID vm_platform_execution_transport_destroy(
    vm_platform_execution_transport *transport);
C_INT vm_platform_execution_is_running_for(
    const vm_platform_execution_transport *transport);
vm_platform_execution_lifecycle vm_platform_execution_get_lifecycle_for(
    const vm_platform_execution_transport *transport);
C_INT vm_platform_execution_get_flip_for(
    const vm_platform_execution_transport *transport);
C_VOID vm_platform_execution_start_for(
    const vm_platform_execution_transport *transport);
C_VOID vm_platform_execution_stop_for(
    const vm_platform_execution_transport *transport);
C_INT vm_platform_execution_wait_for_flip_for(
    const vm_platform_execution_transport *transport, C_INT initial_flip,
    type_unsigned_32 timeout_milliseconds);

#endif
