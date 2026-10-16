#ifndef TEST_CORE_MACHINE_RUNNER_FAILURE_FIXTURE_H
#define TEST_CORE_MACHINE_RUNNER_FAILURE_FIXTURE_H
#include "core/machine/machine_interface.h"

/* Arm only at the paused boundary; passing null clears the one-shot failure. */
lib_status vm_test_runner_reject_run(const vm_machine *machine);
lib_status vm_test_runner_reject_reset(const vm_machine *machine);
lib_bool vm_test_runner_failure_pending(void);
#endif
