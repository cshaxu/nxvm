#include "type.h"

#include "core/machine/machine_interface.h"
#include "core/product/session/session_interface.h"
#include "core/product/session/session_provider.h"
#include "vm/composition/session/control.h"
#include "vm/composition/session/lifecycle.h"
#include "vm/composition/session/provider.h"
#include "vm/composition/session/session_interface.h"
#include "tests/support/vm_session_fixture.h"

static C_INT verify_profile(const vm_session *session,
    core_machine_cpu_profile cpu_profile, core_machine_fpu_profile fpu_profile)
{
    core_machine_cpu_profile observed_cpu;
    core_machine_fpu_profile observed_fpu;

    return session == STD_NULL ||
        core_machine_get_cpu_profile(vm_session_fixture_machine(session), &observed_cpu) !=
            TYPE_STATUS_OK ||
        core_machine_get_fpu_profile(vm_session_fixture_machine(session), &observed_fpu) !=
            TYPE_STATUS_OK ||
        observed_cpu != cpu_profile || observed_fpu != fpu_profile;
}

static C_INT verify_open_profile(core_product_session_manager *manager,
    C_CHAR *cpu_name, core_machine_cpu_profile cpu_profile,
    core_product_session_id expected_id)
{
    C_CHAR fpu_none[] = "none";
    C_CHAR option_cpu[] = "--cpu";
    C_CHAR option_fpu[] = "--fpu";
    C_CHAR *arguments[] = { option_cpu, cpu_name, option_fpu, fpu_none };
    const core_product_session_open_options options = { 4, arguments };
    C_VOID *opaque = STD_NULL;
    core_product_session_id id;

    return core_product_session_manager_open_with_options(manager, &options,
            &id) != TYPE_STATUS_OK || id != expected_id ||
        core_product_session_manager_select(manager, id) != TYPE_STATUS_OK ||
        core_product_session_manager_borrow_selected(manager, &opaque) !=
            TYPE_STATUS_OK || verify_profile((vm_session *)opaque,
            cpu_profile, CORE_MACHINE_FPU_PROFILE_NONE);
}

C_INT main(C_VOID)
{
    C_CHAR cpu_8086[] = "8086";
    C_CHAR cpu_80186[] = "80186";
    C_CHAR cpu_80286[] = "80286";
    C_CHAR cpu_80386[] = "80386";
    C_CHAR fpu_8087[] = "8087";
    C_CHAR option_fpu[] = "--fpu";
    C_CHAR *fpu_options[] = { option_fpu, fpu_8087 };
    const core_product_session_open_options select_fpu = {
        2, fpu_options
    };
    core_product_session_provider provider;
    core_product_session_manager *manager = STD_NULL;
    core_product_session_snapshot snapshots[5];
    C_VOID *opaque = STD_NULL;
    vm_session *session;
    core_product_session_id id;
    STD_SIZE_T count;

    vm_session_provider_initialize(&provider);
    if (core_product_session_manager_create(&provider, &manager) != TYPE_STATUS_OK ||
        core_product_session_manager_list(manager, snapshots, 2u, &count) !=
            TYPE_STATUS_OK || count != 1u ||
        STD_STRCMP(snapshots[0].details, "cpu=80386 fpu=none") ||
        verify_open_profile(manager, cpu_8086, CORE_MACHINE_CPU_PROFILE_8086, 1u) ||
        verify_open_profile(manager, cpu_80186, CORE_MACHINE_CPU_PROFILE_80186, 2u) ||
        verify_open_profile(manager, cpu_80286, CORE_MACHINE_CPU_PROFILE_80286, 3u) ||
        verify_open_profile(manager, cpu_80386, CORE_MACHINE_CPU_PROFILE_80386, 4u) ||
        core_product_session_manager_select(manager, 1u) != TYPE_STATUS_OK ||
        core_product_session_manager_borrow_selected(manager, &opaque) !=
            TYPE_STATUS_OK) goto fail;
    session = (vm_session *)opaque;
    if (verify_profile(session, CORE_MACHINE_CPU_PROFILE_8086,
            CORE_MACHINE_FPU_PROFILE_NONE)) goto fail;
    vm_session_reset(session);
    if (verify_profile(session, CORE_MACHINE_CPU_PROFILE_8086,
            CORE_MACHINE_FPU_PROFILE_NONE) ||
        core_product_session_manager_open_with_options(manager, &select_fpu,
            &id) != TYPE_STATUS_INVALID_STATE ||
        core_product_session_manager_get_count(manager, &count) != TYPE_STATUS_OK ||
        count != 5u || core_product_session_manager_list(manager, snapshots,
            5u, &count) != TYPE_STATUS_OK ||
        STD_STRCMP(snapshots[1].details, "cpu=8086 fpu=none")) goto fail;
    core_product_session_manager_destroy(manager);
    STD_PRINTF("M5:T157:S1:SESSION-PROFILES:OK\n");
    return 0;

fail:
    core_product_session_manager_destroy(manager);
    return 1;
}
