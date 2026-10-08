#include "lib/types/types_interface.h"
#include "lib/types/file.h"
#include "support/cpu_instruction_fixture.h"

typedef struct control_transfer_jcc_case {
    lib_u8 opcode;
    lib_u32 taken_flags;
} control_transfer_jcc_case;

static lib_i32 control_transfer_run(cpu_instruction_fixture *state,
    const lib_u8 *code, lib_u8 bytes, t_cpu *after)
{
    state->cpu.data.eip = 0u;
    return cpu_instruction_run(state, code, bytes, after) == LIB_STATUS_OK &&
        !state->fault.valid;
}

static lib_i32 control_transfer_test_short_jcc(void)
{
    static const core_machine_cpu_profile profiles[] = {
        CORE_MACHINE_CPU_PROFILE_8086, CORE_MACHINE_CPU_PROFILE_80186,
        CORE_MACHINE_CPU_PROFILE_80286, CORE_MACHINE_CPU_PROFILE_80386
    };
    static const control_transfer_jcc_case cases[] = {
        {0x70u,VCPU_EFLAGS_OF}, {0x71u,0u},
        {0x72u,VCPU_EFLAGS_CF}, {0x73u,0u},
        {0x74u,VCPU_EFLAGS_ZF}, {0x75u,0u},
        {0x76u,VCPU_EFLAGS_CF}, {0x77u,0u},
        {0x78u,VCPU_EFLAGS_SF}, {0x79u,0u},
        {0x7au,VCPU_EFLAGS_PF}, {0x7bu,0u},
        {0x7cu,VCPU_EFLAGS_SF}, {0x7du,0u},
        {0x7eu,VCPU_EFLAGS_ZF}, {0x7fu,0u}
    };

    for (lib_size profile = 0u; profile < sizeof(profiles) / sizeof(profiles[0]);
        ++profile) {
        for (lib_size form = 0u; form < sizeof(cases) / sizeof(cases[0]);
            ++form) {
            const lib_u8 code[] = { cases[form].opcode, 2u };
            cpu_instruction_fixture state;
            t_cpu before;
            t_cpu after;

            cpu_instruction_prepare(&state, profiles[profile]);
            state.cpu.data.eax = 0x123456a5u;
            state.cpu.data.eflags = 0x00000002u | cases[form].taken_flags;
            before = state.cpu;
            if (!control_transfer_run(&state, code, sizeof(code), &after) ||
                after.data.eip != 4u || after.data.eax != before.data.eax ||
                after.data.eflags != before.data.eflags) return 0;

            cpu_instruction_prepare(&state, profiles[profile]);
            state.cpu.data.eax = 0x123456a5u;
            state.cpu.data.eflags = 0x00000002u |
                cases[form ^ 1u].taken_flags;
            before = state.cpu;
            if (!control_transfer_run(&state, code, sizeof(code), &after) ||
                after.data.eip != 2u || after.data.eax != before.data.eax ||
                after.data.eflags != before.data.eflags) return 0;
        }
    }
    return 1;
}

static lib_i32 control_transfer_test_zero_displacement_timing(void)
{
    static const core_machine_cpu_profile profiles[] = {
        CORE_MACHINE_CPU_PROFILE_8086, CORE_MACHINE_CPU_PROFILE_8088,
        CORE_MACHINE_CPU_PROFILE_80186, CORE_MACHINE_CPU_PROFILE_80286,
        CORE_MACHINE_CPU_PROFILE_80386
    };
    static const lib_u8 nonzero[] = { 0x74u, 0x01u, 0x90u, 0x90u };
    static const lib_u8 zero[] = { 0x74u, 0x00u, 0x90u };

    for (lib_size index = 0u; index < sizeof(profiles) / sizeof(profiles[0]);
        ++index) {
        cpu_instruction_fixture state;
        core_machine_cpu_timing_result nonzero_timing;
        core_machine_cpu_timing_result zero_timing;
        t_cpu after;

        cpu_instruction_prepare(&state, profiles[index]);
        state.cpu.data.eflags |= VCPU_EFLAGS_ZF;
        if (!control_transfer_run(&state, nonzero, sizeof(nonzero), &after) ||
            !core_machine_cpu_timing_select(&state.execution, &nonzero_timing) ||
            nonzero_timing.source_timing_unallocated) return 0;
        cpu_instruction_prepare(&state, profiles[index]);
        state.cpu.data.eflags |= VCPU_EFLAGS_ZF;
        if (!control_transfer_run(&state, zero, sizeof(zero), &after) ||
            !core_machine_cpu_timing_select(&state.execution, &zero_timing) ||
            zero_timing.source_timing_unallocated ||
            zero_timing.ticks != nonzero_timing.ticks) return 0;
    }
    return 1;
}

static lib_i32 control_transfer_test_jumps_and_near_jcc(void)
{
    static const lib_u8 code32_jumps[][6] = {
        { 0xebu,2u }, { 0xe9u,2u,0u,0u,0u },
        { 0x66u,0xe9u,2u,0u }
    };
    static const lib_u8 jump_sizes[] = { 2u, 5u, 4u };
    static const lib_u8 near_jcc_code32[] = { 0x0fu,0x84u,2u,0u,0u,0u };
    static const lib_u8 near_jcc_code16[] = { 0x66u,0x0fu,0x84u,2u,0u };
    static const lib_u8 near_jcc_default16[] = { 0x0fu,0x84u,2u,0u };
    static const lib_u8 near_jcc_default32[] = {
        0x66u,0x0fu,0x84u,2u,0u,0u,0u
    };
    static const control_transfer_jcc_case conditions[] = {
        {0x80u,VCPU_EFLAGS_OF}, {0x81u,0u},
        {0x82u,VCPU_EFLAGS_CF}, {0x83u,0u},
        {0x84u,VCPU_EFLAGS_ZF}, {0x85u,0u},
        {0x86u,VCPU_EFLAGS_CF}, {0x87u,0u},
        {0x88u,VCPU_EFLAGS_SF}, {0x89u,0u},
        {0x8au,VCPU_EFLAGS_PF}, {0x8bu,0u},
        {0x8cu,VCPU_EFLAGS_SF}, {0x8du,0u},
        {0x8eu,VCPU_EFLAGS_ZF}, {0x8fu,0u}
    };

    for (lib_size form = 0u; form < sizeof(code32_jumps) / sizeof(code32_jumps[0]);
        ++form) {
        cpu_instruction_fixture state;
        t_cpu after;

        cpu_instruction_prepare(&state, CORE_MACHINE_CPU_PROFILE_80386);
        state.cpu.data.cs.seg.data.big = LIB_TRUE;
        if (!control_transfer_run(&state, code32_jumps[form], jump_sizes[form],
            &after) || after.data.eip != jump_sizes[form] + 2u) return 0;
    }
    for (lib_size form = 0u; form < sizeof(conditions) / sizeof(conditions[0]);
        ++form) {
        lib_u8 code32[sizeof(near_jcc_code32)];
        cpu_instruction_fixture state;
        t_cpu before;
        t_cpu after;

        lib_memory_copy(code32, near_jcc_code32, sizeof(code32));
        code32[1] = conditions[form].opcode;
        cpu_instruction_prepare(&state, CORE_MACHINE_CPU_PROFILE_80386);
        state.cpu.data.cs.seg.data.big = LIB_TRUE;
        state.cpu.data.eax = 0x123456a5u;
        state.cpu.data.eflags = 0x00000002u | conditions[form].taken_flags;
        before = state.cpu;
        if (!control_transfer_run(&state, code32, sizeof(code32), &after) ||
            after.data.eip != 8u || after.data.eax != before.data.eax ||
            after.data.eflags != before.data.eflags) return 0;

        cpu_instruction_prepare(&state, CORE_MACHINE_CPU_PROFILE_80386);
        state.cpu.data.cs.seg.data.big = LIB_TRUE;
        state.cpu.data.eflags = 0x00000002u |
            conditions[form ^ 1u].taken_flags;
        if (!control_transfer_run(&state, code32, sizeof(code32), &after) ||
            after.data.eip != sizeof(code32)) return 0;
    }
    {
        cpu_instruction_fixture state;
        t_cpu after;

        cpu_instruction_prepare(&state, CORE_MACHINE_CPU_PROFILE_80386);
        state.cpu.data.cs.seg.data.big = LIB_TRUE;
        state.cpu.data.eflags = VCPU_EFLAGS_ZF | 0x00000002u;
        if (!control_transfer_run(&state, near_jcc_code16,
            sizeof(near_jcc_code16), &after) ||
            after.data.eip != sizeof(near_jcc_code16) + 2u) return 0;
    }
    for (lib_size form = 0u; form != 2u; ++form) {
        const lib_u8 *code = form == 0u ? near_jcc_default16 :
            near_jcc_default32;
        const lib_u8 bytes = form == 0u ? sizeof(near_jcc_default16) :
            sizeof(near_jcc_default32);
        cpu_instruction_fixture state;
        t_cpu after;

        cpu_instruction_prepare(&state, CORE_MACHINE_CPU_PROFILE_80386);
        state.cpu.data.eflags = VCPU_EFLAGS_ZF | 0x00000002u;
        if (!control_transfer_run(&state, code, bytes, &after) ||
            after.data.eip != bytes + 2u) return 0;
    }
    return 1;
}

static lib_i32 control_transfer_test_loop_and_jcxz(void)
{
    static const core_machine_cpu_profile profiles[] = {
        CORE_MACHINE_CPU_PROFILE_8086, CORE_MACHINE_CPU_PROFILE_80186,
        CORE_MACHINE_CPU_PROFILE_80286, CORE_MACHINE_CPU_PROFILE_80386
    };
    static const lib_u8 opcodes[] = { 0xe0u, 0xe1u, 0xe2u };

    for (lib_size profile = 0u; profile < sizeof(profiles) / sizeof(profiles[0]);
        ++profile) {
        for (lib_size opcode = 0u; opcode < sizeof(opcodes); ++opcode) {
            const lib_u8 code[] = { opcodes[opcode], 2u };
            cpu_instruction_fixture state;
            t_cpu before;
            t_cpu after;
            const lib_u32 flags = 0x00000002u |
                (opcodes[opcode] == 0xe1u ? VCPU_EFLAGS_ZF : 0u);

            cpu_instruction_prepare(&state, profiles[profile]);
            state.cpu.data.ecx = 2u;
            state.cpu.data.eax = 0x123456a5u;
            state.cpu.data.eflags = flags;
            before = state.cpu;
            if (!control_transfer_run(&state, code, sizeof(code), &after) ||
                after.data.eip != 4u || after.data.ecx != 1u ||
                after.data.eax != before.data.eax ||
                after.data.eflags != before.data.eflags) return 0;
        }
        for (lib_size condition = 0u; condition != 2u; ++condition) {
            const lib_u8 code[] = { condition == 0u ? 0xe0u : 0xe1u, 2u };
            cpu_instruction_fixture state;
            t_cpu after;

            cpu_instruction_prepare(&state, profiles[profile]);
            state.cpu.data.ecx = 2u;
            state.cpu.data.eflags = 0x00000002u |
                (condition == 0u ? VCPU_EFLAGS_ZF : 0u);
            if (!control_transfer_run(&state, code, sizeof(code), &after) ||
                after.data.eip != 2u || after.data.ecx != 1u) return 0;
        }
        for (lib_u8 count = 0u; count != 2u; ++count) {
            const lib_u8 code[] = { 0xe3u,2u };
            cpu_instruction_fixture state;
            t_cpu after;

            cpu_instruction_prepare(&state, profiles[profile]);
            state.cpu.data.ecx = count;
            if (!control_transfer_run(&state, code, sizeof(code), &after) ||
                after.data.eip != (count == 0u ? 4u : 2u) ||
                after.data.ecx != count) return 0;
        }
    }
    return 1;
}

static lib_i32 control_transfer_test_386_address_forms(void)
{
    static const lib_u8 loop[] = { 0xe2u,2u };
    static const lib_u8 address_loop[] = { 0x67u,0xe2u,2u };
    static const lib_u8 address_jcxz[] = { 0x67u,0xe3u,2u };
    cpu_instruction_fixture state;
    t_cpu after;

    cpu_instruction_prepare(&state, CORE_MACHINE_CPU_PROFILE_80386);
    state.cpu.data.ecx = 0xabcd0002u;
    if (!control_transfer_run(&state, loop, sizeof(loop), &after) ||
        after.data.ecx != 0xabcd0001u) return 0;

    cpu_instruction_prepare(&state, CORE_MACHINE_CPU_PROFILE_80386);
    state.cpu.data.ecx = 0x12340002u;
    if (!control_transfer_run(&state, address_loop, sizeof(address_loop), &after) ||
        after.data.ecx != 0x12340001u) return 0;

    cpu_instruction_prepare(&state, CORE_MACHINE_CPU_PROFILE_80386);
    state.cpu.data.cs.seg.data.big = LIB_TRUE;
    state.cpu.data.ecx = 0x12340002u;
    if (!control_transfer_run(&state, address_loop, sizeof(address_loop), &after) ||
        after.data.ecx != 0x12340001u) return 0;

    cpu_instruction_prepare(&state, CORE_MACHINE_CPU_PROFILE_80386);
    state.cpu.data.ecx = 0u;
    if (!control_transfer_run(&state, address_jcxz, sizeof(address_jcxz), &after) ||
        after.data.eip != 5u) return 0;
    return 1;
}

static lib_i32 control_transfer_expect_fault(const lib_u8 *code, lib_u8 bytes,
    lib_u32 flags, lib_u32 ecx)
{
    cpu_instruction_fixture state;

    cpu_instruction_prepare(&state, CORE_MACHINE_CPU_PROFILE_80386);
    state.cpu.data.cs.seg.data.big = LIB_TRUE;
    state.cpu.data.cs.limit = 0x007fu;
    state.cpu.data.eflags = flags;
    state.cpu.data.ecx = ecx;
    return cpu_instruction_expect_real_fault(&state, code, bytes, 13u);
}

static lib_i32 control_transfer_test_fault_atomicity_and_profile(void)
{
    static const lib_u8 jcc_fault[] = { 0x74u,0x7fu };
    static const lib_u8 loop_fault[] = { 0xe2u,0x7fu };
    static const lib_u8 near_jcc[] = { 0x0fu,0x84u,0u,0u };
    if (!control_transfer_expect_fault(jcc_fault, sizeof(jcc_fault),
        VCPU_EFLAGS_ZF | 0x00000002u, 0u) ||
        !control_transfer_expect_fault(loop_fault, sizeof(loop_fault),
            0x00000002u, 2u)) return 0;
    {
        cpu_instruction_fixture state;

        cpu_instruction_prepare(&state, CORE_MACHINE_CPU_PROFILE_80286);
        if (!cpu_instruction_expect_real_fault(&state, near_jcc, sizeof(near_jcc), 6u))
            return 0;
    }
    return 1;
}

int main(void)
{
    if (!control_transfer_test_short_jcc()) goto fail_short_jcc;
    if (!control_transfer_test_zero_displacement_timing()) goto fail_short_jcc;
    if (!control_transfer_test_jumps_and_near_jcc()) goto fail_near_jcc;
    if (!control_transfer_test_loop_and_jcxz()) goto fail_loop;
    if (!control_transfer_test_386_address_forms()) goto fail_address;
    if (!control_transfer_test_fault_atomicity_and_profile()) goto fail_fault;
    lib_c_printf("%s\n", "M5:T401:S43:LOOP-JCXZ-PROFILES:OK");
    lib_c_printf("%s\n", "M5:T401:S59:NEAR-JCC-PROFILES:OK");
    lib_c_printf("%s\n", "M5:T539:S49:CPU-CONTROL-TRANSFER-BRANCH:OK");
    return 0;

fail_short_jcc:
    lib_c_fprintf(lib_c_stderr, "%s", "short-jcc: ");
    goto fail;
fail_near_jcc:
    lib_c_fprintf(lib_c_stderr, "%s", "near-jcc: ");
    goto fail;
fail_loop:
    lib_c_fprintf(lib_c_stderr, "%s", "loop: ");
    goto fail;
fail_address:
    lib_c_fprintf(lib_c_stderr, "%s", "address: ");
    goto fail;
fail_fault:
    lib_c_fprintf(lib_c_stderr, "%s", "fault: ");
fail:
    lib_c_fprintf(lib_c_stderr, "%s", "M5:T539:S49:CPU-CONTROL-TRANSFER-BRANCH:FAIL\n");
    return 1;
}
