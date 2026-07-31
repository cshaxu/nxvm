#ifndef NXVM_BASELINE_PC_AT_BUILTIN_PROFILE_H
#define NXVM_BASELINE_PC_AT_BUILTIN_PROFILE_H

#include "core/status.h"
#include "runtime/registry.h"

#ifdef __cplusplus
extern "C" {
#endif

#define NXVM_BASELINE_PC_AT_BUILTIN_PROFILE_ID "nxvm.machine.pc_at_builtin"
#define NXVM_BASELINE_PC_AT_BUILTIN_PROVIDER_ID "firmware.provider.pc_at_builtin"

nxvm_core_status nxvm_baseline_pc_at_builtin_register(
    nxvm_runtime_registry *registry);

#ifdef __cplusplus
}
#endif

#endif
