#ifndef VM_PROFILE_MODEL40_OBSERVATION_INTERFACE_H
#define VM_PROFILE_MODEL40_OBSERVATION_INTERFACE_H

#include "app-nxvm/profiles/machine_plan_interface.h"
#include "app-nxvm/profiles/model40/d4_platform_interface.h"

typedef struct vm_profile_model40_external_rom vm_profile_model40_external_rom;

typedef struct vm_profile_model40_observation {
    core_machine_d4_platform_observation d4;
    core_machine_fdc_terminal_observation fdc_terminal;
    lib_bool fdc_terminal_valid;
} vm_profile_model40_observation;

const vm_profile_model40_external_rom *vm_profile_machine_plan_model40_rom_get(
    const vm_profile_machine_plan *plan);
lib_u8 vm_profile_machine_plan_is_model40(const vm_profile_machine_plan *plan);
/* Serialized with Core execution, or captured while its executor is paused.
 * The Profile owns observations; Core's attachment owns the borrowed D4.
 * A non-Model40 plan returns UNSUPPORTED without changing the output. */
lib_status vm_profile_machine_plan_observe_model40(
    const vm_profile_machine_plan *plan, vm_profile_model40_observation *out_observation);

#endif
