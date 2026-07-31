#include <stdio.h>

#include "core/cpu_capability.h"

int main(void)
{
    nxvm_core_cpu_capability_manifest manifest;

    nxvm_core_cpu_capability_manifest_initialize(&manifest);
    if (nxvm_core_cpu_capability_manifest_get(
            &manifest, NXVM_CORE_CPU_CAPABILITY_REAL_MODE_8086) !=
        NXVM_CORE_CPU_CAPABILITY_UNKNOWN ||
        !nxvm_core_cpu_capability_manifest_set(
            &manifest, NXVM_CORE_CPU_CAPABILITY_REAL_MODE_8086,
            NXVM_CORE_CPU_CAPABILITY_PROVEN) ||
        nxvm_core_cpu_capability_manifest_get(
            &manifest, NXVM_CORE_CPU_CAPABILITY_REAL_MODE_8086) !=
        NXVM_CORE_CPU_CAPABILITY_PROVEN ||
        nxvm_core_cpu_capability_name(
            NXVM_CORE_CPU_CAPABILITY_I386_DECODE) == NULL ||
        nxvm_core_cpu_capability_manifest_set(
            &manifest, NXVM_CORE_CPU_CAPABILITY_COUNT,
            NXVM_CORE_CPU_CAPABILITY_PROVEN)) {
        return 1;
    }

    puts("M5:T1:S1:CPU-CAPABILITY:OK");
    return 0;
}
