#ifndef TEST_APP_MYDESKPRO386_DIAGNOSTIC_MODEL40_PROFILE_H
#define TEST_APP_MYDESKPRO386_DIAGNOSTIC_MODEL40_PROFILE_H

#include "core/machine/machine_private.h"
#include "app-mydeskpro386/machine/observation_interface.h"
#include "app-mydeskpro386/machine/model40_private.h"

static inline lib_bool model40_profile_is_selected(
    const vm_machine_construction *construction)
{
    return construction != LIB_NULL &&
        construction->firmware_provider == vm_profile_model40_firmware_provider();
}

static inline lib_status model40_profile_d4_observe(const vm_machine *machine,
    core_machine_d4_platform_observation *out_observation)
{
    vm_profile_model40_observation observation;
    lib_status status;

    if (machine == LIB_NULL || out_observation == LIB_NULL)
        return LIB_STATUS_INVALID_ARGUMENT;
    status = vm_profile_model40_observe(&machine->construction,
        &observation);
    if (status == LIB_STATUS_OK) *out_observation = observation.d4;
    return status;
}

#endif
