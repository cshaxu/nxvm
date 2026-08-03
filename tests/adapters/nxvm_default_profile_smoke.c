#include "type.h"

#include "vm/composition/session.h"
#include "vm/profile/default_profile/profile.h"

C_INT main(C_INT argc, C_CHAR **argv)
{
    core_product_runtime_registry registry;
    vm_session_config config = { argv[1], STD_NULL, 0, 0u, 0 };
    vm_session_reset_vector vector;
    vm_session *session = STD_NULL;

    if (argc != 2) return 1;
    core_product_runtime_registry_initialize(&registry);
    if (vm_profile_register_default_profile_builtin(&registry) != NTVDM64_STATUS_OK ||
        core_product_runtime_registry_find_profile(&registry, VM_PROFILE_PC_AT_PROFILE_ID,
            NXVM_RUNTIME_PROFILE_MACHINE, STD_NULL, STD_NULL) == STD_NULL ||
        core_product_runtime_registry_find_firmware_provider(&registry,
            VM_PROFILE_PC_AT_PROVIDER_ID, VM_PROFILE_PC_AT_PROFILE_ID) == STD_NULL ||
        core_product_runtime_registry_freeze(&registry) != NTVDM64_STATUS_OK ||
        vm_session_create(&config, &session) != NTVDM64_STATUS_OK ||
        vm_session_get_reset_vector(session, &vector) != NTVDM64_STATUS_OK ||
        vector.cs != 0xf000u || vector.ip != 0xfff0u) {
        vm_session_destroy(session);
        return 1;
    }
    vm_session_destroy(session);
    puts("M5:T2:S2:PC-AT-BUILTIN:OK");
    return 0;
}
