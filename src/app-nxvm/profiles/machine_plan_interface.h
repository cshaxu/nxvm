#include "lib/types/types_interface.h"
#ifndef VM_PROFILE_MACHINE_PLAN_INTERFACE_H
#define VM_PROFILE_MACHINE_PLAN_INTERFACE_H

#include "app-nxvm/profiles/selection_interface.h"
#include "app-nxvm/profiles/model40/d4_platform_interface.h"

#include "x86/core/firmware_interface.h"
#include "x86/ibmpc-common/machine_board_interface.h"
#include "x86/ibmpc-common/floppy_interface.h"

typedef struct vm_profile_machine_plan vm_profile_machine_plan;

lib_status vm_profile_machine_plan_describe(vm_profile_machine_plan *plan,
    vm_machine_construction *out_construction);
typedef struct vm_profile_model40_external_rom vm_profile_model40_external_rom;

typedef struct vm_profile_model40_observation {
    core_machine_d4_platform_observation d4;
    core_machine_fdc_terminal_observation fdc_terminal;
    lib_bool fdc_terminal_valid;
} vm_profile_model40_observation;

lib_status vm_profile_machine_plan_create_default(const vm_machine_config *config,
    const vm_machine_assets *assets, vm_profile_machine_plan **out_plan);
lib_status vm_profile_machine_plan_create_5170(const vm_machine_config *config,
    const vm_machine_assets *assets, vm_profile_machine_plan **out_plan);
lib_status vm_profile_machine_plan_create_xt(const vm_machine_config *config,
    const vm_machine_assets *assets, vm_profile_machine_plan **out_plan);
lib_status vm_profile_machine_plan_create_model40(const vm_machine_config *config,
    const vm_machine_assets *assets, vm_profile_machine_plan **out_plan);
void vm_profile_machine_plan_destroy(vm_profile_machine_plan *plan);

const core_machine_config *vm_profile_machine_plan_core_config_get(
    const vm_profile_machine_plan *plan);
const core_machine_controller_timing_rules *
vm_profile_machine_plan_timing_rules_get(const vm_profile_machine_plan *plan);
const core_machine_plan_topology *vm_profile_machine_plan_topology_get(
    const vm_profile_machine_plan *plan);
const core_machine_firmware_provider *vm_profile_machine_plan_firmware_provider_get(
    const vm_profile_machine_plan *plan);
void *vm_profile_machine_plan_firmware_context_get(
    vm_profile_machine_plan *plan);
lib_u8 vm_profile_machine_plan_hdc_present(const vm_profile_machine_plan *plan);
const vm_profile_model40_external_rom *vm_profile_machine_plan_model40_rom_get(
    const vm_profile_machine_plan *plan);
lib_u8 vm_profile_machine_plan_is_model40(const vm_profile_machine_plan *plan);
lib_u8 vm_profile_machine_plan_external_firmware(const vm_profile_machine_plan *plan);
lib_status vm_profile_machine_plan_materialize(vm_profile_machine_plan *plan,
    core_machine_plan *core_plan);
/* Serialized with Core execution, or captured while its executor is paused.
 * The Profile owns observations; Core's attachment owns the borrowed D4.
 * A non-Model40 plan returns UNSUPPORTED without changing the output. */
lib_status vm_profile_machine_plan_observe_model40(
    const vm_profile_machine_plan *plan, vm_profile_model40_observation *out_observation);

#endif
