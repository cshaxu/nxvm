#include "type.h"
#include "core/machine/cpu.h"
#include "core/machine/machine_interface.h"
#include "../support/core_machine_cpu_fixture.h"

typedef struct eflags_machine { core_machine *machine; } eflags_machine;

static C_VOID eflags_reset(C_VOID *opaque)
{
    eflags_machine *state = (eflags_machine *)opaque;
    if (state != STD_NULL)
        (C_VOID)test_core_machine_fixture_reset_real_mode(state->machine);
}

static const core_machine_execution_provider eflags_provider = {
    eflags_reset, STD_NULL
};

static C_INT eflags_prepare_profile(core_machine_cpu_profile profile, eflags_machine *state)
{
    const core_machine_config config = {
        .memory_bytes = CORE_MACHINE_MINIMUM_MEMORY_BYTES,
        .cpu_profile = profile,
        .fpu_profile = CORE_MACHINE_FPU_PROFILE_NONE
    };
    STD_MEMSET(state, 0, sizeof(*state));
    return test_core_machine_fixture_create_bind_freeze_reset(&config,
        &eflags_provider, state, &state->machine);
}

static C_INT eflags_prepare(eflags_machine *state)
{
    return eflags_prepare_profile(CORE_MACHINE_CPU_PROFILE_80386, state);
}

static C_INT eflags_run(eflags_machine *state, type_unsigned_8 opcode, t_cpu *after)
{
    core_machine_run_result result;
    return test_core_machine_fixture_prepare_real_mode_execution(state->machine, 0u) &&
        core_machine_memory_write(state->machine, 0u, &opcode, 1u) == TYPE_STATUS_OK &&
        core_machine_run(state->machine, (core_machine_run_budget){ 1u, 0u }, &result) == TYPE_STATUS_OK &&
        result.reason == CORE_MACHINE_STOP_BUDGET &&
        ((*after = test_core_machine_fixture_capture_cpu_after_run(state->machine)), 1);
}

C_INT main(C_VOID)
{
    const type_unsigned_32 saved = VCPU_EFLAGS_CF | VCPU_EFLAGS_PF | VCPU_EFLAGS_AF |
        VCPU_EFLAGS_ZF | VCPU_EFLAGS_SF | VCPU_EFLAGS_IF |
        VCPU_EFLAGS_DF | VCPU_EFLAGS_OF;
    const type_unsigned_32 sahf_mask = VCPU_EFLAGS_CF | VCPU_EFLAGS_PF | VCPU_EFLAGS_AF |
        VCPU_EFLAGS_ZF | VCPU_EFLAGS_SF;
    type_unsigned_8 op;
    for (op = 0u; op != 2u; ++op) {
        eflags_machine state;
        t_cpu after;
        C_INT failed = !eflags_prepare(&state);
        if (!failed) {
            state.machine->executor_cpu.data.eax = op ? 0x1122ff00u : 0x11220000u;
            state.machine->executor_cpu.data.eflags = saved;
            failed |= !eflags_run(&state, 0x9eu, &after) ||
                (after.data.eflags & sahf_mask) != (op ? sahf_mask : 0u) ||
                (after.data.eflags & ~sahf_mask) != (saved & ~sahf_mask) ||
                after.data.eax != state.machine->executor_cpu.data.eax ||
                after.data.eip != 1u;
        }
        core_machine_destroy(state.machine);
        if (failed) return 1;
    }
    {
        eflags_machine state;
        t_cpu after;
        C_INT failed = !eflags_prepare(&state);
        if (!failed) {
            state.machine->executor_cpu.data.eax = 0x11220000u;
            state.machine->executor_cpu.data.eflags = saved;
            failed |= !eflags_run(&state, 0x9fu, &after) ||
                (after.data.eax & 0xff00u) != 0xd700u ||
                (after.data.eax & 0xffff00ffu) != 0x11220000u || after.data.eflags != saved;
        }
        core_machine_destroy(state.machine);
        if (failed) return 1;
    }
    {
        static const type_unsigned_8 opcodes[] = { 0xf5u, 0xf5u, 0xf8u, 0xf9u, 0xfcu, 0xfdu };
        type_unsigned_8 index;
        for (index = 0u; index != sizeof(opcodes); ++index) {
            eflags_machine state;
            t_cpu after;
            type_unsigned_32 initial = saved;
            type_unsigned_32 expected = saved;
            C_INT failed = !eflags_prepare(&state);
            if (opcodes[index] == 0xf5u)
                initial = index == 0u ? saved & ~VCPU_EFLAGS_CF : saved;
            if (opcodes[index] == 0xf5u)
                expected = initial ^ VCPU_EFLAGS_CF;
            if (opcodes[index] == 0xf8u) expected &= ~VCPU_EFLAGS_CF;
            if (opcodes[index] == 0xf9u) expected |= VCPU_EFLAGS_CF;
            if (opcodes[index] == 0xfcu) expected &= ~VCPU_EFLAGS_DF;
            if (opcodes[index] == 0xfdu) expected |= VCPU_EFLAGS_DF;
            if (!failed) {
                state.machine->executor_cpu.data.eax = 0x11223344u;
                state.machine->executor_cpu.data.eflags = initial;
                failed |= !eflags_run(&state, opcodes[index], &after) ||
                    after.data.eax != 0x11223344u || after.data.eflags != expected ||
                    after.data.eip != 1u;
            }
            core_machine_destroy(state.machine);
            if (failed) return 1;
        }
    }
    {
        static const type_unsigned_8 opcodes[] = { 0x9eu, 0x9fu, 0xf5u, 0xf8u, 0xf9u, 0xfcu, 0xfdu };
        core_machine_cpu_profile profiles[] = { CORE_MACHINE_CPU_PROFILE_8086,
            CORE_MACHINE_CPU_PROFILE_80186 };
        type_unsigned_8 profile;
        type_unsigned_8 index;
        for (profile = 0u; profile != 2u; ++profile) {
        for (index = 0u; index != sizeof(opcodes); ++index) {
            eflags_machine state;
            t_cpu after;
            C_INT failed = !eflags_prepare_profile(profiles[profile], &state);
            if (!failed) {
                state.machine->executor_cpu.data.eax = 0x1122ff00u;
                state.machine->executor_cpu.data.eflags = saved;
                failed |= !eflags_run(&state, opcodes[index], &after) || after.data.eip != 1u;
            }
            core_machine_destroy(state.machine);
            if (failed) return 1;
        }
        }
    }
    STD_PRINTF("M5:T316:S20:EFLAGS-LOCAL:OK\n");
    return 0;
}
