#include "lib/types/types_interface.h"
#include "type.h"

#include "app-nxvm/devices/firmware_interface.h"
#include "app-nxvm/devices/machine.h"
#include "app-nxvm/machine/lifecycle.h"
#include "app-nxvm/machine/machine_private.h"
#include "support/common_machine_fixture.h"
#include "support/rom/session_assets.h"

static C_INT vm_runner_error_wait(const vm_machine *machine,
    const vm_test_common_machine_state_waiter *waiter)
{
    return vm_test_common_machine_wait_state(machine, waiter,
        COMMON_MACHINE_ERROR, 2000u);
}

static type_status vm_runner_reset_failure(C_VOID *context,
    core_machine_firmware_context *firmware)
{
    (C_VOID)context;
    (C_VOID)firmware;
    return TYPE_STATUS_UNSUPPORTED;
}

static const core_machine_firmware_provider vm_runner_reset_failure_provider = {
    LIB_NULL,
    vm_runner_reset_failure,
    LIB_NULL,
    LIB_NULL
};

static C_INT vm_runner_reset_failure_reports_error(C_VOID)
{
    vm_machine *machine = LIB_NULL;
    vm_test_common_machine_state_waiter waiter = {0};
    C_INT succeeded = 0;

    if (vm_test_default_pc_at_session_create(LIB_NULL, &machine) !=
            TYPE_STATUS_OK || machine == LIB_NULL ||
        vm_test_common_machine_bind(machine) != TYPE_STATUS_OK ||
        vm_test_common_machine_state_waiter_initialize(machine, &waiter) !=
            TYPE_STATUS_OK ||
        vm_machine_resume(machine) != TYPE_STATUS_OK) goto done;
    if (!vm_test_common_machine_wait_state(machine, &waiter,
            COMMON_MACHINE_RUNNING, 2000u)) goto done;
    /* The profile provider has already completed its cold reset.  Replacing
     * only its reset callback makes the active direct-control reset fail at
     * the runner boundary without changing its frozen configure topology. */
    machine->core_machine->firmware_provider =
        &vm_runner_reset_failure_provider;
    if (vm_machine_control_reset(&machine->control) != TYPE_STATUS_OK ||
        !vm_runner_error_wait(machine, &waiter)) goto done;
    succeeded = machine->runner_failed == LIB_TRUE;

done:
    if (machine != LIB_NULL) {
        vm_test_common_machine_unbind(machine);
        vm_machine_destroy(machine);
    }
    vm_test_common_machine_state_waiter_finalize(&waiter);
    return succeeded;
}

C_INT main(C_VOID)
{
    vm_machine *machine = LIB_NULL;
    vm_test_common_machine_state_waiter waiter = {0};
    C_INT succeeded = 0;

    if (vm_test_default_pc_at_session_create(LIB_NULL, &machine) !=
            TYPE_STATUS_OK || machine == LIB_NULL ||
        vm_test_common_machine_bind(machine) != TYPE_STATUS_OK ||
        vm_test_common_machine_state_waiter_initialize(machine, &waiter) !=
            TYPE_STATUS_OK ||
        vm_machine_resume(machine) != TYPE_STATUS_OK) goto done;
    if (!vm_test_common_machine_wait_state(machine, &waiter,
            COMMON_MACHINE_RUNNING, 2000u) ||
        vm_machine_request_pause(machine) != TYPE_STATUS_OK) goto done;
    if (!vm_test_common_machine_wait_state(machine, &waiter,
            COMMON_MACHINE_PAUSED, 2000u)) goto done;
    /* The paused Common boundary owns exclusivity.  This deliberate impossible
     * Core lifecycle forces its non-fault INVALID_STATE result on the next
     * bounded runner turn, proving the driver reports ERROR rather than STOPPED. */
    machine->core_machine->lifecycle = CORE_MACHINE_RUNNING;
    if (vm_machine_resume(machine) != TYPE_STATUS_OK ||
        !vm_runner_error_wait(machine, &waiter))
        goto done;
    succeeded = machine->runner_failed == LIB_TRUE &&
        vm_runner_reset_failure_reports_error();

done:
    if (machine != LIB_NULL) {
        vm_test_common_machine_unbind(machine);
        vm_machine_destroy(machine);
    }
    vm_test_common_machine_state_waiter_finalize(&waiter);
    if (!succeeded) return 1;
    STD_PRINTF("M5:T534:S24:RUNNER-ERROR-PROPAGATION:OK\n");
    return 0;
}
