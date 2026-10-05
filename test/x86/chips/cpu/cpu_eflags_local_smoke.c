#include "lib/types/types_interface.h"
#include "lib/types/file.h"
#include "support/cpu_bus_fixture.h"

static lib_i32 eflags_run(cpu_bus_fixture *state, lib_u8 opcode, t_cpu *after)
{
    if (core_machine_cpu_execution_load_segment(&state->execution,
            &state->cpu.data.cs, 0u) ||
        core_machine_cpu_execution_load_segment(&state->execution,
            &state->cpu.data.ds, 0u) ||
        core_machine_cpu_execution_load_segment(&state->execution,
            &state->cpu.data.es, 0u) ||
        core_machine_cpu_execution_load_segment(&state->execution,
            &state->cpu.data.ss, 0u)) return 0;
    state->cpu.data.eip = 0u;
    state->cpu.data.flagHalt = LIB_FALSE;
    state->memory[0] = opcode;
    core_machine_cpu_execution_refresh(&state->execution);
    *after = state->cpu;
    return state->instruction_count == 1u && !state->faults &&
        !state->instructions.data.except && !state->cpu.data.flagHalt;
}

lib_i32 main(void)
{
    const lib_u32 saved = VCPU_EFLAGS_CF | VCPU_EFLAGS_PF | VCPU_EFLAGS_AF |
        VCPU_EFLAGS_ZF | VCPU_EFLAGS_SF | VCPU_EFLAGS_IF |
        VCPU_EFLAGS_DF | VCPU_EFLAGS_OF;
    const lib_u32 sahf_mask = VCPU_EFLAGS_CF | VCPU_EFLAGS_PF | VCPU_EFLAGS_AF |
        VCPU_EFLAGS_ZF | VCPU_EFLAGS_SF;
    lib_u8 op;
    for (op = 0u; op != 2u; ++op) {
        cpu_bus_fixture state;
        t_cpu after;
        lib_i32 failed = 0;
        cpu_bus_prepare(&state, CORE_MACHINE_CPU_PROFILE_80386);
        state.cpu.data.eax = op ? 0x1122ff00u : 0x11220000u;
        state.cpu.data.eflags = saved;
        failed |= !eflags_run(&state, 0x9eu, &after) ||
            (after.data.eflags & sahf_mask) != (op ? sahf_mask : 0u) ||
            (after.data.eflags & ~sahf_mask) != (saved & ~sahf_mask) ||
            after.data.eax != (op ? 0x1122ff00u : 0x11220000u) ||
            after.data.eip != 1u;
        if (failed) return 1;
    }
    {
        cpu_bus_fixture state;
        t_cpu after;
        lib_i32 failed = 0;
        cpu_bus_prepare(&state, CORE_MACHINE_CPU_PROFILE_80386);
        state.cpu.data.eax = 0x11220000u;
        state.cpu.data.eflags = saved;
        failed |= !eflags_run(&state, 0x9fu, &after) ||
            (after.data.eax & 0xff00u) != 0xd700u ||
            (after.data.eax & 0xffff00ffu) != 0x11220000u || after.data.eflags != saved;
        if (failed) return 1;
    }
    {
        static const lib_u8 opcodes[] = { 0xf5u, 0xf5u, 0xf8u, 0xf9u, 0xfcu, 0xfdu };
        lib_u8 index;
        for (index = 0u; index != sizeof(opcodes); ++index) {
            cpu_bus_fixture state;
            t_cpu after;
            lib_u32 initial = saved;
            lib_u32 expected = saved;
            lib_i32 failed = 0;
            cpu_bus_prepare(&state, CORE_MACHINE_CPU_PROFILE_80386);
            if (opcodes[index] == 0xf5u)
                initial = index == 0u ? saved & ~VCPU_EFLAGS_CF : saved;
            if (opcodes[index] == 0xf5u)
                expected = initial ^ VCPU_EFLAGS_CF;
            if (opcodes[index] == 0xf8u) expected &= ~VCPU_EFLAGS_CF;
            if (opcodes[index] == 0xf9u) expected |= VCPU_EFLAGS_CF;
            if (opcodes[index] == 0xfcu) expected &= ~VCPU_EFLAGS_DF;
            if (opcodes[index] == 0xfdu) expected |= VCPU_EFLAGS_DF;
            state.cpu.data.eax = 0x11223344u;
            state.cpu.data.eflags = initial;
            failed |= !eflags_run(&state, opcodes[index], &after) ||
                after.data.eax != 0x11223344u || after.data.eflags != expected ||
                after.data.eip != 1u;
            if (failed) return 1;
        }
    }
    {
        static const lib_u8 opcodes[] = { 0x9eu, 0x9fu, 0xf5u, 0xf8u, 0xf9u, 0xfcu, 0xfdu };
        core_machine_cpu_profile profiles[] = { CORE_MACHINE_CPU_PROFILE_8086,
            CORE_MACHINE_CPU_PROFILE_80186 };
        lib_u8 profile;
        lib_u8 index;
        for (profile = 0u; profile != 2u; ++profile) {
        for (index = 0u; index != sizeof(opcodes); ++index) {
            cpu_bus_fixture state;
            t_cpu after;
            lib_i32 failed = 0;
            cpu_bus_prepare(&state, profiles[profile]);
            state.cpu.data.eax = 0x1122ff00u;
            state.cpu.data.eflags = saved;
            failed |= !eflags_run(&state, opcodes[index], &after) || after.data.eip != 1u;
            if (failed) return 1;
        }
        }
    }
    lib_c_printf("M5:T316:S20:EFLAGS-LOCAL:OK\n");
    return 0;
}
