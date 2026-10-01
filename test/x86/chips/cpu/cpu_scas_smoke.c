#include "support/cpu_instruction_fixture.h"
#include <stdio.h>
/* T337_REAL_UD_TERMINAL_CPU_OWNER: SCAS prefix rejection is CPU-owned. */

#define SCAS_CMP_FLAGS (VCPU_EFLAGS_CF | VCPU_EFLAGS_PF | VCPU_EFLAGS_AF | \
    VCPU_EFLAGS_ZF | VCPU_EFLAGS_SF | VCPU_EFLAGS_OF)

static void scas_seed(cpu_instruction_fixture *state)
{
    t_cpu *cpu = &state->cpu;

    cpu->data.eax = 0xaabb0010u; cpu->data.ecx = 0x11220003u;
    cpu->data.edx = 0x778899aau; cpu->data.ebx = 0xbbccdEeu;
    cpu->data.esp = 0x8000u; cpu->data.ebp = 0x120u;
    cpu->data.esi = 0x10u; cpu->data.edi = 0x20u;
    cpu->data.eflags = VCPU_EFLAGS_IF;
    cpu->data.cs.base = 0u; cpu->data.es.base = 0x20000u;
    cpu->data.fs.base = 0x40000u;
}

static lib_i32 scas_others_same(const t_cpu *before, const t_cpu *after)
{
    return after->data.eax == before->data.eax &&
        after->data.edx == before->data.edx &&
        after->data.ebx == before->data.ebx &&
        after->data.esp == before->data.esp &&
        after->data.ebp == before->data.ebp &&
        after->data.esi == before->data.esi;
}

static lib_u32 scas_known_flags(core_machine_cpu_profile profile)
{
    return profile < CORE_MACHINE_CPU_PROFILE_80286 ? 0x0fd5u : 0x7fd5u;
}

static lib_i32 scas_flags_match(core_machine_cpu_profile profile,
    const t_cpu *before, const t_cpu *after, lib_u32 expected)
{
    lib_u32 unchanged = scas_known_flags(profile) & ~SCAS_CMP_FLAGS;

    return (after->data.eflags & SCAS_CMP_FLAGS) == expected &&
        (after->data.eflags & unchanged) ==
        (before->data.eflags & unchanged);
}

static lib_i32 scas_single(core_machine_cpu_profile profile,
    const lib_u8 *code, lib_u8 bytes, lib_u8 width, lib_bool address32,
    lib_bool decrement)
{
    cpu_instruction_fixture state;
    t_cpu before, after;
    lib_u32 image = width == 4u ? 0xaabb0001u : 1u;
    lib_u32 observed = 0u;
    lib_u32 index = address32 ? 0x1020u : 0x20u;
    lib_u32 physical = 0x20000u + index;

    cpu_instruction_prepare(&state, profile);
    scas_seed(&state);
    state.cpu.data.edi = address32 ? index :
        (state.cpu.data.edi & 0xffff0000u) | index;
    if (decrement) state.cpu.data.eflags |= VCPU_EFLAGS_DF;
    lib_memory_copy(state.memory + physical, &image, width);
    before = state.cpu;
    if (cpu_instruction_run(&state, code, bytes, &after) != LIB_STATUS_OK ||
        state.fault.valid || after.data.eip != bytes ||
        !scas_others_same(&before, &after) ||
        after.data.ecx != before.data.ecx ||
        after.data.edi != (address32 ? index + (decrement ?
        -(lib_i32)width : width) : (before.data.edi & 0xffff0000u) |
        (lib_u16)(index + (decrement ? -(lib_i32)width : width))) ||
        !scas_flags_match(profile, &before, &after,
        VCPU_EFLAGS_PF | VCPU_EFLAGS_AF)) {
        printf("SCAS single code=%02x profile=%u width=%u\n",
            code[0], profile, width);
        return 0;
    }
    lib_memory_copy(&observed, state.memory + physical, width);
    return observed == (width == 4u ? 0xaabb0001u : 1u);
}

static lib_i32 scas_flag_case(lib_u8 accumulator, lib_u8 image,
    lib_u32 expected)
{
    const lib_u8 code = 0xaeu;
    cpu_instruction_fixture state;
    t_cpu before, after;

    cpu_instruction_prepare(&state, CORE_MACHINE_CPU_PROFILE_80386);
    scas_seed(&state);
    state.cpu.data.eax = (state.cpu.data.eax & 0xffffff00u) | accumulator;
    state.memory[0x20020u] = image;
    before = state.cpu;
    if (cpu_instruction_run(&state, &code, 1u, &after) != LIB_STATUS_OK ||
        state.fault.valid || after.data.eip != 1u ||
        !scas_others_same(&before, &after) ||
        after.data.ecx != before.data.ecx || after.data.edi != 0x21u ||
        !scas_flags_match(CORE_MACHINE_CPU_PROFILE_80386,
            &before, &after, expected) ||
        state.memory[0x20020u] != image) return 0;
    return 1;
}

static lib_i32 scas_rep(core_machine_cpu_profile profile,
    const lib_u8 *code, lib_u8 bytes, lib_u16 count,
    const lib_u8 image[3], lib_u16 expected_count, lib_u16 expected_di,
    lib_u32 expected_flags)
{
    cpu_instruction_fixture state;
    t_cpu before, after;
    lib_u8 step;
    lib_u8 steps = count == 0u ? 1u : (lib_u8)(count - expected_count);

    cpu_instruction_prepare(&state, profile);
    scas_seed(&state);
    state.cpu.data.ecx = 0x11220000u | count;
    lib_memory_copy(state.memory + 0x20020u, image, 3u);
    lib_memory_copy(state.memory, code, bytes);
    before = state.cpu;
    for (step = 0u; step != steps; ++step) {
        core_machine_cpu_execution_refresh(&state.execution);
        if (state.execution.stop_requested) return 0;
    }
    after = state.cpu;
    if (state.fault.valid || after.data.eip != bytes ||
        !scas_others_same(&before, &after) ||
        after.data.ecx != ((before.data.ecx & 0xffff0000u) | expected_count) ||
        after.data.edi != ((before.data.edi & 0xffff0000u) | expected_di) ||
        !scas_flags_match(profile, &before, &after, expected_flags) ||
        lib_memory_compare(state.memory + 0x20020u, image, 3u) != 0) {
        printf("SCAS REP profile=%u code=%02x count=%u\n",
            profile, code[0], count);
        return 0;
    }
    return 1;
}

static lib_i32 scas_expect_ud(core_machine_cpu_profile profile,
    const lib_u8 *code, lib_u8 bytes)
{
    cpu_instruction_fixture state;
    t_cpu before, after;
    const lib_u32 image = 0x11223344u;

    cpu_instruction_prepare(&state, profile);
    scas_seed(&state);
    lib_memory_copy(state.memory + 0x20020u, &image, sizeof(image));
    state.cpu.data.idtr.limit = 0x17u;
    before = state.cpu;
    (void)cpu_instruction_run(&state, code, bytes, &after);
    return state.fault.valid &&
        (state.fault.exception_mask & VCPUINS_EXCEPT_UD) &&
        after.data.eip == 0u &&
        after.data.eax == before.data.eax &&
        after.data.ecx == before.data.ecx &&
        after.data.edx == before.data.edx &&
        after.data.ebx == before.data.ebx &&
        after.data.esp == before.data.esp &&
        after.data.ebp == before.data.ebp &&
        after.data.esi == before.data.esi &&
        after.data.edi == before.data.edi &&
        after.data.eflags == before.data.eflags &&
        lib_memory_compare(&before.data.es, &after.data.es,
            sizeof(before.data.es)) == 0 &&
        lib_memory_compare(state.memory + 0x20020u,
            &image, sizeof(image)) == 0;
}

static lib_i32 scas_test_rejections(void)
{
    static const core_machine_cpu_profile legacy[] = {
        CORE_MACHINE_CPU_PROFILE_8086, CORE_MACHINE_CPU_PROFILE_80186,
        CORE_MACHINE_CPU_PROFILE_80286
    };
    static const lib_u8 attr[][4] = {
        {0x66u,0xaeu}, {0x67u,0xafu}, {0x66u,0x67u,0xafu},
        {0xf3u,0x66u,0xaeu}, {0xf2u,0x67u,0xafu},
        {0xf3u,0x66u,0x67u,0xafu}
    };
    static const lib_u8 attr_bytes[] = {2u, 2u, 3u, 3u, 3u, 4u};
    static const lib_u8 locks[][5] = {
        {0xf0u,0xaeu}, {0xf0u,0xafu}, {0xf0u,0xf3u,0xaeu},
        {0xf0u,0xf2u,0xafu}, {0xf0u,0x66u,0xafu},
        {0xf0u,0x67u,0xaeu}, {0xf0u,0x66u,0x67u,0xafu},
        {0xf0u,0xf3u,0x66u,0x67u,0xafu}
    };
    static const lib_u8 lock_bytes[] = {2u, 2u, 3u, 3u, 3u, 3u, 4u, 5u};
    lib_u8 profile, form;

    for (profile = 0u; profile != sizeof(legacy) / sizeof(legacy[0]); ++profile)
        for (form = 0u; form != sizeof(attr_bytes); ++form)
            if (!scas_expect_ud(legacy[profile], attr[form],
                attr_bytes[form])) return 0;
    for (form = 0u; form != sizeof(lock_bytes); ++form)
        if (!scas_expect_ud(CORE_MACHINE_CPU_PROFILE_80386,
            locks[form], lock_bytes[form])) return 0;
    return 1;
}

static void scas_prepare_protected(cpu_instruction_fixture *state,
    lib_u32 es_limit)
{
    cpu_instruction_prepare(state, CORE_MACHINE_CPU_PROFILE_80386);
    scas_seed(state);
    state->cpu.data.cr0 |= VCPU_CR0_PE;
    state->cpu.data.cs.flagValid = LIB_TRUE;
    state->cpu.data.cs.selector = 0x08u;
    state->cpu.data.cs.sregtype = SREG_CODE;
    state->cpu.data.cs.base = 0x2000u;
    state->cpu.data.cs.limit = 0xffffu;
    state->cpu.data.cs.seg.executable = LIB_TRUE;
    state->cpu.data.cs.seg.exec.readable = LIB_TRUE;
    state->cpu.data.es.flagValid = LIB_TRUE;
    state->cpu.data.es.selector = 0x10u;
    state->cpu.data.es.sregtype = SREG_DATA;
    state->cpu.data.es.base = 0x3000u;
    state->cpu.data.es.limit = es_limit;
    state->cpu.data.es.seg.data.writable = LIB_TRUE;
    state->cpu.data.ss.flagValid = LIB_TRUE;
    state->cpu.data.ss.selector = 0x18u;
    state->cpu.data.ss.sregtype = SREG_STACK;
    state->cpu.data.ss.base = 0x4000u;
    state->cpu.data.ss.limit = 0xffffu;
    state->cpu.data.ss.seg.data.writable = LIB_TRUE;
}

static lib_i32 scas_test_protected(void)
{
    static const lib_u8 single[] = {0xaeu};
    static const lib_u8 rep[] = {0xf3u, 0xaeu};
    cpu_instruction_fixture state;
    t_cpu before, after;

    scas_prepare_protected(&state, 0x0fu);
    state.cpu.data.edi = 0x10u;
    state.memory[0x3010u] = 1u;
    state.memory[0x2000u] = single[0];
    before = state.cpu;
    core_machine_cpu_execution_refresh(&state.execution);
    after = state.cpu;
    if (!state.execution.stop_requested || !state.fault.valid ||
        !(state.fault.exception_mask & VCPUINS_EXCEPT_DF) ||
        after.data.eip != 0u || after.data.eax != before.data.eax ||
        after.data.ecx != before.data.ecx ||
        !scas_others_same(&before, &after) ||
        after.data.edi != before.data.edi ||
        after.data.eflags != before.data.eflags ||
        lib_memory_compare(&before.data.es, &after.data.es,
            sizeof(before.data.es)) != 0 || state.memory[0x3010u] != 1u)
        return 0;

    scas_prepare_protected(&state, 0x10u);
    state.cpu.data.edi = 0x10u;
    state.cpu.data.ecx = 0x11220003u;
    state.memory[0x3010u] = 0x10u;
    state.memory[0x3011u] = 1u;
    lib_memory_copy(state.memory + 0x2000u, rep, sizeof(rep));
    before = state.cpu;
    core_machine_cpu_execution_refresh(&state.execution);
    if (state.execution.stop_requested) return 0;
    core_machine_cpu_execution_refresh(&state.execution);
    after = state.cpu;
    return state.execution.stop_requested && state.fault.valid &&
        (state.fault.exception_mask & VCPUINS_EXCEPT_DF) &&
        after.data.eip == 0u && after.data.eax == before.data.eax &&
        after.data.ecx == 0x11220002u && after.data.edi == 0x11u &&
        scas_others_same(&before, &after) &&
        after.data.eflags == (VCPU_EFLAGS_IF | VCPU_EFLAGS_PF |
            VCPU_EFLAGS_ZF) &&
        lib_memory_compare(&before.data.es, &after.data.es,
            sizeof(before.data.es)) == 0 &&
        state.memory[0x3010u] == 0x10u && state.memory[0x3011u] == 1u;
}

lib_i32 main(void)
{
    static const core_machine_cpu_profile profiles[] = {
        CORE_MACHINE_CPU_PROFILE_8086, CORE_MACHINE_CPU_PROFILE_80186,
        CORE_MACHINE_CPU_PROFILE_80286, CORE_MACHINE_CPU_PROFILE_80386
    };
    static const lib_u8 ae = 0xaeu, af = 0xafu;
    static const lib_u8 dword[] = {0x66u, 0xafu};
    static const lib_u8 address[] = {0x67u, 0xaeu};
    static const lib_u8 both[] = {0x66u, 0x67u, 0xafu};
    static const lib_u8 cs[] = {0x2eu, 0xaeu}, fs[] = {0x64u, 0xaeu};
    static const lib_u8 repe[] = {0xf3u, 0xaeu};
    static const lib_u8 repne[] = {0xf2u, 0xaeu};
    static const lib_u8 equals[] = {0x10u, 1u, 0x10u};
    static const lib_u8 unequal[] = {1u, 0x10u, 1u};
    static const lib_u8 zero[] = {1u, 1u, 1u};
    static const lib_u8 one[] = {0x10u, 1u, 1u};
    lib_u8 profile;

    for (profile = 0u; profile != sizeof(profiles) / sizeof(profiles[0]); ++profile)
        if (!scas_single(profiles[profile], &ae, 1u, 1u,
                LIB_FALSE, LIB_FALSE) ||
            !scas_single(profiles[profile], &af, 1u, 2u,
                LIB_FALSE, LIB_FALSE) ||
            !scas_rep(profiles[profile], repe, 2u, 0u, zero,
                0u, 0x20u, 0u) ||
            !scas_rep(profiles[profile], repe, 2u, 1u, one,
                0u, 0x21u, VCPU_EFLAGS_PF | VCPU_EFLAGS_ZF) ||
            !scas_rep(profiles[profile], repne, 2u, 0u, zero,
                0u, 0x20u, 0u) ||
            !scas_rep(profiles[profile], repne, 2u, 1u, unequal,
                0u, 0x21u, VCPU_EFLAGS_PF | VCPU_EFLAGS_AF) ||
            !scas_rep(profiles[profile], repe, 2u, 3u, equals,
                1u, 0x22u, VCPU_EFLAGS_PF | VCPU_EFLAGS_AF) ||
            !scas_rep(profiles[profile], repne, 2u, 3u, unequal,
                1u, 0x22u, VCPU_EFLAGS_PF | VCPU_EFLAGS_ZF)) {
            printf("SCAS stage=profile %u\n", profile);
            return 1;
        }
    if (!scas_single(CORE_MACHINE_CPU_PROFILE_80386, dword, 2u, 4u,
            LIB_FALSE, LIB_FALSE) ||
        !scas_single(CORE_MACHINE_CPU_PROFILE_80386, address, 2u, 1u,
            LIB_TRUE, LIB_FALSE) ||
        !scas_single(CORE_MACHINE_CPU_PROFILE_80386, both, 3u, 4u,
            LIB_TRUE, LIB_FALSE) ||
        !scas_single(CORE_MACHINE_CPU_PROFILE_80386, &af, 1u, 2u,
            LIB_FALSE, LIB_TRUE) ||
        !scas_single(CORE_MACHINE_CPU_PROFILE_80386, cs, 2u, 1u,
            LIB_FALSE, LIB_FALSE) ||
        !scas_single(CORE_MACHINE_CPU_PROFILE_80386, fs, 2u, 1u,
            LIB_FALSE, LIB_FALSE)) {
        printf("SCAS stage=attributes\n");
        return 1;
    }
    if (!scas_flag_case(0x10u, 0x01u, VCPU_EFLAGS_PF | VCPU_EFLAGS_AF) ||
        !scas_flag_case(0x10u, 0x10u, VCPU_EFLAGS_PF | VCPU_EFLAGS_ZF) ||
        !scas_flag_case(0x00u, 0x01u, VCPU_EFLAGS_CF | VCPU_EFLAGS_PF |
            VCPU_EFLAGS_AF | VCPU_EFLAGS_SF) ||
        !scas_flag_case(0x80u, 0x01u, VCPU_EFLAGS_AF | VCPU_EFLAGS_OF) ||
        !scas_test_rejections() || !scas_test_protected()) {
        printf("SCAS stage=flags/rejections/protected\n");
        return 1;
    }
    printf("M5:T316:S36:SCAS:OK\n");
    printf("M5:T401:S19:SCAS-PROFILES:OK\n");
    return 0;
}
