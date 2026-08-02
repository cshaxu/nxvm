#ifndef NTVDM64_VM_PLATFORM_EXECUTION_H
#define NTVDM64_VM_PLATFORM_EXECUTION_H

typedef struct vm_platform_execution_sink {
    int (*is_running)(void *context);
    int (*get_flip)(void *context);
    void (*start)(void *context);
    void (*stop)(void *context);
} vm_platform_execution_sink;

typedef struct vm_platform_execution_transport {
    const vm_platform_execution_sink *sink;
    void *context;
} vm_platform_execution_transport;

void vm_platform_execution_transport_initialize(
    vm_platform_execution_transport *transport,
    const vm_platform_execution_sink *sink, void *context);
int vm_platform_execution_is_running_for(
    const vm_platform_execution_transport *transport);
int vm_platform_execution_get_flip_for(
    const vm_platform_execution_transport *transport);
void vm_platform_execution_start_for(
    const vm_platform_execution_transport *transport);
void vm_platform_execution_stop_for(
    const vm_platform_execution_transport *transport);

void vm_platform_execution_bind(const vm_platform_execution_sink *sink,
                                void *context);
int vm_platform_execution_is_running(void);
int vm_platform_execution_get_flip(void);
void vm_platform_execution_start(void);
void vm_platform_execution_stop(void);

#endif
