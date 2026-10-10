#include "core/machine/machine_interface.h"
#include "lib/types/types_interface.h"
#include <stdio.h>
#include "core/x86/device_support_interface.h"

#include "core/x86/debug_interface.h"
#include "core/x86/machine_interface.h"
#include "core/x86/memory_interface.h"
#include "core/machine/control.h"
#include "core/machine/fault.h"
#include "core/machine/lifecycle.h"
#include "core/machine/machine_interface.h"
#include "../../support/rom/session_assets.h"
#include "core/machine/machine_private.h"

static lib_i32 vm_fault_outcome_prepare(vm_machine *session)
{
    /* REAL_UD_TERMINAL_GUEST_LIDT: LIDT [0100h] makes vector 6
       unavailable before the invalid opcode, without private CPU mutation. */
    const lib_u8 program[] = { 0x0fu, 0x01u, 0x1eu, 0x00u, 0x01u, 0xd6u };
    const lib_u8 idtr[] = { 0x17u, 0u, 0u, 0u, 0u, 0u };
    const core_machine_debug_register_patch entry = {
        .mask = CORE_MACHINE_DEBUG_REGISTER_MASK(CORE_MACHINE_DEBUG_CS) |
            CORE_MACHINE_DEBUG_REGISTER_MASK(CORE_MACHINE_DEBUG_DS) |
            CORE_MACHINE_DEBUG_REGISTER_MASK(CORE_MACHINE_DEBUG_ES) |
            CORE_MACHINE_DEBUG_REGISTER_MASK(CORE_MACHINE_DEBUG_SS) |
            CORE_MACHINE_DEBUG_REGISTER_MASK(CORE_MACHINE_DEBUG_EIP)
    };

    if (session == LIB_NULL || session->core_machine == LIB_NULL) return 0;
    return core_machine_debug_patch_registers(session->core_machine, &entry) ==
        LIB_STATUS_OK && core_machine_memory_write(session->core_machine, 0u,
        program, sizeof(program)) == LIB_STATUS_OK &&
        core_machine_memory_write(session->core_machine, 0x0100u, idtr,
            sizeof(idtr)) == LIB_STATUS_OK;
}

lib_i32 main(void)
{
    vm_machine *session = LIB_NULL;
    vm_machine_fault_outcome outcome;
    core_machine_run_budget budget = { 1u, 0u };
    core_machine_run_result run;
    core_machine_cpu_diagnostic diagnostic;
    core_machine_lifecycle lifecycle;

    if (vm_test_default_pc_at_session_create(LIB_NULL, &session) != LIB_STATUS_OK ||
        !vm_fault_outcome_prepare(session)) goto fail;
    vm_machine_control_start(&session->control);
    if (vm_machine_control_is_running(&session->control) ||
        vm_machine_fault_get(session, &outcome) != 0 || outcome.valid ||
        core_machine_get_lifecycle(session->core_machine, &lifecycle) !=
        LIB_STATUS_OK || lifecycle != CORE_MACHINE_PAUSED ||
        core_machine_run(session->core_machine, budget, &run) != LIB_STATUS_OK ||
        run.reason != CORE_MACHINE_STOP_WAITING_FOR_INTERRUPT ||
        run.detail != VCPUINS_EXCEPT_SHUTDOWN) goto fail;
    if (vm_machine_reset(session) != LIB_STATUS_OK ||
        vm_machine_fault_get(session, &outcome) != 0 || outcome.valid ||
        core_machine_get_lifecycle(session->core_machine, &lifecycle) !=
        LIB_STATUS_OK || lifecycle != CORE_MACHINE_STOPPED ||
        core_machine_get_cpu_diagnostic(session->core_machine, &diagnostic) !=
        LIB_STATUS_OK || diagnostic.first_fault.valid) goto fail;
    vm_machine_destroy(session);
    printf("FAULT-OUTCOME:OK\n");
    return 0;

fail:
    vm_machine_destroy(session);
    return 1;
}
