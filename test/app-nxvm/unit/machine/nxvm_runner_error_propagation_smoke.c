#include "core/machine/machine_interface.h"
#include "lib/types/types_interface.h"
#include <stdio.h>

#include "core/x86/firmware_interface.h"
#include "../../../core/board-base/composition/composition_fixture.h"
#include "core/machine/lifecycle.h"
#include "core/machine/machine_private.h"
#include "../support/ibmpc/machine/support/common_machine_fixture.h"
#include "../support/rom/session_assets.h"

static lib_i32 vm_runner_error_wait(const vm_machine *machine,
    const vm_test_common_machine_state_waiter *waiter)
{
    return vm_test_common_machine_wait_state(machine, waiter,
        COMMON_MACHINE_ERROR, 2000u);
}

static lib_status vm_runner_reset_failure(void *context,
    core_machine_firmware_context *firmware)
{
    (void)context;
    (void)firmware;
    return LIB_STATUS_UNSUPPORTED;
}

static const core_machine_firmware_provider vm_runner_reset_failure_provider = {
    LIB_NULL,
    vm_runner_reset_failure,
    LIB_NULL
};

static lib_i32 vm_runner_reset_failure_reports_error(void)
{
    vm_machine *machine = LIB_NULL;
    vm_test_common_machine_state_waiter waiter = {0};
    lib_i32 succeeded = 0;

    if (vm_test_default_pc_at_session_create(LIB_NULL, &machine) !=
            LIB_STATUS_OK || machine == LIB_NULL ||
        vm_test_common_machine_bind(machine) != LIB_STATUS_OK ||
        vm_test_common_machine_state_waiter_initialize(machine, &waiter) !=
            LIB_STATUS_OK ||
        vm_machine_resume(machine) != LIB_STATUS_OK) goto done;
    if (!vm_test_common_machine_wait_state(machine, &waiter,
            COMMON_MACHINE_RUNNING, 2000u)) goto done;
    if (vm_machine_request_pause(machine) != LIB_STATUS_OK ||
        !vm_test_common_machine_wait_state(machine, &waiter,
            COMMON_MACHINE_PAUSED, 2000u)) goto done;
    /* Inject only while the paused boundary excludes Core execution, then
     * exercise the active reset failure without changing frozen topology. */
    test_core_replace_firmware_provider(machine->core_machine,
        &vm_runner_reset_failure_provider);
    if (vm_machine_resume(machine) != LIB_STATUS_OK ||
        !vm_test_common_machine_wait_state(machine, &waiter,
            COMMON_MACHINE_RUNNING, 2000u)) goto done;
    if (vm_machine_control_reset(&machine->control) != LIB_STATUS_OK ||
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

lib_i32 main(void)
{
    vm_machine *machine = LIB_NULL;
    vm_test_common_machine_state_waiter waiter = {0};
    lib_i32 succeeded = 0;

    if (vm_test_default_pc_at_session_create(LIB_NULL, &machine) !=
            LIB_STATUS_OK || machine == LIB_NULL ||
        vm_test_common_machine_bind(machine) != LIB_STATUS_OK ||
        vm_test_common_machine_state_waiter_initialize(machine, &waiter) !=
            LIB_STATUS_OK ||
        vm_machine_resume(machine) != LIB_STATUS_OK) goto done;
    if (!vm_test_common_machine_wait_state(machine, &waiter,
            COMMON_MACHINE_RUNNING, 2000u) ||
        vm_machine_request_pause(machine) != LIB_STATUS_OK) goto done;
    if (!vm_test_common_machine_wait_state(machine, &waiter,
            COMMON_MACHINE_PAUSED, 2000u)) goto done;
    /* The paused Common boundary owns exclusivity.  This deliberate impossible
     * Core lifecycle forces its non-fault INVALID_STATE result on the next
     * bounded runner turn, proving the driver reports ERROR rather than STOPPED. */
    test_core_set_lifecycle(machine->core_machine, CORE_MACHINE_RUNNING);
    if (vm_machine_resume(machine) != LIB_STATUS_OK ||
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
    printf("M5:T534:S24:RUNNER-ERROR-PROPAGATION:OK\n");
    return 0;
}
