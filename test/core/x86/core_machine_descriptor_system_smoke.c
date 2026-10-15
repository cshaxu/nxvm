#include "lib/types/types_interface.h"
#include "core/x86/debug_interface.h"
#include "lib/types/file.h"

lib_i32 main(void)
{
    static const lib_u8 code[] = {
        0x26u, 0xc7u, 0x47u, 0x02u, 0xffu, 0xffu, 0xf4u
    };
    const core_machine_executor_config config = {
        .memory_bytes = (2u * 1024u * 1024u),
        .cpu_profile = X86_CORE_CPU_PROFILE_80386,
        .fpu_profile = X86_FPU_PROFILE_NONE
    };
    const x86_core_cpu_debug_register_patch entry = {
        .mask = X86_CORE_CPU_DEBUG_REGISTER_MASK(X86_CORE_CPU_DEBUG_CS) |
            X86_CORE_CPU_DEBUG_REGISTER_MASK(X86_CORE_CPU_DEBUG_DS) |
            X86_CORE_CPU_DEBUG_REGISTER_MASK(X86_CORE_CPU_DEBUG_ES) |
            X86_CORE_CPU_DEBUG_REGISTER_MASK(X86_CORE_CPU_DEBUG_SS) |
            X86_CORE_CPU_DEBUG_REGISTER_MASK(X86_CORE_CPU_DEBUG_EIP) |
            X86_CORE_CPU_DEBUG_REGISTER_MASK(X86_CORE_CPU_DEBUG_EBX)
    };
    core_machine *machine = LIB_NULL;
    core_machine_run_result result;
    x86_core_cpu_diagnostic diagnostic;
    lib_u16 observed = 0u;
    lib_i32 failed = core_machine_neutral_create(&config, &machine) != LIB_STATUS_OK ||
        core_machine_freeze_execution_providers(machine) != LIB_STATUS_OK ||
        core_machine_reset(machine) != LIB_STATUS_OK ||
        core_machine_debug_patch_registers(machine, &entry) != LIB_STATUS_OK ||
        core_machine_memory_write(machine, 0u, code, sizeof(code)) != LIB_STATUS_OK ||
        core_machine_run(machine, (core_machine_run_budget){32u, 0u}, &result) != LIB_STATUS_OK ||
        result.reason != CORE_MACHINE_STOP_WAITING_FOR_INTERRUPT ||
        core_machine_get_cpu_diagnostic(machine, &diagnostic) != LIB_STATUS_OK ||
        diagnostic.first_fault.valid ||
        core_machine_memory_read(machine, 2u, &observed, sizeof(observed)) != LIB_STATUS_OK ||
        observed != 0xffffu;

    core_machine_destroy(machine);
    if (failed) return 1;
    lib_c_printf("M5::CONTROL-STATE:OK\n");
    return 0;
}
