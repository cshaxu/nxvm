#include "type.h"

#include "vm/composition/session.h"
#include "vm/profile/full_pc_profile.h"

static C_INT verify_image(const C_CHAR *fdd, const C_CHAR *hdd, C_INT boot_hdd)
{
    vm_session_config config = { fdd, hdd, 0, 0u, boot_hdd };
    vm_session_reset_vector vector;
    vm_session *session = STD_NULL;

    if (vm_session_create(&config, &session) != NTVDM64_STATUS_OK ||
        vm_session_get_reset_vector(session, &vector) != NTVDM64_STATUS_OK ||
        vector.cs != 0xf000u || vector.ip != 0xfff0u) {
        vm_session_destroy(session);
        return 1;
    }
    vm_session_destroy(session);
    return 0;
}

C_INT main(C_INT argc, C_CHAR **argv)
{
    const core_product_runtime_profile_descriptor *profile =
        vm_profile_full_pc_profile_descriptor();

    if (argc != 3 || profile == STD_NULL ||
        STD_STRCMP(profile->name, "nxvm.full_pc") != 0 ||
        !profile->permits_disk_boot || !profile->uses_legacy_adapter ||
        verify_image(argv[1], STD_NULL, 0) != 0 ||
        verify_image(STD_NULL, argv[2], 1) != 0) return 1;
    puts("M3:T3:S1:FULL-PC-PROFILE:OK");
    return 0;
}
