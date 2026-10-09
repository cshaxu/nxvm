#ifndef VM_PROFILE_MODEL40_OBSERVATION_INTERFACE_H
#define VM_PROFILE_MODEL40_OBSERVATION_INTERFACE_H

#include "core/machine/construction_interface.h"
#include "app-mydeskpro386/profiles/d4_platform_interface.h"

typedef struct vm_profile_model40_observation {
    core_machine_d4_platform_observation d4;
    core_machine_fdc_terminal_observation fdc_terminal;
    lib_bool fdc_terminal_valid;
} vm_profile_model40_observation;

/* Serialized with Core execution, or captured while its executor is paused.
 * The Profile owns observations; Core's attachment owns the borrowed D4.
 * A non-Model40 construction returns UNSUPPORTED without changing the output. */
lib_status vm_profile_model40_observe(
    const vm_machine_construction *construction, vm_profile_model40_observation *out_observation);

#endif
