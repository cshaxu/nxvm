#include "support/cpu_bus_fixture.h"
#include "lib/types/file.h"

static lib_i32 cpu_completion_wake(x86_core_cpu_profile profile, lib_bool nmi, lib_u8 failure)
{
    cpu_bus_fixture fixture;
    cpu_bus_prepare(&fixture,profile);
    fixture.cpu.data.flags |= VCPU_EFLAGS_IF;
    fixture.memory[0x100u]=0xf4u;
    fixture.memory[0x101u]=0x90u;
    const lib_u8 vector=nmi ? 2u : 0x30u;
    fixture.memory[4u*vector]=0u;
    fixture.memory[4u*vector+1u]=2u;
    fixture.memory[0x200u]=0x90u;
    x86_core_cpu_execution_refresh(&fixture.execution);
    if (fixture.execution.stop_requested || !fixture.cpu.data.flagHalt ||
        !fixture.execution.completed_observation_valid ||
        x86_core_cpu_execution_consume_instruction_fault_delivery(&fixture.execution)) return 1;
    t_cpu checkpoint=fixture.cpu;
    checkpoint.data.flagHalt=LIB_FALSE;
    if (nmi) {
        if (!x86_core_cpu_request_nmi(&fixture.execution)) return 1;
    } else fixture.interrupt=LIB_TRUE;
    fixture.fail_transfer=failure == 1u;
    fixture.fail_acknowledge=failure == 2u;
    x86_core_cpu_execution_refresh(&fixture.execution);
    const lib_bool delivered=x86_core_cpu_execution_consume_instruction_fault_delivery(&fixture.execution);
    if (fixture.execution.completed_observation_valid ||
        x86_core_cpu_execution_consume_instruction_fault_delivery(&fixture.execution) ||
        x86_core_cpu_capture_timing(&fixture.execution).ticks) return 1;
    if (failure) {
        if (!fixture.execution.stop_requested || fixture.faults != 1u || delivered ||
            fixture.delivered_exceptions || fixture.writes ||
            lib_memory_compare(&fixture.cpu,&checkpoint,sizeof(checkpoint))) return 1;
    } else {
        if (fixture.execution.stop_requested || fixture.faults || !delivered ||
            fixture.cpu.data.eip != 0x200u || fixture.cpu.data.sp != 0x6fau ||
            fixture.cpu.data.flagHalt || fixture.acknowledgements != (lib_u32)!nmi) return 1;
        x86_core_cpu_execution_refresh(&fixture.execution);
        if (fixture.execution.stop_requested || !fixture.execution.completed_observation_valid ||
            x86_core_cpu_execution_consume_instruction_fault_delivery(&fixture.execution) ||
            fixture.cpu.data.eip != 0x201u) return 1;
    }
    return 0;
}

static lib_i32 cpu_completion_divide(x86_core_cpu_profile profile)
{
    cpu_bus_fixture fixture;
    cpu_bus_prepare(&fixture,profile);
    fixture.cpu.data.bx=0u;
    fixture.memory[0x100u]=0xf6u;
    fixture.memory[0x101u]=0xf3u;
    fixture.memory[0u]=0u;
    fixture.memory[1u]=2u;
    x86_core_cpu_execution_refresh(&fixture.execution);
    const lib_bool software=profile == X86_CORE_CPU_PROFILE_V30;
    const lib_bool delivered=x86_core_cpu_execution_consume_instruction_fault_delivery(&fixture.execution);
    if (fixture.execution.stop_requested || fixture.faults || fixture.delivered_exceptions != 1u ||
        fixture.cpu.data.eip != 0x200u || fixture.cpu.data.sp != 0x6fau ||
        fixture.execution.completed_observation_valid != software || delivered == software ||
        x86_core_cpu_execution_consume_instruction_fault_delivery(&fixture.execution)) {
        lib_c_printf("CPU:COMPLETION:RESULT stop=%u faults=%u diagnostics=%u pc=%x sp=%x completed=%u event=%u\n",
            fixture.execution.stop_requested,fixture.faults,fixture.delivered_exceptions,
            fixture.cpu.data.eip,fixture.cpu.data.sp,fixture.execution.completed_observation_valid,delivered);
        return 1;
    }
    return 0;
}

lib_i32 main(void)
{
    static const x86_core_cpu_profile profiles[]={
        X86_CORE_CPU_PROFILE_8086,X86_CORE_CPU_PROFILE_8088,X86_CORE_CPU_PROFILE_80186,
        X86_CORE_CPU_PROFILE_80286,X86_CORE_CPU_PROFILE_80386,X86_CORE_CPU_PROFILE_80486SX,
        X86_CORE_CPU_PROFILE_V30};
    /* DEFAULT is a construction sentinel, not a runnable chip alias. */
    cpu_bus_fixture rejected;
    cpu_bus_prepare(&rejected,X86_CORE_CPU_PROFILE_DEFAULT);
    rejected.memory[0x100u]=0xf4u;
    x86_core_cpu_execution_refresh(&rejected.execution);
    if (!rejected.execution.stop_requested || rejected.faults != 1u ||
        rejected.execution.completed_observation_valid ||
        x86_core_cpu_execution_consume_instruction_fault_delivery(&rejected.execution)) return 1;
    lib_u32 cases=1u;
    for (lib_u8 index=0u; index < sizeof(profiles)/sizeof(profiles[0]); ++index) {
        if (cpu_completion_divide(profiles[index])) {
            lib_c_printf("CPU:COMPLETION:DIVIDE profile=%u\n",profiles[index]);
            return 1;
        }
        ++cases;
        for (lib_u8 nmi=0u; nmi < 2u; ++nmi)
            for (lib_u8 failure=0u; failure < (nmi ? 2u : 3u); ++failure) {
                if (cpu_completion_wake(profiles[index],nmi != 0u,failure)) {
                    lib_c_printf("CPU:COMPLETION:WAKE profile=%u nmi=%u failure=%u\n",
                        profiles[index],nmi,failure);
                    return 1;
                }
                ++cases;
            }
    }
    lib_c_printf("CPU:COMPLETION:OK cases=%u\n",cases);
    return 0;
}
