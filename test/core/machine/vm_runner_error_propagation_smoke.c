#include "type.h"

#include "lib/base/sync_interface.h"
#include "core/devices/machine.h"
#include "core/machine/lifecycle.h"
#include "core/machine/machine_private.h"
#include "support/common_machine_fixture.h"
#include "support/rom/session_assets.h"

static C_INT vm_runner_error_wait(const vm_machine *machine)
{
    C_UINT waited;

    for (waited = 0u; waited < 2000u; ++waited) {
        if (common_machine_state_get(machine->executor) == COMMON_MACHINE_ERROR)
            return TYPE_TRUE;
        base_sync_sleep_milliseconds(1u);
    }
    return TYPE_FALSE;
}

C_INT main(C_VOID)
{
    vm_machine *machine = STD_NULL;
    C_INT succeeded = 0;

    if (vm_test_default_pc_at_session_create(STD_NULL, &machine) !=
            TYPE_STATUS_OK || machine == STD_NULL ||
        vm_test_common_machine_bind(machine) != TYPE_STATUS_OK ||
        vm_machine_resume(machine) != TYPE_STATUS_OK) goto done;
    for (C_UINT waited = 0u; waited < 2000u; ++waited) {
        if (common_machine_state_get(machine->executor) == COMMON_MACHINE_RUNNING)
            break;
        base_sync_sleep_milliseconds(1u);
    }
    if (common_machine_state_get(machine->executor) != COMMON_MACHINE_RUNNING ||
        vm_machine_request_pause(machine) != TYPE_STATUS_OK) goto done;
    for (C_UINT waited = 0u; waited < 2000u; ++waited) {
        if (common_machine_state_get(machine->executor) == COMMON_MACHINE_PAUSED)
            break;
        base_sync_sleep_milliseconds(1u);
    }
    if (common_machine_state_get(machine->executor) != COMMON_MACHINE_PAUSED)
        goto done;
    /* The paused Common boundary owns exclusivity.  This deliberate impossible
     * Core lifecycle forces its non-fault INVALID_STATE result on the next
     * bounded runner turn, proving the driver reports ERROR rather than STOPPED. */
    machine->core_machine->lifecycle = CORE_MACHINE_RUNNING;
    if (vm_machine_resume(machine) != TYPE_STATUS_OK || !vm_runner_error_wait(machine))
        goto done;
    succeeded = machine->runner_failed == TYPE_TRUE;

done:
    if (machine != STD_NULL) {
        vm_test_common_machine_unbind(machine);
        vm_machine_destroy(machine);
    }
    if (!succeeded) return 1;
    STD_PRINTF("M5:T534:S20:RUNNER-ERROR-PROPAGATION:OK\n");
    return 0;
}
