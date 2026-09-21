#ifndef CORE_MACHINE_LIFECYCLE_INTERFACE_H
#define CORE_MACHINE_LIFECYCLE_INTERFACE_H

#ifdef __cplusplus
extern "C" {
#endif

typedef enum core_machine_lifecycle {
    CORE_MACHINE_INITIALIZED = 0,
    CORE_MACHINE_PAUSED,
    CORE_MACHINE_RUNNING,
    CORE_MACHINE_STOPPED,
    CORE_MACHINE_FAULTED
} core_machine_lifecycle;

#ifdef __cplusplus
}
#endif

#endif
