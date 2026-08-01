#include "machine/core/contract/cpu_capability.h"

#include <string.h>

static const char *const nxvm_core_cpu_capability_names[] = {
    "x86.real_mode_8086",
    "x86.protected_mode_286",
    "x86.i386_decode",
    "x86.i386_real_mode",
    "x86.i386_protected_mode",
    "x86.i386_paging"
};

void nxvm_core_cpu_capability_manifest_initialize(
    nxvm_core_cpu_capability_manifest *manifest)
{
    if (manifest != NULL) {
        memset(manifest, 0, sizeof(*manifest));
    }
}

int nxvm_core_cpu_capability_manifest_set(
    nxvm_core_cpu_capability_manifest *manifest,
    nxvm_core_cpu_capability capability,
    nxvm_core_cpu_capability_verdict verdict)
{
    if (manifest == NULL || capability < 0 ||
        capability >= NXVM_CORE_CPU_CAPABILITY_COUNT ||
        verdict < NXVM_CORE_CPU_CAPABILITY_UNKNOWN ||
        verdict > NXVM_CORE_CPU_CAPABILITY_FAILED) {
        return 0;
    }
    manifest->verdicts[capability] = verdict;
    return 1;
}

nxvm_core_cpu_capability_verdict nxvm_core_cpu_capability_manifest_get(
    const nxvm_core_cpu_capability_manifest *manifest,
    nxvm_core_cpu_capability capability)
{
    if (manifest == NULL || capability < 0 ||
        capability >= NXVM_CORE_CPU_CAPABILITY_COUNT) {
        return NXVM_CORE_CPU_CAPABILITY_UNKNOWN;
    }
    return manifest->verdicts[capability];
}

const char *nxvm_core_cpu_capability_name(
    nxvm_core_cpu_capability capability)
{
    if (capability < 0 || capability >= NXVM_CORE_CPU_CAPABILITY_COUNT) {
        return NULL;
    }
    return nxvm_core_cpu_capability_names[capability];
}
