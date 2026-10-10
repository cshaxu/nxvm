#include "app-nxvm/profiles/profile_interface.h"
#include "lib/types/types_interface.h"

lib_i32 main(void)
{
    const vm_profile_default_at_request request = {
        VM_PROFILE_DEFAULT_AT_SESSION_OPTION_CPU_FPU |
        VM_PROFILE_DEFAULT_AT_SESSION_OPTION_MEMORY |
        VM_PROFILE_DEFAULT_AT_SESSION_OPTION_FLOPPY,
        CORE_MACHINE_CPU_PROFILE_80386, X86_FPU_PROFILE_NONE,
        32u * 1024u * 1024u, 0x40u};
    vm_profile_default_pc_at_plan_snapshot profile;

    return vm_profile_default_at_plan_create(&request, &profile) != LIB_STATUS_OK ||
        profile.values.core.configuration.cpu_profile != CORE_MACHINE_CPU_PROFILE_80386 ||
        profile.values.core.configuration.memory_bytes != 32u * 1024u * 1024u ||
        profile.values.allowed_session_options !=
            (VM_PROFILE_DEFAULT_AT_SESSION_OPTION_CPU_FPU |
             VM_PROFILE_DEFAULT_AT_SESSION_OPTION_MEMORY |
             VM_PROFILE_DEFAULT_AT_SESSION_OPTION_FLOPPY) ||
        profile.descriptor.cmos.floppy_type != 0x40u;
}
