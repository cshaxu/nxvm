#include "profile.h"
#include "app-nxvm/profiles/model40/observation_interface.h"
#include "app-nxvm/profiles/machine_factory_interface.h"
#ifndef NXVM_TEST_MODEL40_H
#define NXVM_TEST_MODEL40_H

#include "ibmpc/machine/machine_private.h"

/* Test views capture the actual Profile owner; they maintain no live state. */
static inline vm_profile_model40_observation vm_test_model40_observation(
    const vm_machine *machine)
{
    vm_profile_model40_observation observation = {0};

    if (machine != LIB_NULL)
        (void)vm_profile_machine_plan_observe_model40(vm_test_profile_plan(machine), &observation);
    return observation;
}

static inline lib_status vm_test_model40_d4_observe(const vm_machine *machine,
    core_machine_d4_platform_observation *out_observation)
{
    vm_profile_model40_observation observation;
    lib_status status;

    if (machine == LIB_NULL || out_observation == LIB_NULL) return LIB_STATUS_INVALID_ARGUMENT;
    status = vm_profile_machine_plan_observe_model40(vm_test_profile_plan(machine), &observation);
    if (status == LIB_STATUS_OK) *out_observation = observation.d4;
    return status;
}

#endif
