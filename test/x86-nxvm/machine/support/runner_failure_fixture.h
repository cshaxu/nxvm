#ifndef TEST_X86_NXVM_MACHINE_RUNNER_FAILURE_FIXTURE_H
#define TEST_X86_NXVM_MACHINE_RUNNER_FAILURE_FIXTURE_H
#include "x86-nxvm/machine/machine_interface.h"

/* Arm only at the paused boundary; passing null clears the one-shot failure. */
lib_status vm_test_runner_reject_run(const x86_nxvm_machine *machine);
lib_status vm_test_runner_reject_reset(const x86_nxvm_machine *machine);
lib_bool vm_test_runner_failure_pending(void);
#endif
