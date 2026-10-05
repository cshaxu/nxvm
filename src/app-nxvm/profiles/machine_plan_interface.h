#include "lib/types/types_interface.h"
#ifndef VM_PROFILE_MACHINE_PLAN_INTERFACE_H
#define VM_PROFILE_MACHINE_PLAN_INTERFACE_H

#include "ibmpc/machine/input_interface.h"

#include "x86/core/firmware_interface.h"
#include "ibmpc/board-common/machine_board_interface.h"
#include "ibmpc/board-common/floppy_interface.h"

typedef struct vm_profile_machine_plan vm_profile_machine_plan;

lib_status vm_profile_machine_plan_describe(vm_profile_machine_plan *plan,
    vm_machine_construction *out_construction);
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
lib_u8 vm_profile_machine_plan_external_firmware(const vm_profile_machine_plan *plan);
lib_status vm_profile_machine_plan_materialize(vm_profile_machine_plan *plan,
    core_machine_plan *core_plan);
#endif
