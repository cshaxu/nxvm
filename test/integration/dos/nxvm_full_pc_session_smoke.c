#include "type.h"

#include "vm/composition/session/session_interface.h"

C_INT main(C_INT argc, C_CHAR **argv)
{
    vm_session_config config = {
        .floppy_image = { argv[1] },
        .fixed_disk_image = { argv[2] },
        .cpu_profile = CORE_MACHINE_CPU_PROFILE_80386,
        .fpu_profile = CORE_MACHINE_FPU_PROFILE_NONE
    };
    vm_session_reset_vector vector;
    vm_session *session = STD_NULL;

    if (argc != 3 || vm_session_create(&config, &session) != TYPE_STATUS_OK ||
        vm_session_get_reset_vector(session, &vector) != TYPE_STATUS_OK ||
        vector.cs != 0xf000u || vector.ip != 0xfff0u) {
        vm_session_destroy(session);
        return 1;
    }
    vm_session_destroy(session);
    puts("M5:T13:S8:VM-SESSION:OK");
    return 0;
}
