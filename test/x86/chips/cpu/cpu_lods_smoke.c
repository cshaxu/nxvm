#include "support/cpu_instruction_fixture.h"
#include "lib/types/file.h"
/* T337_REAL_UD_TERMINAL_CPU_OWNER: LODS prefix rejection is CPU-owned. */

static void lods_seed(cpu_instruction_fixture *state)
{
    t_cpu *cpu = &state->cpu;
    cpu->data.eax = 0xaabb3344u; cpu->data.ecx = 0x11225566u;
    cpu->data.edx = 0x778899aau; cpu->data.ebx = 0xbbccdEeu;
    cpu->data.esp = 0x8000u; cpu->data.ebp = 0x120u;
    cpu->data.esi = 0x10u; cpu->data.edi = 0x20u;
    cpu->data.eflags = VCPU_EFLAGS_CF | VCPU_EFLAGS_ZF;
    cpu->data.ds.base = 0x10000u; cpu->data.cs.base = 0u;
    cpu->data.ss.base = 0x30000u; cpu->data.es.base = 0x20000u;
    cpu->data.fs.base = 0x40000u; cpu->data.gs.base = 0x50000u;
}

static lib_i32 lods_case(core_machine_cpu_profile profile, const lib_u8 *code,
    lib_u8 bytes, lib_u8 width, lib_bool address32, lib_bool decrement,
    lib_u32 address, lib_u32 source)
{
    cpu_instruction_fixture state;
    t_cpu before, after;
    lib_u32 source_after = 0u;
    lib_u32 expected = width == 1u ? 0xaabb3300u | (source & 0xffu) :
        width == 2u ? 0xaabb0000u | (source & 0xffffu) : source;
    lib_i32 failed;

    cpu_instruction_prepare(&state, profile);
    lods_seed(&state);
    if (address32) state.cpu.data.esi = 0x1010u;
    if (decrement) state.cpu.data.eflags |= VCPU_EFLAGS_DF;
    lib_memory_copy(state.memory + address, &source, width);
    before = state.cpu;
    failed = cpu_instruction_run(&state, code, bytes, &after) != LIB_STATUS_OK ||
        state.fault.valid || after.data.eip != bytes ||
        after.data.eax != expected || after.data.ecx != before.data.ecx ||
        after.data.edx != before.data.edx || after.data.ebx != before.data.ebx ||
        after.data.esp != before.data.esp || after.data.ebp != before.data.ebp ||
        after.data.edi != before.data.edi || after.data.eflags != before.data.eflags ||
        after.data.esi != (address32 ? before.data.esi +
        (decrement ? -(lib_i32)width : width) :
        ((before.data.esi & 0xffff0000u) | (lib_u16)((lib_u16)before.data.esi +
        (decrement ? -(lib_i32)width : width))));
    lib_memory_copy(&source_after, state.memory + address, width);
    if (failed) lib_c_printf("LODS case code=%02x width=%u address=%u eip=%u fault=%u\n",
        code[0], width, address, after.data.eip, state.fault.valid);
    return !failed && (width == 1u ? (source_after & 0xffu) == (source & 0xffu) :
        width == 2u ? (source_after & 0xffffu) == (source & 0xffffu) :
        source_after == source);
}

static lib_i32 lods_rep_case(core_machine_cpu_profile profile,
    const lib_u8 *code, lib_u8 bytes, lib_u8 width, lib_bool address32,
    lib_bool decrement, lib_u16 count, const lib_u32 *source)
{
    cpu_instruction_fixture state;
    t_cpu before, after;
    lib_u32 address, expected_eax, expected_esi, expected_ecx, source_after;
    lib_u16 index;
    lib_i32 failed = 0;

    cpu_instruction_prepare(&state, profile);
    lods_seed(&state);
    state.cpu.data.ecx = address32 ? count : 0x11220000u | count;
    state.cpu.data.esi = address32 ? 0x1010u : 0x10u;
    if (decrement) state.cpu.data.eflags |= VCPU_EFLAGS_DF;
    for (index = 0u; index != count; ++index) {
        address = 0x10000u + (address32 ? 0x1010u : 0x10u) +
            (decrement ? -(lib_i32)(index * width) : index * width);
        lib_memory_copy(state.memory + address, &source[index], width);
    }
    lib_memory_copy(state.memory, code, bytes);
    before = state.cpu;
    for (index = 0u; index != (count == 0u ? 1u : count); ++index) {
        core_machine_cpu_execution_refresh(&state.execution);
        if (state.execution.stop_requested) return 0;
    }
    after = state.cpu;
    expected_eax = count == 0u ? before.data.eax : width == 1u ?
        (before.data.eax & 0xffffff00u) | (source[count - 1u] & 0xffu) :
        width == 2u ? (before.data.eax & 0xffff0000u) |
        (source[count - 1u] & 0xffffu) : source[count - 1u];
    expected_esi = address32 ? before.data.esi + (decrement ?
        -(lib_i32)(count * width) : count * width) :
        (before.data.esi & 0xffff0000u) | (lib_u16)((lib_u16)
        before.data.esi + (decrement ? -(lib_i32)(count * width) : count * width));
    expected_ecx = address32 ? 0u : before.data.ecx & 0xffff0000u;
    failed |= state.fault.valid || after.data.eip != bytes ||
        after.data.eax != expected_eax || after.data.ecx != expected_ecx ||
        after.data.esi != expected_esi || after.data.edx != before.data.edx ||
        after.data.ebx != before.data.ebx || after.data.esp != before.data.esp ||
        after.data.ebp != before.data.ebp || after.data.edi != before.data.edi ||
        after.data.eflags != before.data.eflags;
    for (index = 0u; index != count; ++index) {
        address = 0x10000u + (address32 ? 0x1010u : 0x10u) +
            (decrement ? -(lib_i32)(index * width) : index * width);
        source_after = 0u;
        lib_memory_copy(&source_after, state.memory + address, width);
        failed |= width == 1u ? (source_after & 0xffu) !=
            (source[index] & 0xffu) : width == 2u ?
            (source_after & 0xffffu) != (source[index] & 0xffffu) :
            source_after != source[index];
    }
    if (failed) lib_c_printf("LODS rep code=%02x width=%u count=%u eip=%u fault=%u\n",
        code[0], width, count, after.data.eip, state.fault.valid);
    return !failed;
}

static lib_i32 lods_expect_ud(core_machine_cpu_profile profile,
    const lib_u8 *code, lib_u8 bytes)
{
    static const lib_u32 source = 0x12345678u;
    cpu_instruction_fixture state;
    t_cpu before, after;
    lib_u32 source_after;

    cpu_instruction_prepare(&state, profile);
    lods_seed(&state);
    lib_memory_copy(state.memory + 0x10010u, &source, sizeof(source));
    state.cpu.data.idtr.limit = 0x17u;
    before = state.cpu;
    (void)cpu_instruction_run(&state, code, bytes, &after);
    lib_memory_copy(&source_after, state.memory + 0x10010u, sizeof(source));
    if (!state.fault.valid) lib_c_printf("LODS UD missing code=%02x profile=%u\n", code[0], profile);
    return state.fault.valid &&
        (state.fault.exception_mask & VCPUINS_EXCEPT_UD) != 0u &&
        after.data.eip == 0u && after.data.eax == before.data.eax &&
        after.data.ecx == before.data.ecx && after.data.edx == before.data.edx &&
        after.data.ebx == before.data.ebx && after.data.esp == before.data.esp &&
        after.data.ebp == before.data.ebp && after.data.esi == before.data.esi &&
        after.data.edi == before.data.edi && after.data.eflags == before.data.eflags &&
        source_after == source;
}

static lib_i32 lods_test_rejections(void)
{
    static const core_machine_cpu_profile profiles[] = {
        CORE_MACHINE_CPU_PROFILE_8086, CORE_MACHINE_CPU_PROFILE_80186,
        CORE_MACHINE_CPU_PROFILE_80286
    };
    static const lib_u8 prefixes[][4] = {
        {0x66u, 0xacu}, {0x67u, 0xadu}, {0x66u, 0x67u, 0xadu},
        {0xf3u, 0x66u, 0xacu}, {0xf3u, 0x67u, 0xadu},
        {0xf3u, 0x66u, 0x67u, 0xadu}
    };
    static const lib_u8 prefix_bytes[] = {2u, 2u, 3u, 3u, 3u, 4u};
    static const lib_u8 locks[][5] = {
        {0xf0u, 0xacu}, {0xf0u, 0xadu},
        {0xf0u, 0xf3u, 0xacu}, {0xf0u, 0xf3u, 0xadu},
        {0xf0u, 0x66u, 0xacu}, {0xf0u, 0xf3u, 0x66u, 0xadu},
        {0xf0u, 0x67u, 0xacu}, {0xf0u, 0xf3u, 0x67u, 0xadu},
        {0xf0u, 0x66u, 0x67u, 0xadu},
        {0xf0u, 0xf3u, 0x66u, 0x67u, 0xadu}
    };
    static const lib_u8 lock_bytes[] = {2u, 2u, 3u, 3u, 3u, 4u, 3u, 4u, 4u, 5u};
    lib_u8 profile, form;

    for (profile = 0u; profile != sizeof(profiles) / sizeof(profiles[0]); ++profile)
        for (form = 0u; form != sizeof(prefix_bytes); ++form)
            if (!lods_expect_ud(profiles[profile], prefixes[form], prefix_bytes[form]))
                return 0;
    for (form = 0u; form != sizeof(lock_bytes); ++form)
        if (!lods_expect_ud(CORE_MACHINE_CPU_PROFILE_80386,
            locks[form], lock_bytes[form])) return 0;
    return 1;
}

static void lods_prepare_protected(cpu_instruction_fixture *state,
    lib_u32 ds_limit)
{
    cpu_instruction_prepare(state, CORE_MACHINE_CPU_PROFILE_80386);
    lods_seed(state);
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
    state->cpu.data.ds.limit = ds_limit;
    state->cpu.data.ds.seg.data.writable = LIB_TRUE;
    state->cpu.data.ss.flagValid = LIB_TRUE;
    state->cpu.data.ss.selector = 0x18u;
    state->cpu.data.ss.sregtype = SREG_STACK;
    state->cpu.data.ss.base = 0x4000u;
    state->cpu.data.ss.limit = 0xffffu;
    state->cpu.data.ss.seg.data.writable = LIB_TRUE;
}

static lib_i32 lods_test_protected_limits(void)
{
    static const lib_u8 single_codes[][2] = {{0xacu, 0u}, {0x66u, 0xadu}};
    static const lib_u8 rep_ac[] = {0xf3u, 0xacu};
    lib_u8 form;

    for (form = 0u; form != 2u; ++form) {
        cpu_instruction_fixture state;
        t_cpu before, after;
        lib_u32 source = 0x11223344u;
        lib_u8 width = form == 0u ? 1u : 4u;

        lods_prepare_protected(&state, 0x0fu);
        lib_memory_copy(state.memory + 0x3010u, &source, width);
        lib_memory_copy(state.memory + 0x2000u, single_codes[form],
            form == 0u ? 1u : 2u);
        before = state.cpu;
        core_machine_cpu_execution_refresh(&state.execution);
        after = state.cpu;
        if (!state.execution.stop_requested || !state.fault.valid ||
            !(state.fault.exception_mask & VCPUINS_EXCEPT_DF) ||
            after.data.eip != 0u || after.data.eax != before.data.eax ||
            after.data.ecx != before.data.ecx ||
            after.data.edx != before.data.edx ||
            after.data.ebx != before.data.ebx ||
            after.data.esp != before.data.esp ||
            after.data.ebp != before.data.ebp ||
            after.data.esi != before.data.esi ||
            after.data.edi != before.data.edi ||
            after.data.eflags != before.data.eflags ||
            lib_memory_compare(&before.data.ds, &after.data.ds,
                sizeof(before.data.ds)) != 0 ||
            lib_memory_compare(state.memory + 0x3010u, &source, width) != 0)
            return 0;
    }
    {
        cpu_instruction_fixture state;
        t_cpu before, after;
        const lib_u8 first = 0x51u, second = 0x62u;

        lods_prepare_protected(&state, 0x10u);
        state.cpu.data.ecx = 0x11220003u;
        state.memory[0x3010u] = first;
        state.memory[0x3011u] = second;
        lib_memory_copy(state.memory + 0x2000u, rep_ac, sizeof(rep_ac));
        before = state.cpu;
        core_machine_cpu_execution_refresh(&state.execution);
        if (state.execution.stop_requested) return 0;
        core_machine_cpu_execution_refresh(&state.execution);
        after = state.cpu;
        if (!state.execution.stop_requested || !state.fault.valid ||
            !(state.fault.exception_mask & VCPUINS_EXCEPT_DF) ||
            after.data.eip != 0u || after.data.eax != 0xaabb3351u ||
            after.data.ecx != 0x11220002u || after.data.esi != 0x11u ||
            after.data.edx != before.data.edx ||
            after.data.ebx != before.data.ebx ||
            after.data.esp != before.data.esp ||
            after.data.ebp != before.data.ebp ||
            after.data.edi != before.data.edi ||
            after.data.eflags != before.data.eflags ||
            lib_memory_compare(&before.data.ds, &after.data.ds,
                sizeof(before.data.ds)) != 0 ||
            state.memory[0x3010u] != first || state.memory[0x3011u] != second)
            return 0;
    }
    return 1;
}

lib_i32 main(void)
{
    static const core_machine_cpu_profile profiles[] = {
        CORE_MACHINE_CPU_PROFILE_8086, CORE_MACHINE_CPU_PROFILE_80186,
        CORE_MACHINE_CPU_PROFILE_80286, CORE_MACHINE_CPU_PROFILE_80386
    };
    static const lib_u8 ac = 0xacu, ad = 0xadu;
    static const lib_u8 dword[] = {0x66u, 0xadu}, addr[] = {0x67u, 0xacu};
    static const lib_u8 both[] = {0x66u, 0x67u, 0xadu};
    static const lib_u8 cs[] = {0x2eu, 0xacu}, ss[] = {0x36u, 0xacu};
    static const lib_u8 es[] = {0x26u, 0xacu}, fs[] = {0x64u, 0xacu};
    static const lib_u8 gs[] = {0x65u, 0xacu};
    static const lib_u8 rep_ac[] = {0xf3u, 0xacu};
    static const lib_u8 rep_ad[] = {0xf3u, 0xadu};
    static const lib_u8 rep_66_ad[] = {0xf3u, 0x66u, 0xadu};
    static const lib_u8 rep_67_ac[] = {0xf3u, 0x67u, 0xacu};
    static const lib_u8 rep_66_67_ad[] = {0xf3u, 0x66u, 0x67u, 0xadu};
    static const lib_u32 byte_source[] = {0x51u, 0x62u, 0x73u};
    static const lib_u32 word_source[] = {0x1151u, 0x2262u, 0x3373u};
    static const lib_u32 dword_source[] = {0x10203040u, 0x50607080u, 0x90a0b0c0u};
    lib_u8 profile;

    for (profile = 0u; profile != sizeof(profiles) / sizeof(profiles[0]); ++profile)
        if (!lods_case(profiles[profile], &ac, 1u, 1u, LIB_FALSE, LIB_FALSE,
            0x10010u, 0x12345678u) || !lods_case(profiles[profile], &ad,
            1u, 2u, LIB_FALSE, LIB_FALSE, 0x10010u, 0x12345678u) ||
            !lods_rep_case(profiles[profile], rep_ac, 2u, 1u, LIB_FALSE,
            LIB_FALSE, 0u, byte_source) || !lods_rep_case(profiles[profile],
            rep_ac, 2u, 1u, LIB_FALSE, LIB_FALSE, 1u, byte_source) ||
            !lods_rep_case(profiles[profile], rep_ad, 2u, 2u, LIB_FALSE,
            LIB_FALSE, 3u, word_source)) {
            lib_c_printf("LODS stage=profile profile=%u\n", profile);
            return 1;
        }
    if (!lods_case(CORE_MACHINE_CPU_PROFILE_80386, dword, 2u, 4u, LIB_FALSE,
        LIB_FALSE, 0x10010u, 0x12345678u) ||
        !lods_case(CORE_MACHINE_CPU_PROFILE_80386, addr, 2u, 1u, LIB_TRUE,
        LIB_FALSE, 0x11010u, 0x12345678u) ||
        !lods_case(CORE_MACHINE_CPU_PROFILE_80386, both, 3u, 4u, LIB_TRUE,
        LIB_FALSE, 0x11010u, 0x12345678u) ||
        !lods_case(CORE_MACHINE_CPU_PROFILE_80386, &ad, 1u, 2u, LIB_FALSE,
        LIB_TRUE, 0x10010u, 0x12345678u) ||
        !lods_case(CORE_MACHINE_CPU_PROFILE_80386, cs, 2u, 1u, LIB_FALSE,
        LIB_FALSE, 0x10u, 0x51u) ||
        !lods_case(CORE_MACHINE_CPU_PROFILE_80386, ss, 2u, 1u, LIB_FALSE,
        LIB_FALSE, 0x30010u, 0x52u) ||
        !lods_case(CORE_MACHINE_CPU_PROFILE_80386, es, 2u, 1u, LIB_FALSE,
        LIB_FALSE, 0x20010u, 0x53u) ||
        !lods_case(CORE_MACHINE_CPU_PROFILE_80386, fs, 2u, 1u, LIB_FALSE,
        LIB_FALSE, 0x40010u, 0x54u) ||
        !lods_case(CORE_MACHINE_CPU_PROFILE_80386, gs, 2u, 1u, LIB_FALSE,
        LIB_FALSE, 0x50010u, 0x55u) ||
        !lods_rep_case(CORE_MACHINE_CPU_PROFILE_80386, rep_66_ad, 3u, 4u,
        LIB_FALSE, LIB_FALSE, 3u, dword_source) ||
        !lods_rep_case(CORE_MACHINE_CPU_PROFILE_80386, rep_67_ac, 3u, 1u,
        LIB_TRUE, LIB_FALSE, 3u, byte_source) ||
        !lods_rep_case(CORE_MACHINE_CPU_PROFILE_80386, rep_66_67_ad, 4u, 4u,
        LIB_TRUE, LIB_FALSE, 3u, dword_source) ||
        !lods_rep_case(CORE_MACHINE_CPU_PROFILE_80386, rep_ac, 2u, 1u,
        LIB_FALSE, LIB_TRUE, 3u, byte_source) || !lods_test_rejections() ||
        !lods_test_protected_limits()) {
        lib_c_printf("LODS stage=extended\n");
        return 1;
    }
    lib_c_printf("M5:T316:S35:LODS:OK\n");
    lib_c_printf("M5:T401:S18:LODS-PROFILES:OK\n");
    return 0;
}
