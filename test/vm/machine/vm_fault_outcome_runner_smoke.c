#include "type.h"

#include "core/machine/cpu_instructions.h"
#include "core/machine/debug_interface.h"
#include "core/machine/machine_interface.h"
#include "core/machine/memory.h"
#include "core/product/debug/debug_access.h"
#include "vm/composition/session/control.h"
#include "vm/composition/session/debug_target.h"
#include "vm/composition/session/fault.h"
#include "vm/composition/session/lifecycle.h"
#include "vm/composition/session/session_interface.h"
#include "../support/rom/session_assets.h"
#include "vm/composition/session/session_private.h"
#include "../../core/support/core_machine_cpu_fixture.h"

static C_INT vm_fault_outcome_prepare(vm_session *session)
{
    const type_unsigned_8 program[] = { 0xd6u };

    if (session == STD_NULL || session->core_machine == STD_NULL) return 0;
    return test_core_machine_fixture_prepare_real_mode_execution(
            session->core_machine, 0u) && core_machine_memory_write(session->core_machine, 0u, program,
        sizeof(program)) == TYPE_STATUS_OK &&
        test_core_machine_fixture_preflight_real_ud_terminal(session->core_machine);
}

C_INT main(C_VOID)
{
    vm_session *session = STD_NULL;
    vm_session_fault_outcome outcome;
    core_product_debug_fault_outcome debug_outcome;
    core_machine_run_budget budget = { 1u, 0u };
    core_machine_run_result run;
    core_machine_cpu_diagnostic diagnostic;
    core_machine_lifecycle lifecycle;
    const core_product_debug_target *target;
    C_INT failed = 0;

    if (vm_test_default_pc_at_session_create(STD_NULL, &session) != TYPE_STATUS_OK ||
        !vm_fault_outcome_prepare(session)) goto fail;
    vm_session_control_start(&session->control);
    target = vm_session_debug_target(session);
    failed |= vm_session_control_is_running(&session->control);
    failed |= vm_session_fault_get(session, &outcome) != 0 || !outcome.valid ||
        outcome.run.reason != CORE_MACHINE_STOP_FAULT ||
        outcome.run.detail != VCPUINS_EXCEPT_UD ||
        !outcome.diagnostic.first_fault.valid ||
        !TYPE_GET_BIT(outcome.diagnostic.first_fault.exception_mask,
            VCPUINS_EXCEPT_UD);
    failed |= !core_product_debug_get_fault_outcome(target, &debug_outcome) ||
        !debug_outcome.valid || debug_outcome.detail != VCPUINS_EXCEPT_UD ||
        !debug_outcome.diagnostic_valid ||
        !TYPE_GET_BIT(debug_outcome.exception_mask, VCPUINS_EXCEPT_UD);
    failed |= core_machine_get_lifecycle(session->core_machine, &lifecycle) !=
        TYPE_STATUS_OK || lifecycle != CORE_MACHINE_FAULTED;
    failed |= core_machine_run(session->core_machine, budget, &run) !=
        TYPE_STATUS_FAULT || run.reason != CORE_MACHINE_STOP_FAULT ||
        run.detail != VCPUINS_EXCEPT_UD;
    vm_session_reset(session);
    failed |= vm_session_fault_get(session, &outcome) != 0 || outcome.valid;
    failed |= core_machine_get_lifecycle(session->core_machine, &lifecycle) !=
        TYPE_STATUS_OK || lifecycle != CORE_MACHINE_STOPPED;
    failed |= core_machine_get_cpu_diagnostic(session->core_machine, &diagnostic) !=
        TYPE_STATUS_OK || diagnostic.first_fault.valid;
    if (failed) goto fail;
    vm_session_destroy(session);
    STD_PRINTF("M5:T214:S3:FAULT-OUTCOME:OK\n");
    return 0;

fail:
    vm_session_destroy(session);
    return 1;
}
