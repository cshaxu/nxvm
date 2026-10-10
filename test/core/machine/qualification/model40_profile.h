#ifndef TEST_CORE_MACHINE_QUALIFICATION_MODEL40_PROFILE_H
#define TEST_CORE_MACHINE_QUALIFICATION_MODEL40_PROFILE_H

#include "../support/profile_qualification.h"
#include "app-mydeskpro386/profiles/observation_interface.h"
#include "app-mydeskpro386/profiles/model40_private.h"

static inline lib_bool pc_qualification_profile_is_model40(
    const vm_machine_construction *construction)
{
    return construction != LIB_NULL &&
        construction->firmware_provider == vm_profile_model40_firmware_provider();
}

/* This view is qualification-only: it observes a selected Model 40 without
 * making an App test depend on MyDeskPro386 test support. */
static inline vm_profile_model40_observation pc_qualification_model40_observation(
    const vm_machine *machine)
{
    vm_profile_model40_observation observation = {0};

    if (machine != LIB_NULL)
        (void)vm_profile_model40_observe(pc_qualification_profile_construction(machine),
            &observation);
    return observation;
}

static inline lib_status pc_qualification_model40_d4_observe(const vm_machine *machine,
    core_machine_d4_platform_observation *out_observation)
{
    vm_profile_model40_observation observation;
    lib_status status;

    if (machine == LIB_NULL || out_observation == LIB_NULL)
        return LIB_STATUS_INVALID_ARGUMENT;
    status = vm_profile_model40_observe(pc_qualification_profile_construction(machine),
        &observation);
    if (status == LIB_STATUS_OK) *out_observation = observation.d4;
    return status;
}

#endif
