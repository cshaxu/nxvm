#include "profile.h"
#include "app-mydeskpro386/machine_observation_interface.h"
#include "core/machine/machine_interface.h"
#ifndef NXVM_TEST_MODEL40_H
#define NXVM_TEST_MODEL40_H

#include "core/machine/machine_private.h"

/* Test views capture the actual Profile owner; they maintain no live state. */
static inline mydeskpro386_observation vm_test_model40_observation(
    const vm_machine *machine)
{
    mydeskpro386_observation observation = {0};

    if (machine != LIB_NULL)
        (void)mydeskpro386_observe(vm_test_profile_construction(machine), &observation);
    return observation;
}

static inline lib_status vm_test_model40_d4_observe(const vm_machine *machine,
    mydeskpro386_d4_platform_observation *out_observation)
{
    mydeskpro386_observation observation;
    lib_status status;

    if (machine == LIB_NULL || out_observation == LIB_NULL) return LIB_STATUS_INVALID_ARGUMENT;
    status = mydeskpro386_observe(vm_test_profile_construction(machine), &observation);
    if (status == LIB_STATUS_OK) *out_observation = observation.d4;
    return status;
}

#endif
