#ifndef TEST_APP_MYDESKPRO386_DIAGNOSTIC_MODEL40_PROFILE_H
#define TEST_APP_MYDESKPRO386_DIAGNOSTIC_MODEL40_PROFILE_H

#include "x86-nxvm/machine/machine_private.h"
#include "app-mydeskpro386/machine_observation_interface.h"
#include "app-mydeskpro386/machine_config.h"

static inline lib_bool model40_profile_is_selected(
    const x86_nxvm_machine_construction *construction)
{
    return construction != LIB_NULL &&
        construction->firmware_provider == mydeskpro386_firmware_provider();
}

static inline lib_status model40_profile_d4_observe(const x86_nxvm_machine *machine,
    mydeskpro386_d4_platform_observation *out_observation)
{
    mydeskpro386_observation observation;
    lib_status status;

    if (machine == LIB_NULL || out_observation == LIB_NULL)
        return LIB_STATUS_INVALID_ARGUMENT;
    status = mydeskpro386_observe(&machine->construction,
        &observation);
    if (status == LIB_STATUS_OK) *out_observation = observation.d4;
    return status;
}

#endif
