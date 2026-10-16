#ifndef X86_CORE_MACHINE_LIFECYCLE_INTERFACE_H
#define X86_CORE_MACHINE_LIFECYCLE_INTERFACE_H

#ifdef __cplusplus
extern "C" {
#endif

typedef enum x86_core_machine_lifecycle {
    X86_CORE_MACHINE_INITIALIZED = 0,
    X86_CORE_MACHINE_PAUSED,
    X86_CORE_MACHINE_RUNNING,
    X86_CORE_MACHINE_STOPPED,
    X86_CORE_MACHINE_FAULTED
} x86_core_machine_lifecycle;

#ifdef __cplusplus
}
#endif

#endif
