#ifndef NXVM_CORE_STATUS_H
#define NXVM_CORE_STATUS_H

#ifdef __cplusplus
extern "C" {
#endif

#define NXVM_CORE_ABI_VERSION 1u

typedef enum nxvm_core_status {
    NXVM_CORE_STATUS_OK = 0,
    NXVM_CORE_STATUS_INVALID_ARGUMENT,
    NXVM_CORE_STATUS_INVALID_STATE,
    NXVM_CORE_STATUS_UNSUPPORTED,
    NXVM_CORE_STATUS_NO_MEMORY,
    NXVM_CORE_STATUS_FAULT
} nxvm_core_status;

#ifdef __cplusplus
}
#endif

#endif
