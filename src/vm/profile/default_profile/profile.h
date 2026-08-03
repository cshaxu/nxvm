#ifndef VM_PROFILE_DEFAULT_PROFILE_H
#define VM_PROFILE_DEFAULT_PROFILE_H

#include "core/product/runtime/registry.h"

#define VM_PROFILE_PC_AT_PROFILE_ID "nxvm.machine.default_profile_builtin"
#define VM_PROFILE_PC_AT_PROVIDER_ID "firmware.provider.default_profile_builtin"

ntvdm64_status vm_profile_register_default_profile_builtin(
    core_product_runtime_registry *registry);

#endif
