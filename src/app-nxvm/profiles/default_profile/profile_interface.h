#ifndef NXVM_DEFAULT_PROFILE_INTERFACE_H
#define NXVM_DEFAULT_PROFILE_INTERFACE_H

#include "ibmpc/board-common/pc_at_profile_interface.h"

#define VM_PROFILE_DEFAULT_AT_SESSION_OPTION_CPU_FPU 0x01u
#define VM_PROFILE_DEFAULT_AT_SESSION_OPTION_MEMORY 0x02u
#define VM_PROFILE_DEFAULT_AT_SESSION_OPTION_FLOPPY 0x04u

typedef struct vm_profile_default_at_request {
    lib_u32 requested_options;
    core_machine_cpu_profile cpu_profile;
    x86_fpu_profile fpu_profile;
    lib_size memory_bytes;
    lib_u8 floppy_cmos_type;
} vm_profile_default_at_request;

const vm_profile_default_pc_at_descriptor *
vm_profile_default_pc_at_descriptor_get(void);
lib_status vm_profile_default_at_plan_create(
    const vm_profile_default_at_request *request,
    vm_profile_default_pc_at_plan_snapshot *out_profile);

#endif
