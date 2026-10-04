#ifndef NXVM_TEST_MODEL40_H
#define NXVM_TEST_MODEL40_H

#include "app-nxvm/machine/machine_private.h"

/* Test views capture the actual Profile owner; they maintain no live state. */
static inline vm_profile_model40_observation vm_test_model40_observation(
    const vm_machine *machine)
{
    vm_profile_model40_observation observation = {0};

    if (machine != LIB_NULL)
        (void)vm_profile_machine_plan_observe_model40(machine->profile_plan, &observation);
    return observation;
}

static inline lib_status vm_test_model40_d4_observe(const vm_machine *machine,
    core_machine_d4_platform_observation *out_observation)
{
    vm_profile_model40_observation observation;
    lib_status status;

    if (machine == LIB_NULL || out_observation == LIB_NULL) return LIB_STATUS_INVALID_ARGUMENT;
    status = vm_profile_machine_plan_observe_model40(machine->profile_plan, &observation);
    if (status == LIB_STATUS_OK) *out_observation = observation.d4;
    return status;
}

#endif
