#include "type.h"

#include "vm/composition/session/session_interface.h"
#include "vm/composition/session/session_private.h"
#include "vm/composition/session/control.h"
#include "vm/composition/session/lifecycle.h"

static C_INT verify(const C_CHAR *fdd, const C_CHAR *hdd, C_INT boot_hdd)
{
    vm_session_config config = {
        .fdd_image = fdd,
        .hdd_image = hdd,
        .boot_hdd = boot_hdd,
        .cpu_profile = CORE_MACHINE_CPU_PROFILE_80386,
        .fpu_profile = CORE_MACHINE_FPU_PROFILE_NONE
    };
    vm_session_reset_vector vector;
    vm_session *session = STD_NULL;

    if (vm_session_create(&config, &session) != TYPE_STATUS_OK ||
        session->core_machine == STD_NULL ||
        vm_session_get_reset_vector(session, &vector) != TYPE_STATUS_OK ||
        vector.cs != 0xf000u || vector.ip != 0xfff0u) {
        vm_session_destroy(session);
        return 1;
    }
    vm_session_destroy(session);
    return 0;
}

static C_INT verify_created(C_VOID)
{
    vm_session_config config = {
        .create_fdd = 1,
        .create_hdd_cylinders = 1u,
        .boot_hdd = 1,
        .cpu_profile = CORE_MACHINE_CPU_PROFILE_80386,
        .fpu_profile = CORE_MACHINE_FPU_PROFILE_NONE
    };
    vm_session *session = STD_NULL;

    if (vm_session_create(&config, &session) != TYPE_STATUS_OK ||
        !session->fdd.connect.flagDiskExist || !session->hdd.connect.flagDiskExist ||
        session->hdd.data.ncyl != 1u) {
        vm_session_destroy(session);
        return 1;
    }
    vm_session_destroy(session);
    return 0;
}

static C_INT verify_selected_cpu_uses_the_resolved_topology(C_VOID)
{
    const vm_session_config config = {
        .cpu_profile = CORE_MACHINE_CPU_PROFILE_80286,
        .fpu_profile = CORE_MACHINE_FPU_PROFILE_80287
    };
    core_machine_cpu_profile profile;
    vm_session *session = STD_NULL;

    if (vm_session_create(&config, &session) != TYPE_STATUS_OK ||
        core_machine_get_cpu_profile(session->core_machine, &profile) != TYPE_STATUS_OK ||
        profile != CORE_MACHINE_CPU_PROFILE_80286) {
        vm_session_destroy(session);
        return 1;
    }
    vm_session_destroy(session);
    return 0;
}

static C_INT verify_initialize_once(C_VOID)
{
    vm_session *session = STD_NULL;
    core_machine *core_machine;
    vm_session_control_state *control;

    if (vm_session_create(STD_NULL, &session) != TYPE_STATUS_OK ||
        session == STD_NULL || !session->active) {
        vm_session_destroy(session);
        return 1;
    }
    core_machine = session->core_machine;
    control = &session->control;
    vm_session_initialize(session);
    if (!session->active || session->core_machine != core_machine ||
        &session->control != control) {
        vm_session_destroy(session);
        return 1;
    }
    vm_session_destroy(session);
    return 0;
}

C_INT main(C_INT argc, C_CHAR **argv)
{
    if (argc != 3 || verify(argv[1], argv[2], 0) != 0 ||
        verify(argv[1], argv[2], 1) != 0 || verify_created() != 0 ||
        verify_selected_cpu_uses_the_resolved_topology() != 0 ||
        verify_initialize_once() != 0) return 1;
    puts("M5:T7:S1:NXVM-SESSION:OK");
    return 0;
}
