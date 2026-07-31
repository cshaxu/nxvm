#ifndef NXVM_PRODUCT_NXVM_PROFILE_H
#define NXVM_PRODUCT_NXVM_PROFILE_H

#include "runtime/registry.h"

#define NXVM_PRODUCT_NXVM_PC_AT_PROFILE_ID "nxvm.machine.pc_at_builtin"
#define NXVM_PRODUCT_NXVM_PC_AT_PROVIDER_ID "firmware.provider.pc_at_builtin"

nxvm_core_status nxvm_product_nxvm_register_pc_at_builtin(
    nxvm_runtime_registry *registry);

#endif
