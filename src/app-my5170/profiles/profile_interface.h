#ifndef MY5170_PROFILE_INTERFACE_H
#define MY5170_PROFILE_INTERFACE_H

#include "ibmpc/board-common/pc_at_profile_interface.h"

#define VM_PROFILE_5170_SESSION_OPTION_MEMORY 0x02u
const vm_profile_default_pc_at_descriptor *
vm_profile_ibm_5170_model_339_descriptor_get(void);
lib_status vm_profile_ibm_5170_values_create(lib_size memory_bytes,
    vm_profile_contract_values *out_values);
lib_status vm_profile_ibm_5170_plan_create_memory(lib_size memory_bytes,
    vm_profile_default_pc_at_plan_snapshot *out_profile);
lib_status vm_profile_ibm_5170_plan_create(
    vm_profile_default_pc_at_plan_snapshot *out_profile);

#endif
