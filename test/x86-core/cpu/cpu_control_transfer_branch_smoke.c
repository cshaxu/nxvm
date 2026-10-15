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
    static const x86_core_cpu_profile profiles[] = {
        X86_CORE_CPU_PROFILE_8086, X86_CORE_CPU_PROFILE_80186,
        X86_CORE_CPU_PROFILE_80286
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

static lib_i32 control_transfer_test_condition_matrix(x86_core_cpu_profile profile)
{
    for (lib_u32 context = 0u; context < 384u; ++context) {
        const lib_u8 mode = (lib_u8)(context % 6u);
        const lib_bool code32 = ((context / 6u) & 1u) != 0u;
        const lib_u8 prefixes = (lib_u8)((context / 12u) & 3u);
        const lib_bool near_form = ((context / 48u) & 1u) != 0u;
        const lib_bool backward = ((context / 96u) & 1u) != 0u;
        const lib_u32 origin = context >= 192u ? 0x10000u : 0u;
        const lib_bool operand32 = code32 != ((prefixes & 1u) != 0u);
        cpu_instruction_fixture state;

        if (code32 && (mode == 0u || mode == 5u)) continue;
        if (origin != 0u && !code32) continue;
        cpu_instruction_prepare(&state, profile);
        if (mode != 0u) cpu_instruction_prepare_protected_exception(
            &state, mode == 5u ? 4u : (lib_u8)(mode - 1u));
        state.cpu.data.cs.seg.exec.defsize = code32;
        if (origin != 0u) state.cpu.data.cs.limit = 0x1ffffu;
        state.cpu.data.eax = 0x123456a5u;
        state.cpu.data.ecx = 0x89abcdefu;
        for (lib_u32 index = 0u; index < 512u; ++index) {
            const lib_u8 condition = (lib_u8)(index & 15u);
            const lib_u8 image = (lib_u8)(index >> 4u);
            const lib_bool cf = (image & 1u) != 0u;
            const lib_bool pf = (image & 2u) != 0u;
            const lib_bool zf = (image & 4u) != 0u;
            const lib_bool sf = (image & 8u) != 0u;
            const lib_bool of = (image & 16u) != 0u;
            const lib_bool taken[] = {of,!of,cf,!cf,zf,!zf,cf || zf,
                !cf && !zf,sf,!sf,pf,!pf,sf != of,sf == of,
                zf || sf != of,!zf && sf == of};
            const lib_u8 displacement_bytes = near_form ?
                (operand32 ? 4u : 2u) : 1u;
            lib_u8 code[8] = {0u};
            lib_u8 bytes = 0u;
            t_cpu expected;
            t_cpu after;

            if ((prefixes & 1u) != 0u) code[bytes++] = 0x66u;
            if ((prefixes & 2u) != 0u) code[bytes++] = 0x67u;
            if (near_form) code[bytes++] = 0x0fu;
            code[bytes++] = (lib_u8)((near_form ? 0x80u : 0x70u) | condition);
            for (lib_u8 byte = 0u; byte < displacement_bytes; ++byte)
                code[bytes++] = backward ? 0xffu : (byte == 0u ? 17u : 0u);
            state.cpu.data.eip = origin;
            state.cpu.data.eflags = 2u | VCPU_EFLAGS_AF | VCPU_EFLAGS_DF |
                VCPU_EFLAGS_IF | (mode == 5u ? VCPU_EFLAGS_VM : 0u) |
                (cf ? VCPU_EFLAGS_CF : 0u) | (pf ? VCPU_EFLAGS_PF : 0u) |
                (zf ? VCPU_EFLAGS_ZF : 0u) | (sf ? VCPU_EFLAGS_SF : 0u) |
                (of ? VCPU_EFLAGS_OF : 0u);
            expected = state.cpu;
            expected.data.eip = origin + bytes;
            if (taken[condition]) {
                expected.data.eip += backward ? 0xffffffffu : 17u;
                if (!operand32) expected.data.eip &= 0xffffu;
            }
            lib_memory_copy(state.memory + origin, code, bytes);
            x86_core_cpu_execution_invalidate_prefetch(&state.execution);
            if (cpu_instruction_run(&state, code, bytes, &after) != LIB_STATUS_OK ||
                state.fault.valid ||
                state.delivered_exception.valid || state.writes != 0u ||
                x86_core_cpu_is_shutdown(&state.execution) ||
                lib_memory_compare(&expected, &after, sizeof(after)) != 0) {
                lib_c_fprintf(lib_c_stderr,
                    "JCC:MATRIX profile=%u context=%u case=%u eip=%x/%x\n",
                    profile, context, index, after.data.eip, expected.data.eip);
                return 0;
            }
        }
    }
    return 1;
}

static lib_i32 control_transfer_test_counter_matrix(x86_core_cpu_profile profile)
{
    static const lib_u32 counts[] = {0u,1u,2u,0xffffu,0x10000u,0xffffffffu,
        0xabcd0002u,0x12340002u};

    for (lib_u32 context = 0u; context < 192u; ++context) {
        const lib_u8 mode = (lib_u8)(context % 6u);
        const lib_bool code32 = ((context / 6u) & 1u) != 0u;
        const lib_u8 prefixes = (lib_u8)((context / 12u) & 3u);
        const lib_bool backward = ((context / 48u) & 1u) != 0u;
        const lib_u32 origin = context >= 96u ? 0x10000u : 0u;
        const lib_bool address32 = code32 != ((prefixes & 2u) != 0u);
        const lib_bool operand32 = code32 != ((prefixes & 1u) != 0u);
        cpu_instruction_fixture state;

        if (code32 && (mode == 0u || mode == 5u)) continue;
        if (origin != 0u && !code32) continue;
        cpu_instruction_prepare(&state, profile);
        if (mode != 0u) cpu_instruction_prepare_protected_exception(
            &state, mode == 5u ? 4u : (lib_u8)(mode - 1u));
        state.cpu.data.cs.seg.exec.defsize = code32;
        if (origin != 0u) state.cpu.data.cs.limit = 0x1ffffu;
        state.cpu.data.eax = 0x123456a5u;
        for (lib_u32 index = 0u; index < 64u; ++index) {
            const lib_u8 operation = (lib_u8)(index & 3u);
            const lib_bool zf = (index & 4u) != 0u;
            const lib_u32 count = counts[index >> 3u];
            const lib_u32 active = address32 ? count : count & 0xffffu;
            const lib_u32 remaining = operation == 3u ? active :
                ((active - 1u) & (address32 ? 0xffffffffu : 0xffffu));
            const lib_bool taken = operation == 3u ? active == 0u :
                remaining != 0u && (operation == 2u ||
                    (operation == 1u ? zf : !zf));
            lib_u8 code[4];
            lib_u8 bytes = 0u;
            t_cpu expected;
            t_cpu after;

            if ((prefixes & 1u) != 0u) code[bytes++] = 0x66u;
            if ((prefixes & 2u) != 0u) code[bytes++] = 0x67u;
            code[bytes++] = (lib_u8)(0xe0u | operation);
            code[bytes++] = backward ? 0xffu : 17u;
            state.cpu.data.eip = origin;
            state.cpu.data.ecx = count;
            state.cpu.data.eflags = 2u | VCPU_EFLAGS_CF | VCPU_EFLAGS_OF |
                VCPU_EFLAGS_AF | VCPU_EFLAGS_DF | VCPU_EFLAGS_IF |
                (zf ? VCPU_EFLAGS_ZF : 0u) | (mode == 5u ? VCPU_EFLAGS_VM : 0u);
            expected = state.cpu;
            if (operation != 3u) expected.data.ecx = address32 ? remaining :
                (count & 0xffff0000u) | remaining;
            expected.data.eip = origin + bytes;
            if (taken) {
                expected.data.eip += backward ? 0xffffffffu : 17u;
                if (!operand32) expected.data.eip &= 0xffffu;
            }
            lib_memory_copy(state.memory + origin, code, bytes);
            x86_core_cpu_execution_invalidate_prefetch(&state.execution);
            if (cpu_instruction_run(&state, code, bytes, &after) != LIB_STATUS_OK ||
                state.fault.valid ||
                state.delivered_exception.valid || state.writes != 0u ||
                x86_core_cpu_is_shutdown(&state.execution) ||
                lib_memory_compare(&expected, &after, sizeof(after)) != 0) {
                lib_c_fprintf(lib_c_stderr,
                    "LOOP-JCXZ:MATRIX profile=%u context=%u case=%u ecx=%x/%x eip=%x/%x\n",
                    profile, context, index, after.data.ecx, expected.data.ecx,
                    after.data.eip, expected.data.eip);
                return 0;
            }
        }
    }
    return 1;
}

static lib_i32 control_transfer_test_zero_displacement_timing(void)
{
    static const x86_core_cpu_profile profiles[] = {
        X86_CORE_CPU_PROFILE_8086, X86_CORE_CPU_PROFILE_8088,
        X86_CORE_CPU_PROFILE_80186, X86_CORE_CPU_PROFILE_80286,
        X86_CORE_CPU_PROFILE_80386
    };
    static const lib_u8 nonzero[] = { 0x74u, 0x01u, 0x90u, 0x90u };
    static const lib_u8 zero[] = { 0x74u, 0x00u, 0x90u };

    for (lib_size index = 0u; index < sizeof(profiles) / sizeof(profiles[0]);
        ++index) {
        cpu_instruction_fixture state;
        x86_core_cpu_timing_result nonzero_timing;
        x86_core_cpu_timing_result zero_timing;
        t_cpu after;

        cpu_instruction_prepare(&state, profiles[index]);
        state.cpu.data.eflags |= VCPU_EFLAGS_ZF;
        if (!control_transfer_run(&state, nonzero, sizeof(nonzero), &after)) return 0;
        nonzero_timing = x86_core_cpu_capture_timing(&state.execution);
        if (nonzero_timing.source_timing_unallocated || !nonzero_timing.ticks) return 0;
        cpu_instruction_prepare(&state, profiles[index]);
        state.cpu.data.eflags |= VCPU_EFLAGS_ZF;
        if (!control_transfer_run(&state, zero, sizeof(zero), &after)) return 0;
        zero_timing = x86_core_cpu_capture_timing(&state.execution);
        if (zero_timing.source_timing_unallocated ||
            zero_timing.ticks != nonzero_timing.ticks) return 0;
    }
    return 1;
}

static lib_i32 control_transfer_test_jumps(x86_core_cpu_profile profile)
{
    static const lib_u8 code32_jumps[][6] = {
        { 0xebu,2u }, { 0xe9u,2u,0u,0u,0u },
        { 0x66u,0xe9u,2u,0u }
    };
    static const lib_u8 jump_sizes[] = { 2u, 5u, 4u };

    for (lib_size form = 0u; form < sizeof(code32_jumps) / sizeof(code32_jumps[0]);
        ++form) {
        cpu_instruction_fixture state;
        t_cpu after;

        cpu_instruction_prepare(&state, profile);
        state.cpu.data.cs.seg.data.big = LIB_TRUE;
        if (!control_transfer_run(&state, code32_jumps[form], jump_sizes[form],
            &after) || after.data.eip != jump_sizes[form] + 2u) return 0;
    }
    return 1;
}

static lib_i32 control_transfer_test_loop_and_jcxz(void)
{
    static const x86_core_cpu_profile profiles[] = {
        X86_CORE_CPU_PROFILE_8086, X86_CORE_CPU_PROFILE_80186,
        X86_CORE_CPU_PROFILE_80286
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

static lib_i32 control_transfer_expect_fault(x86_core_cpu_profile profile,
    const lib_u8 *code, lib_u8 bytes, lib_u32 flags, lib_u32 ecx)
{
    cpu_instruction_fixture state;

    cpu_instruction_prepare(&state, profile);
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
    static const x86_core_cpu_profile profiles[] = {
        X86_CORE_CPU_PROFILE_80386, X86_CORE_CPU_PROFILE_80486SX
    };

    for (lib_size index = 0u; index < sizeof(profiles) / sizeof(profiles[0]);
        ++index) {
        if (!control_transfer_expect_fault(profiles[index], jcc_fault,
                sizeof(jcc_fault), VCPU_EFLAGS_ZF | 0x00000002u, 0u) ||
            !control_transfer_expect_fault(profiles[index], loop_fault,
                sizeof(loop_fault), 0x00000002u, 2u)) return 0;
    }
    {
        cpu_instruction_fixture state;

        cpu_instruction_prepare(&state, X86_CORE_CPU_PROFILE_80286);
        if (!cpu_instruction_expect_real_fault(&state, near_jcc, sizeof(near_jcc), 6u))
            return 0;
    }
    return 1;
}

int main(void)
{
    if (!control_transfer_test_short_jcc()) goto fail_short_jcc;
    if (!control_transfer_test_condition_matrix(X86_CORE_CPU_PROFILE_80386) ||
        !control_transfer_test_condition_matrix(X86_CORE_CPU_PROFILE_80486SX))
        goto fail_near_jcc;
    if (!control_transfer_test_counter_matrix(X86_CORE_CPU_PROFILE_80386) ||
        !control_transfer_test_counter_matrix(X86_CORE_CPU_PROFILE_80486SX))
        goto fail_loop;
    if (!control_transfer_test_zero_displacement_timing()) goto fail_short_jcc;
    if (!control_transfer_test_jumps(X86_CORE_CPU_PROFILE_80386) ||
        !control_transfer_test_jumps(X86_CORE_CPU_PROFILE_80486SX))
        goto fail_near_jcc;
    if (!control_transfer_test_loop_and_jcxz()) goto fail_loop;
    if (!control_transfer_test_fault_atomicity_and_profile()) goto fail_fault;
    lib_c_printf("%s\n", "LOOP-JCXZ-PROFILES:OK");
    lib_c_printf("%s\n", "NEAR-JCC-PROFILES:OK");
    lib_c_printf("%s\n", "CPU-CONTROL-TRANSFER-BRANCH:OK");
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
fail_fault:
    lib_c_fprintf(lib_c_stderr, "%s", "fault: ");
fail:
    lib_c_fprintf(lib_c_stderr, "%s", "CPU-CONTROL-TRANSFER-BRANCH:FAIL\n");
    return 1;
}
