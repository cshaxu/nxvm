#ifndef NTVDM64_VM_PLATFORM_EXECUTION_H
#define NTVDM64_VM_PLATFORM_EXECUTION_H


#include "type.h"
typedef struct vm_platform_execution_sink {
    C_INT (*is_running)(C_VOID *context);
    C_INT (*get_flip)(C_VOID *context);
    C_VOID (*start)(C_VOID *context);
    C_VOID (*stop)(C_VOID *context);
} vm_platform_execution_sink;

typedef struct vm_platform_execution_transport {
    const vm_platform_execution_sink *sink;
    C_VOID *context;
} vm_platform_execution_transport;

C_VOID vm_platform_execution_transport_initialize(
    vm_platform_execution_transport *transport,
    const vm_platform_execution_sink *sink, C_VOID *context);
C_INT vm_platform_execution_is_running_for(
    const vm_platform_execution_transport *transport);
C_INT vm_platform_execution_get_flip_for(
    const vm_platform_execution_transport *transport);
C_VOID vm_platform_execution_start_for(
    const vm_platform_execution_transport *transport);
C_VOID vm_platform_execution_stop_for(
    const vm_platform_execution_transport *transport);

#endif
