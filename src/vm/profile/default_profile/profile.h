#ifndef NXVM_PRODUCT_NXVM_PROFILE_H
#define NXVM_PRODUCT_NXVM_PROFILE_H

#include "core/product/runtime/registry.h"

#define NXVM_PRODUCT_NXVM_PC_AT_PROFILE_ID "nxvm.machine.default_profile_builtin"
#define NXVM_PRODUCT_NXVM_PC_AT_PROVIDER_ID "firmware.provider.default_profile_builtin"

ntvdm64_status nxvm_product_nxvm_register_default_profile_builtin(
    nxvm_runtime_registry *registry);

#endif
