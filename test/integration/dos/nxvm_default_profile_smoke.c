#include "type.h"

#include "core/machine/memory.h"
#include "vm/composition/session/session_interface.h"
#include "vm/composition/session/session_private.h"
#include "vm/composition/session/lifecycle.h"
#include "vm/machine/fdd.h"
#include "vm/machine/hdd.h"
#include "vm/platform/platform.h"

static C_INT verify(const C_CHAR *fdd, const C_CHAR *hdd, C_INT boot_hdd)
{
    vm_session_config config = {
        .memory_bytes = 16384u * 1024u,
        .floppy_image = { fdd },
        .fixed_disk_image = { hdd },
        .boot_hdd = boot_hdd,
        .cpu_profile = CORE_MACHINE_CPU_PROFILE_80386,
        .fpu_profile = CORE_MACHINE_FPU_PROFILE_NONE
    };
    vm_session_reset_vector vector;
    vm_session *session = STD_NULL;

    if (vm_session_create(&config, &session) != TYPE_STATUS_OK ||
        vm_session_control_is_running(&session->control)) {
        vm_session_destroy(session);
        return 1;
    }
    vm_session_reset(session);
    if (vm_machine_fdd_remove_for(&session->fdd, STD_NULL) ||
        vm_machine_hdd_remove(&session->hdd, STD_NULL) ||
        vm_session_get_reset_vector(session, &vector) != TYPE_STATUS_OK ||
        vector.cs != 0xf000u || vector.ip != 0xfff0u) {
        vm_session_destroy(session);
        return 1;
    }
    vm_platform_run_context_set_window_display(session->platform_run_context, 0);
    vm_session_destroy(session);
    return 0;
}

C_INT main(C_INT argc, C_CHAR **argv)
{
    if (argc != 3 || verify(argv[1], argv[2], 0) != 0 ||
        verify(argv[1], argv[2], 1) != 0) return 1;
    puts("M5:T5:S2:NXVM-PC-AT:OK");
    return 0;
}
