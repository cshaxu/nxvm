#ifndef NXVM_CORE_LIFECYCLE_H
#define NXVM_CORE_LIFECYCLE_H

#ifdef __cplusplus
extern "C" {
#endif

typedef enum nxvm_core_machine_lifecycle {
    NXVM_CORE_MACHINE_INITIALIZED = 0,
    NXVM_CORE_MACHINE_PAUSED,
    NXVM_CORE_MACHINE_RUNNING,
    NXVM_CORE_MACHINE_STOPPED,
    NXVM_CORE_MACHINE_FAULTED
} nxvm_core_machine_lifecycle;

#ifdef __cplusplus
}
#endif

#endif
