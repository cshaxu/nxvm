#include "x86-nxvm/machine/machine_interface.h"
#include "lib/types/types_interface.h"
#include "lib/types/file.h"

#include "x86-nxvm/machine/lifecycle.h"
#include "x86-nxvm/machine/machine_private.h"
#include "../../x86-nxvm/machine/support/emulator_machine_fixture.h"
#include "../../x86-nxvm/machine/support/runner_failure_fixture.h"
#include "../support/rom/session_assets.h"

static lib_i32 vm_runner_error_wait(const x86_nxvm_machine *machine,
    const vm_test_emulator_machine_state_waiter *waiter)
{
    return vm_test_emulator_machine_wait_state(machine, waiter,
        EMULATOR_MACHINE_ERROR, 2000u);
}

static lib_i32 vm_runner_reset_failure_reports_error(void)
{
    x86_nxvm_machine *machine = LIB_NULL;
    vm_test_emulator_machine_state_waiter waiter = {0};
    lib_i32 succeeded = 0;

    if (vm_test_default_pc_at_session_create(LIB_NULL, &machine) !=
            LIB_STATUS_OK || machine == LIB_NULL ||
        vm_test_emulator_machine_bind(machine) != LIB_STATUS_OK ||
        vm_test_emulator_machine_state_waiter_initialize(machine, &waiter) !=
            LIB_STATUS_OK ||
        x86_nxvm_machine_resume(machine) != LIB_STATUS_OK) goto done;
    if (!vm_test_emulator_machine_wait_state(machine, &waiter,
            EMULATOR_MACHINE_RUNNING, 2000u)) goto done;
    if (x86_nxvm_machine_request_pause(machine) != LIB_STATUS_OK ||
        !vm_test_emulator_machine_wait_state(machine, &waiter,
            EMULATOR_MACHINE_PAUSED, 2000u)) goto done;
    /* The adapter receives an existing reset failure, not a rewritten Core. */
    if (vm_test_runner_reject_reset(machine) != LIB_STATUS_OK ||
        x86_nxvm_machine_resume(machine) != LIB_STATUS_OK ||
        !vm_test_emulator_machine_wait_state(machine, &waiter,
            EMULATOR_MACHINE_RUNNING, 2000u)) goto done;
    if (x86_nxvm_machine_control_reset(&machine->control) != LIB_STATUS_OK ||
        !vm_runner_error_wait(machine, &waiter)) goto done;
    succeeded = machine->runner_failed == LIB_TRUE && !vm_test_runner_failure_pending();

done:
    if (machine != LIB_NULL) {
        vm_test_emulator_machine_unbind(machine);
        succeeded = vm_test_runner_reject_reset(LIB_NULL) == LIB_STATUS_OK && succeeded;
        x86_nxvm_machine_destroy(machine);
    }
    vm_test_emulator_machine_state_waiter_finalize(&waiter);
    return succeeded;
}

lib_i32 main(void)
{
    x86_nxvm_machine *machine = LIB_NULL;
    vm_test_emulator_machine_state_waiter waiter = {0};
    lib_i32 succeeded = 0;

    if (vm_test_default_pc_at_session_create(LIB_NULL, &machine) !=
            LIB_STATUS_OK || machine == LIB_NULL ||
        vm_test_emulator_machine_bind(machine) != LIB_STATUS_OK ||
        vm_test_emulator_machine_state_waiter_initialize(machine, &waiter) !=
            LIB_STATUS_OK ||
        x86_nxvm_machine_resume(machine) != LIB_STATUS_OK) goto done;
    if (!vm_test_emulator_machine_wait_state(machine, &waiter,
            EMULATOR_MACHINE_RUNNING, 2000u) ||
        x86_nxvm_machine_request_pause(machine) != LIB_STATUS_OK) goto done;
    if (!vm_test_emulator_machine_wait_state(machine, &waiter,
            EMULATOR_MACHINE_PAUSED, 2000u)) goto done;
    /* Inject the non-fault result at the actual adapter dependency boundary. */
    if (vm_test_runner_reject_run(machine) != LIB_STATUS_OK ||
        x86_nxvm_machine_resume(machine) != LIB_STATUS_OK ||
        !vm_runner_error_wait(machine, &waiter))
        goto done;
    succeeded = machine->runner_failed == LIB_TRUE && !vm_test_runner_failure_pending() &&
        vm_runner_reset_failure_reports_error();

done:
    if (machine != LIB_NULL) {
        vm_test_emulator_machine_unbind(machine);
        succeeded = vm_test_runner_reject_run(LIB_NULL) == LIB_STATUS_OK && succeeded;
        x86_nxvm_machine_destroy(machine);
    }
    vm_test_emulator_machine_state_waiter_finalize(&waiter);
    if (!succeeded) return 1;
    lib_c_printf("RUNNER-ERROR-PROPAGATION:OK\n");
    return 0;
}
