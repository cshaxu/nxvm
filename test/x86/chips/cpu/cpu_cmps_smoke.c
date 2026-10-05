#include "support/cpu_instruction_fixture.h"
#include "lib/types/file.h"
/* T337_REAL_UD_TERMINAL_CPU_OWNER: CMPS prefix rejection is CPU-owned. */

#define CMPS_FLAGS (VCPU_EFLAGS_CF | VCPU_EFLAGS_PF | VCPU_EFLAGS_AF | \
    VCPU_EFLAGS_ZF | VCPU_EFLAGS_SF | VCPU_EFLAGS_OF)

static void cmps_seed(cpu_instruction_fixture *state)
{
    t_cpu *cpu = &state->cpu;

    cpu->data.eax = 0xaabbcc10u; cpu->data.ecx = 0x11220003u;
    cpu->data.edx = 0x778899aau; cpu->data.ebx = 0xbbccdEeu;
    cpu->data.esp = 0x8000u; cpu->data.ebp = 0x120u;
    cpu->data.esi = 0x10u; cpu->data.edi = 0x20u;
    cpu->data.eflags = VCPU_EFLAGS_IF;
    cpu->data.cs.base = 0u; cpu->data.ds.base = 0x20000u;
    cpu->data.es.base = 0x30000u; cpu->data.fs.base = 0x40000u;
}

static lib_i32 cmps_others_same(const t_cpu *before, const t_cpu *after)
{
    return after->data.eax == before->data.eax &&
        after->data.edx == before->data.edx &&
        after->data.ebx == before->data.ebx &&
        after->data.esp == before->data.esp &&
        after->data.ebp == before->data.ebp;
}

static lib_i32 cmps_all_gpr_same(const t_cpu *before, const t_cpu *after)
{
    return cmps_others_same(before, after) &&
        after->data.ecx == before->data.ecx &&
        after->data.esi == before->data.esi &&
        after->data.edi == before->data.edi;
}

static lib_i32 cmps_memory_same(const cpu_instruction_fixture *state,
    lib_u32 source, lib_u32 destination, const void *left,
    const void *right, lib_u8 width)
{
    return lib_memory_compare(state->memory + source, left, width) == 0 &&
        lib_memory_compare(state->memory + destination, right, width) == 0;
}

static lib_i32 cmps_single(core_machine_cpu_profile profile,
    const lib_u8 *code, lib_u8 bytes, lib_u8 width, lib_bool address32,
    lib_bool decrement, lib_u32 source, lib_u32 destination)
{
    cpu_instruction_fixture state;
    t_cpu before, after;
    lib_u32 left = width == 4u ? 0xaabb0010u : 0x10u;
    lib_u32 right = 1u;
    lib_u32 index = address32 ? 0x1020u : 0x20u;
    lib_u32 expected = index + (decrement ? -(lib_i32)width : width);

    cpu_instruction_prepare(&state, profile);
    cmps_seed(&state);
    if (address32) {
        state.cpu.data.esi = 0x1010u;
        state.cpu.data.edi = index;
    }
    if (decrement) state.cpu.data.eflags |= VCPU_EFLAGS_DF;
    lib_memory_copy(state.memory + source, &left, width);
    lib_memory_copy(state.memory + destination, &right, width);
    before = state.cpu;
    if (cpu_instruction_run(&state, code, bytes, &after) != LIB_STATUS_OK ||
        state.fault.valid || after.data.eip != bytes ||
        !cmps_others_same(&before, &after) ||
        after.data.ecx != before.data.ecx ||
        after.data.esi != (address32 ? 0x1010u +
        (decrement ? -(lib_i32)width : width) :
        (before.data.esi & 0xffff0000u) | (lib_u16)(expected - 0x10u)) ||
        after.data.edi != (address32 ? expected :
        (before.data.edi & 0xffff0000u) | (lib_u16)expected) ||
        (after.data.eflags & CMPS_FLAGS) !=
        (VCPU_EFLAGS_PF | VCPU_EFLAGS_AF |
        (width == 4u ? VCPU_EFLAGS_SF : 0u)) ||
        (after.data.eflags & ~CMPS_FLAGS) !=
        (before.data.eflags & ~CMPS_FLAGS)) {
        lib_c_printf("CMPS single code=%02x profile=%u width=%u\n",
            code[0], profile, width);
        return 0;
    }
    return cmps_memory_same(&state, source, destination, &left, &right, width);
}

static lib_i32 cmps_flag_case(lib_u8 left, lib_u8 right, lib_u32 flags)
{
    const lib_u8 code = 0xa6u;
    cpu_instruction_fixture state;
    t_cpu before, after;

    cpu_instruction_prepare(&state, CORE_MACHINE_CPU_PROFILE_80386);
    cmps_seed(&state);
    state.memory[0x20010u] = left;
    state.memory[0x30020u] = right;
    before = state.cpu;
    return cpu_instruction_run(&state, &code, 1u, &after) == LIB_STATUS_OK &&
        !state.fault.valid && after.data.eip == 1u &&
        cmps_others_same(&before, &after) &&
        after.data.ecx == before.data.ecx &&
        after.data.esi == 0x11u && after.data.edi == 0x21u &&
        (after.data.eflags & CMPS_FLAGS) == flags &&
        (after.data.eflags & ~CMPS_FLAGS) ==
        (before.data.eflags & ~CMPS_FLAGS) &&
        cmps_memory_same(&state, 0x20010u, 0x30020u,
            &left, &right, 1u);
}

static lib_i32 cmps_override(const lib_u8 *code, lib_u8 bytes,
    lib_u32 source)
{
    cpu_instruction_fixture state;
    t_cpu before, after;
    const lib_u8 left = 0x10u, right = 1u;

    cpu_instruction_prepare(&state, CORE_MACHINE_CPU_PROFILE_80386);
    cmps_seed(&state);
    state.memory[source] = left;
    state.memory[0x30020u] = right;
    before = state.cpu;
    return cpu_instruction_run(&state, code, bytes, &after) == LIB_STATUS_OK &&
        !state.fault.valid && after.data.eip == bytes &&
        cmps_others_same(&before, &after) &&
        after.data.ecx == before.data.ecx &&
        after.data.esi == 0x11u && after.data.edi == 0x21u &&
        (after.data.eflags & CMPS_FLAGS) ==
        (VCPU_EFLAGS_PF | VCPU_EFLAGS_AF) &&
        cmps_memory_same(&state, source, 0x30020u, &left, &right, 1u);
}

static lib_i32 cmps_rep(core_machine_cpu_profile profile,
    const lib_u8 *code, lib_u8 bytes, lib_u16 count,
    const lib_u8 left[3], const lib_u8 right[3],
    lib_u16 expected_count, lib_u16 expected_si,
    lib_u16 expected_di, lib_u32 flags)
{
    cpu_instruction_fixture state;
    t_cpu before, after;
    lib_u8 step, steps = count == 0u ? 1u :
        (lib_u8)(count - expected_count);

    cpu_instruction_prepare(&state, profile);
    cmps_seed(&state);
    state.cpu.data.ecx = 0x11220000u | count;
    lib_memory_copy(state.memory + 0x20010u, left, 3u);
    lib_memory_copy(state.memory + 0x30020u, right, 3u);
    lib_memory_copy(state.memory, code, bytes);
    before = state.cpu;
    for (step = 0u; step != steps; ++step) {
        core_machine_cpu_execution_refresh(&state.execution);
        if (state.execution.stop_requested) return 0;
    }
    after = state.cpu;
    if (state.fault.valid || after.data.eip != bytes ||
        !cmps_others_same(&before, &after) ||
        after.data.ecx != ((before.data.ecx & 0xffff0000u) | expected_count) ||
        after.data.esi != ((before.data.esi & 0xffff0000u) | expected_si) ||
        after.data.edi != ((before.data.edi & 0xffff0000u) | expected_di) ||
        (after.data.eflags & CMPS_FLAGS) != flags ||
        (after.data.eflags & ~CMPS_FLAGS) !=
        (before.data.eflags & ~CMPS_FLAGS) ||
        !cmps_memory_same(&state, 0x20010u, 0x30020u,
            left, right, 3u)) {
        lib_c_printf("CMPS REP code=%02x profile=%u count=%u\n",
            code[0], profile, count);
        return 0;
    }
    return 1;
}

static lib_i32 cmps_rep_attributes(const lib_u8 *code, lib_u8 bytes,
    lib_u8 width, lib_bool address32, lib_bool decrement)
{
    cpu_instruction_fixture state;
    t_cpu before, after;
    lib_u32 image = width == 4u ? 0x11223344u :
        width == 2u ? 0x3344u : 0x44u;
    lib_u32 source_base = address32 ? 0x21010u : 0x20010u;
    lib_u32 dest_base = address32 ? 0x31020u : 0x30020u;
    lib_u32 source_index = address32 ? 0x1010u : 0x10u;
    lib_u32 dest_index = address32 ? 0x1020u : 0x20u;
    lib_u32 expected_source, expected_dest;
    lib_u8 item;

    cpu_instruction_prepare(&state, CORE_MACHINE_CPU_PROFILE_80386);
    cmps_seed(&state);
    if (decrement) {
        source_index += width * 2u;
        dest_index += width * 2u;
        state.cpu.data.eflags |= VCPU_EFLAGS_DF;
    }
    state.cpu.data.esi = source_index;
    state.cpu.data.edi = dest_index;
    state.cpu.data.ecx = address32 ? 3u : 0x11220003u;
    for (item = 0u; item != 3u; ++item) {
        lib_memory_copy(state.memory + source_base + item * width,
            &image, width);
        lib_memory_copy(state.memory + dest_base + item * width,
            &image, width);
    }
    lib_memory_copy(state.memory, code, bytes);
    before = state.cpu;
    for (item = 0u; item != 3u; ++item) {
        core_machine_cpu_execution_refresh(&state.execution);
        if (state.execution.stop_requested) return 0;
    }
    after = state.cpu;
    expected_source = decrement ? source_index - width * 3u :
        source_index + width * 3u;
    expected_dest = decrement ? dest_index - width * 3u :
        dest_index + width * 3u;
    if (state.fault.valid || after.data.eip != bytes ||
        !cmps_others_same(&before, &after) ||
        after.data.ecx != (address32 ? 0u : 0x11220000u) ||
        (after.data.eflags & CMPS_FLAGS) !=
        (VCPU_EFLAGS_PF | VCPU_EFLAGS_ZF) ||
        (after.data.eflags & ~CMPS_FLAGS) !=
        (before.data.eflags & ~CMPS_FLAGS) ||
        after.data.esi != (address32 ? expected_source :
            (before.data.esi & 0xffff0000u) | (lib_u16)expected_source) ||
        after.data.edi != (address32 ? expected_dest :
            (before.data.edi & 0xffff0000u) | (lib_u16)expected_dest)) return 0;
    for (item = 0u; item != 3u; ++item)
        if (!cmps_memory_same(&state, source_base + item * width,
            dest_base + item * width, &image, &image, width)) return 0;
    return 1;
}

static lib_i32 cmps_expect_ud(core_machine_cpu_profile profile,
    const lib_u8 *code, lib_u8 bytes)
{
    cpu_instruction_fixture state;
    t_cpu before, after;
    const lib_u8 left = 0x10u, right = 1u;

    cpu_instruction_prepare(&state, profile);
    cmps_seed(&state);
    state.memory[0x20010u] = left;
    state.memory[0x30020u] = right;
    state.cpu.data.idtr.limit = 0x17u;
    before = state.cpu;
    (void)cpu_instruction_run(&state, code, bytes, &after);
    return state.fault.valid &&
        (state.fault.exception_mask & VCPUINS_EXCEPT_UD) &&
        after.data.eip == 0u && cmps_all_gpr_same(&before, &after) &&
        after.data.eflags == before.data.eflags &&
        cmps_memory_same(&state, 0x20010u, 0x30020u,
            &left, &right, 1u);
}

static lib_i32 cmps_test_rejections(void)
{
    static const core_machine_cpu_profile legacy[] = {
        CORE_MACHINE_CPU_PROFILE_8086, CORE_MACHINE_CPU_PROFILE_80186,
        CORE_MACHINE_CPU_PROFILE_80286
    };
    static const lib_u8 attr[][4] = {
        {0x66u,0xa6u}, {0x67u,0xa7u}, {0x66u,0x67u,0xa7u},
        {0xf3u,0x66u,0xa6u}, {0xf2u,0x67u,0xa7u},
        {0xf3u,0x66u,0x67u,0xa7u}
    };
    static const lib_u8 attr_bytes[] = {2u, 2u, 3u, 3u, 3u, 4u};
    static const lib_u8 locks[][5] = {
        {0xf0u,0xa6u}, {0xf0u,0xa7u}, {0xf0u,0xf3u,0xa6u},
        {0xf0u,0xf2u,0xa7u}, {0xf0u,0x66u,0xa7u},
        {0xf0u,0x67u,0xa6u}, {0xf0u,0x66u,0x67u,0xa7u},
        {0xf0u,0xf3u,0x66u,0x67u,0xa7u}
    };
    static const lib_u8 lock_bytes[] = {2u, 2u, 3u, 3u, 3u, 3u, 4u, 5u};
    lib_u8 profile, form;

    for (profile = 0u; profile != sizeof(legacy) / sizeof(legacy[0]); ++profile)
        for (form = 0u; form != sizeof(attr_bytes); ++form)
            if (!cmps_expect_ud(legacy[profile], attr[form],
                attr_bytes[form])) return 0;
    for (form = 0u; form != sizeof(lock_bytes); ++form)
        if (!cmps_expect_ud(CORE_MACHINE_CPU_PROFILE_80386,
            locks[form], lock_bytes[form])) return 0;
    return 1;
}

static void cmps_prepare_protected(cpu_instruction_fixture *state,
    lib_bool source_fault, lib_u32 limit)
{
    cpu_instruction_prepare(state, CORE_MACHINE_CPU_PROFILE_80386);
    cmps_seed(state);
    state->cpu.data.cr0 |= VCPU_CR0_PE;
    state->cpu.data.cs.flagValid = LIB_TRUE;
    state->cpu.data.cs.selector = 0x08u;
    state->cpu.data.cs.sregtype = SREG_CODE;
    state->cpu.data.cs.base = 0x2000u;
    state->cpu.data.cs.limit = 0xffffu;
    state->cpu.data.cs.seg.executable = LIB_TRUE;
    state->cpu.data.cs.seg.exec.readable = LIB_TRUE;
    state->cpu.data.ds.flagValid = LIB_TRUE;
    state->cpu.data.ds.selector = 0x10u;
    state->cpu.data.ds.sregtype = SREG_DATA;
    state->cpu.data.ds.base = 0x3000u;
    state->cpu.data.ds.limit = source_fault ? limit : 0xffffu;
    state->cpu.data.ds.seg.data.writable = LIB_TRUE;
    state->cpu.data.es.flagValid = LIB_TRUE;
    state->cpu.data.es.selector = 0x18u;
    state->cpu.data.es.sregtype = SREG_DATA;
    state->cpu.data.es.base = 0x4000u;
    state->cpu.data.es.limit = source_fault ? 0xffffu : limit;
    state->cpu.data.es.seg.data.writable = LIB_TRUE;
    state->cpu.data.ss.flagValid = LIB_TRUE;
    state->cpu.data.ss.selector = 0x20u;
    state->cpu.data.ss.sregtype = SREG_STACK;
    state->cpu.data.ss.base = 0x5000u;
    state->cpu.data.ss.limit = 0xffffu;
    state->cpu.data.ss.seg.data.writable = LIB_TRUE;
}

static lib_i32 cmps_protected_case(lib_bool source_fault,
    lib_bool repeated)
{
    static const lib_u8 single[] = {0xa6u};
    static const lib_u8 rep[] = {0xf3u, 0xa6u};
    const lib_u8 *code = repeated ? rep : single;
    lib_u8 bytes = repeated ? sizeof(rep) : sizeof(single);
    cpu_instruction_fixture state;
    t_cpu before, after;
    const lib_u8 left[] = {0x10u, 1u};
    const lib_u8 right[] = {0x10u, 1u};

    cmps_prepare_protected(&state, source_fault, repeated ? 0x10u : 0x0fu);
    state.cpu.data.esi = repeated || source_fault ? 0x10u : 0u;
    state.cpu.data.edi = repeated || !source_fault ? 0x10u : 0u;
    if (repeated) state.cpu.data.ecx = 0x11220003u;
    lib_memory_copy(state.memory + 0x3010u, left, sizeof(left));
    lib_memory_copy(state.memory + 0x4010u, right, sizeof(right));
    lib_memory_copy(state.memory + 0x2000u, code, bytes);
    before = state.cpu;
    core_machine_cpu_execution_refresh(&state.execution);
    if (repeated && state.execution.stop_requested) return 0;
    if (repeated) core_machine_cpu_execution_refresh(&state.execution);
    after = state.cpu;
    return state.execution.stop_requested && state.fault.valid &&
        (state.fault.exception_mask & VCPUINS_EXCEPT_DF) &&
        after.data.eip == 0u && cmps_others_same(&before, &after) &&
        after.data.ecx == (repeated ? 0x11220002u : before.data.ecx) &&
        after.data.esi == (repeated ? 0x11u : before.data.esi) &&
        after.data.edi == (repeated ? 0x11u : before.data.edi) &&
        after.data.eflags == (repeated ? VCPU_EFLAGS_IF | VCPU_EFLAGS_PF |
            VCPU_EFLAGS_ZF : before.data.eflags) &&
        lib_memory_compare(&before.data.ds, &after.data.ds,
            sizeof(before.data.ds)) == 0 &&
        lib_memory_compare(&before.data.es, &after.data.es,
            sizeof(before.data.es)) == 0 &&
        cmps_memory_same(&state, 0x3010u, 0x4010u,
            left, right, sizeof(left));
}

lib_i32 main(void)
{
    static const core_machine_cpu_profile profiles[] = {
        CORE_MACHINE_CPU_PROFILE_8086, CORE_MACHINE_CPU_PROFILE_80186,
        CORE_MACHINE_CPU_PROFILE_80286, CORE_MACHINE_CPU_PROFILE_80386
    };
    static const lib_u8 a6 = 0xa6u, a7 = 0xa7u;
    static const lib_u8 dword[] = {0x66u, 0xa7u};
    static const lib_u8 address[] = {0x67u, 0xa6u};
    static const lib_u8 both[] = {0x66u, 0x67u, 0xa7u};
    static const lib_u8 cs[] = {0x2eu, 0xa6u}, fs[] = {0x64u, 0xa6u};
    static const lib_u8 repe[] = {0xf3u, 0xa6u};
    static const lib_u8 repne[] = {0xf2u, 0xa6u};
    static const lib_u8 zero[] = {1u, 1u, 1u};
    static const lib_u8 equal[] = {0x10u, 0x10u, 0x10u};
    static const lib_u8 different[] = {0x10u, 1u, 0x10u};
    static const lib_u8 operand32[] = {0xf3u, 0x66u, 0xa7u};
    static const lib_u8 address32[] = {0xf3u, 0x67u, 0xa6u};
    static const lib_u8 combined[] = {0xf3u, 0x66u, 0x67u, 0xa7u};
    lib_u8 profile;

    for (profile = 0u; profile != sizeof(profiles) / sizeof(profiles[0]); ++profile)
        if (!cmps_single(profiles[profile], &a6, 1u, 1u, LIB_FALSE,
            LIB_FALSE, 0x20010u, 0x30020u) ||
            !cmps_single(profiles[profile], &a7, 1u, 2u, LIB_FALSE,
            LIB_FALSE, 0x20010u, 0x30020u) ||
            !cmps_rep(profiles[profile], repe, 2u, 0u, zero, zero,
                0u, 0x10u, 0x20u, 0u) ||
            !cmps_rep(profiles[profile], repe, 2u, 1u, equal, equal,
                0u, 0x11u, 0x21u, VCPU_EFLAGS_PF | VCPU_EFLAGS_ZF) ||
            !cmps_rep(profiles[profile], repe, 2u, 3u, equal, different,
                1u, 0x12u, 0x22u, VCPU_EFLAGS_PF | VCPU_EFLAGS_AF) ||
            !cmps_rep(profiles[profile], repne, 2u, 0u, zero, zero,
                0u, 0x10u, 0x20u, 0u) ||
            !cmps_rep(profiles[profile], repne, 2u, 1u, different, zero,
                0u, 0x11u, 0x21u, VCPU_EFLAGS_PF | VCPU_EFLAGS_AF) ||
            !cmps_rep(profiles[profile], repne, 2u, 3u, different, equal,
                2u, 0x11u, 0x21u, VCPU_EFLAGS_PF | VCPU_EFLAGS_ZF)) {
            lib_c_printf("CMPS stage=profile %u\n", profile);
            return 1;
        }
    if (!cmps_single(CORE_MACHINE_CPU_PROFILE_80386, dword, 2u, 4u,
            LIB_FALSE, LIB_FALSE, 0x20010u, 0x30020u) ||
        !cmps_single(CORE_MACHINE_CPU_PROFILE_80386, address, 2u, 1u,
            LIB_TRUE, LIB_FALSE, 0x21010u, 0x31020u) ||
        !cmps_single(CORE_MACHINE_CPU_PROFILE_80386, both, 3u, 4u,
            LIB_TRUE, LIB_FALSE, 0x21010u, 0x31020u) ||
        !cmps_single(CORE_MACHINE_CPU_PROFILE_80386, &a7, 1u, 2u,
            LIB_FALSE, LIB_TRUE, 0x20010u, 0x30020u) ||
        !cmps_override(cs, 2u, 0x10u) ||
        !cmps_override(fs, 2u, 0x40010u) ||
        !cmps_rep_attributes(operand32, 3u, 4u, LIB_FALSE, LIB_FALSE) ||
        !cmps_rep_attributes(address32, 3u, 1u, LIB_TRUE, LIB_FALSE) ||
        !cmps_rep_attributes(combined, 4u, 4u, LIB_TRUE, LIB_FALSE) ||
        !cmps_rep_attributes(repe, 2u, 1u, LIB_FALSE, LIB_TRUE)) {
        lib_c_printf("CMPS stage=attributes\n");
        return 1;
    }
    if (!cmps_flag_case(0x10u, 0x01u, VCPU_EFLAGS_PF | VCPU_EFLAGS_AF) ||
        !cmps_flag_case(0x10u, 0x10u, VCPU_EFLAGS_PF | VCPU_EFLAGS_ZF) ||
        !cmps_flag_case(0x00u, 0x01u, VCPU_EFLAGS_CF | VCPU_EFLAGS_PF |
            VCPU_EFLAGS_AF | VCPU_EFLAGS_SF) ||
        !cmps_flag_case(0x80u, 0x01u, VCPU_EFLAGS_AF | VCPU_EFLAGS_OF) ||
        !cmps_test_rejections() ||
        !cmps_protected_case(LIB_TRUE, LIB_FALSE) ||
        !cmps_protected_case(LIB_FALSE, LIB_FALSE) ||
        !cmps_protected_case(LIB_TRUE, LIB_TRUE) ||
        !cmps_protected_case(LIB_FALSE, LIB_TRUE)) {
        lib_c_printf("CMPS stage=flags/rejections/protected\n");
        return 1;
    }
    lib_c_printf("M5:T316:S37:CMPS:OK\n");
    lib_c_printf("M5:T401:S16:CMPS-PROFILES:OK\n");
    return 0;
}
