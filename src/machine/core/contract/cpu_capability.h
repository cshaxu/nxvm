#ifndef NXVM_CORE_CPU_CAPABILITY_H
#define NXVM_CORE_CPU_CAPABILITY_H

#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef enum nxvm_core_cpu_capability {
    NXVM_CORE_CPU_CAPABILITY_REAL_MODE_8086 = 0,
    NXVM_CORE_CPU_CAPABILITY_PROTECTED_MODE_286,
    NXVM_CORE_CPU_CAPABILITY_I386_DECODE,
    NXVM_CORE_CPU_CAPABILITY_I386_REAL_MODE,
    NXVM_CORE_CPU_CAPABILITY_I386_PROTECTED_MODE,
    NXVM_CORE_CPU_CAPABILITY_I386_PAGING,
    NXVM_CORE_CPU_CAPABILITY_COUNT
} nxvm_core_cpu_capability;

typedef enum nxvm_core_cpu_capability_verdict {
    NXVM_CORE_CPU_CAPABILITY_UNKNOWN = 0,
    NXVM_CORE_CPU_CAPABILITY_PROVEN,
    NXVM_CORE_CPU_CAPABILITY_UNSUPPORTED,
    NXVM_CORE_CPU_CAPABILITY_FAILED
} nxvm_core_cpu_capability_verdict;

typedef struct nxvm_core_cpu_capability_manifest {
    nxvm_core_cpu_capability_verdict verdicts[NXVM_CORE_CPU_CAPABILITY_COUNT];
} nxvm_core_cpu_capability_manifest;

void nxvm_core_cpu_capability_manifest_initialize(
    nxvm_core_cpu_capability_manifest *manifest);
int nxvm_core_cpu_capability_manifest_set(
    nxvm_core_cpu_capability_manifest *manifest,
    nxvm_core_cpu_capability capability,
    nxvm_core_cpu_capability_verdict verdict);
nxvm_core_cpu_capability_verdict nxvm_core_cpu_capability_manifest_get(
    const nxvm_core_cpu_capability_manifest *manifest,
    nxvm_core_cpu_capability capability);
const char *nxvm_core_cpu_capability_name(
    nxvm_core_cpu_capability capability);

#ifdef __cplusplus
}
#endif

#endif
