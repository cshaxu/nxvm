#include "core/machine/machine_interface.h"
#include "lib/types/types_interface.h"
#include "../../../core/board-base/composition/composition_fixture.h"
#include <stdio.h>

#include "core/machine/lifecycle.h"
#include "core/machine/machine_private.h"
#include "core/machine/machine_interface.h"
#include "app-nxvm/profiles/profile_interface.h"

#include "core/x86/debug_interface.h"
#include "../../support/rom/session_assets.h"

static lib_i32 vm_pcat_composition_reset_state_matches(vm_machine *session)
{
    core_machine_timeline_observation timeline;
    vm_machine_reset_vector vector;
    lib_i32 nmi_masked = LIB_TRUE;

    return session == LIB_NULL || session->core_machine == LIB_NULL ||
        !session->active ||
        vm_machine_get_reset_vector(session, &vector) != LIB_STATUS_OK ||
        vector.cs != 0xf000u || vector.ip != 0xfff0u ||
        core_machine_get_timeline_observation(session->core_machine,
            &timeline) != LIB_STATUS_OK ||
        timeline.now != 0u || timeline.pending_events != 0u ||
        timeline.next_sequence != 0u ||
        core_machine_get_nmi_mask(session->core_machine, &nmi_masked) !=
            LIB_STATUS_OK || nmi_masked;
}

static lib_i32 vm_pcat_composition_reset_rearms_selected_machine(
    vm_machine *session)
{
    static const lib_u8 nop = 0x90u;
    const core_machine_debug_register_patch entry = {
        .mask = CORE_MACHINE_DEBUG_REGISTER_MASK(CORE_MACHINE_DEBUG_CS) |
            CORE_MACHINE_DEBUG_REGISTER_MASK(CORE_MACHINE_DEBUG_DS) |
            CORE_MACHINE_DEBUG_REGISTER_MASK(CORE_MACHINE_DEBUG_ES) |
            CORE_MACHINE_DEBUG_REGISTER_MASK(CORE_MACHINE_DEBUG_SS) |
            CORE_MACHINE_DEBUG_REGISTER_MASK(CORE_MACHINE_DEBUG_EIP),
        .values = { [CORE_MACHINE_DEBUG_EIP] = 0x1000u }
    };
    core_machine_run_budget budget = { 1u, 0u };
    core_machine_run_result result;
    core_machine_timeline_observation timeline;
    lib_i32 nmi_masked = LIB_FALSE;

    if (session == LIB_NULL || session->core_machine == LIB_NULL ||
        core_machine_debug_patch_registers(session->core_machine, &entry) !=
            LIB_STATUS_OK ||
        core_machine_memory_write(session->core_machine, 0x1000u, &nop,
            sizeof(nop)) != LIB_STATUS_OK ||
        core_machine_run(session->core_machine, budget, &result) != LIB_STATUS_OK ||
        result.reason != CORE_MACHINE_STOP_BUDGET || result.elapsed_ticks == 0u ||
        core_machine_get_timeline_observation(session->core_machine,
            &timeline) != LIB_STATUS_OK || timeline.now == 0u) {
        return 1;
    }
    test_core_write_port_after_run(session->core_machine, 0x0070u, 0x80u);
    if (core_machine_get_nmi_mask(session->core_machine, &nmi_masked) !=
            LIB_STATUS_OK || !nmi_masked) return 1;
    vm_machine_reset(session);
    return vm_pcat_composition_reset_state_matches(session);
}

lib_i32 main(void)
{
    const vm_profile_default_pc_at_descriptor *profile =
        vm_profile_default_pc_at_descriptor_get();
    vm_machine *session = LIB_NULL;
    lib_i32 failed;

    if (profile == LIB_NULL ||
        !vm_profile_default_pc_at_descriptor_is_valid(profile) ||
        vm_test_default_pc_at_session_create(LIB_NULL, &session) != LIB_STATUS_OK ||
        session == LIB_NULL) {
        vm_machine_destroy(session);
        return 1;
    }
    failed = vm_pcat_composition_reset_state_matches(session) != 0 ||
        vm_pcat_composition_reset_rearms_selected_machine(session) != 0;
    vm_machine_destroy(session);
    if (failed) return 1;
    printf("PCAT-COMPOSITION:OK\n");
    return 0;
}
