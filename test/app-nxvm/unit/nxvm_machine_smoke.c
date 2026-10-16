#include "x86-nxvm/machine/machine_interface.h"
#include "lib/types/types_interface.h"
#include <stdio.h>

#include "x86-nxvm/machine/machine_interface.h"
#include "x86-nxvm/machine/machine_private.h"
#include "x86-nxvm/machine/control.h"
#include "x86-nxvm/machine/lifecycle.h"
#include "../support/rom/session_assets.h"

static lib_i32 verify(void)
{
    x86_nxvm_machine_config config = {
        .create_fdd = 1,
        .create_hdd_cylinders = 1u,
        .cpu_profile = X86_CORE_CPU_PROFILE_80386,
        .fpu_profile = X86_FPU_PROFILE_NONE
    };
    x86_nxvm_machine_reset_vector vector;
    x86_nxvm_machine *session = LIB_NULL;

    if (vm_test_default_pc_at_session_create(&config, &session) != LIB_STATUS_OK ||
        session->x86_core_machine == LIB_NULL ||
        x86_nxvm_machine_get_reset_vector(session, &vector) != LIB_STATUS_OK ||
        vector.cs != 0xf000u || vector.ip != 0xfff0u) {
        x86_nxvm_machine_destroy(session);
        return 1;
    }
    x86_nxvm_machine_destroy(session);
    return 0;
}

static lib_i32 verify_created(void)
{
    x86_nxvm_machine_config config = {
        .create_fdd = 1,
        .create_hdd_cylinders = 1u,
        .cpu_profile = X86_CORE_CPU_PROFILE_80386,
        .fpu_profile = X86_FPU_PROFILE_NONE
    };
    x86_nxvm_machine_information information;
    x86_nxvm_machine *session = LIB_NULL;

    if (vm_test_default_pc_at_session_create(&config, &session) != LIB_STATUS_OK ||
        x86_nxvm_machine_get_information(session, &information) != LIB_STATUS_OK ||
        !information.floppy_media_inserted || !information.fixed_disk_media_connected ||
        information.fixed_disk_cylinders != 1u || !information.fixed_disk_present ||
        information.cpu_profile != X86_CORE_CPU_PROFILE_80386 ||
        information.active || information.fault_valid) {
        x86_nxvm_machine_destroy(session);
        return 1;
    }
    x86_nxvm_machine_destroy(session);
    return 0;
}

static lib_i32 verify_selected_cpu_uses_the_resolved_topology(void)
{
    const x86_nxvm_machine_config config = {
        .cpu_profile = X86_CORE_CPU_PROFILE_80286,
        .fpu_profile = X86_FPU_PROFILE_80287
    };
    x86_core_cpu_profile profile;
    x86_nxvm_machine *session = LIB_NULL;

    if (vm_test_default_pc_at_session_create(&config, &session) != LIB_STATUS_OK ||
        x86_core_machine_get_cpu_profile(session->x86_core_machine, &profile) != LIB_STATUS_OK ||
        profile != X86_CORE_CPU_PROFILE_80286) {
        x86_nxvm_machine_destroy(session);
        return 1;
    }
    x86_nxvm_machine_destroy(session);
    return 0;
}

static lib_i32 verify_initialize_once(void)
{
    x86_nxvm_machine *session = LIB_NULL;
    x86_core_machine *x86_core_machine;
    x86_nxvm_machine_control_state *control;

    if (vm_test_default_pc_at_session_create(LIB_NULL, &session) != LIB_STATUS_OK ||
        session == LIB_NULL || !session->active) {
        x86_nxvm_machine_destroy(session);
        return 1;
    }
    x86_core_machine = session->x86_core_machine;
    control = &session->control;
    x86_nxvm_machine_initialize(session);
    if (!session->active || session->x86_core_machine != x86_core_machine ||
        &session->control != control) {
        x86_nxvm_machine_destroy(session);
        return 1;
    }
    x86_nxvm_machine_destroy(session);
    return 0;
}

static lib_i32 verify_rejects_unattached_media_slots(void)
{
    x86_nxvm_machine_config second_disk = { .fixed_disk_image = { LIB_NULL, "disk.img" } };
    x86_nxvm_machine_config second_floppy = { .floppy_image = { LIB_NULL, "disk.img" } };
    x86_nxvm_machine *session = LIB_NULL;

    if (vm_test_default_pc_at_session_create(&second_disk, &session) != LIB_STATUS_INVALID_ARGUMENT ||
        session != LIB_NULL || vm_test_default_pc_at_session_create(&second_floppy, &session) !=
        LIB_STATUS_INVALID_ARGUMENT || session != LIB_NULL) return 1;
    return 0;
}

lib_i32 main(lib_i32 argc, char **argv)
{
    (void)argc;
    (void)argv;
    if (verify() != 0 || verify_created() != 0 ||
        verify_selected_cpu_uses_the_resolved_topology() != 0 ||
        verify_initialize_once() != 0 || verify_rejects_unattached_media_slots() != 0)
        return 1;
    puts("NXVM-SESSION:OK");
    return 0;
}
