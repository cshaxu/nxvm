#include "support/cpu_instruction_fixture.h"
/* T337_REAL_UD_TERMINAL_CPU_OWNER: unsupported IMUL encodings are CPU-owned. */
#include "lib/types/file.h"

static void imul_seed(t_cpu *cpu)
{
    cpu->data.eax = 0xa1a10000u;
    cpu->data.ecx = 0xb2b2fffeu;
    cpu->data.edx = 0xc3c30000u;
    cpu->data.ebx = 0xd4d40000u;
    cpu->data.esp = 0x00008000u;
    cpu->data.ebp = 0xe5e50000u;
    cpu->data.esi = 0x00004000u;
    cpu->data.edi = 0xf6f60000u;
    cpu->data.eflags = VCPU_EFLAGS_IF | VCPU_EFLAGS_DF | VCPU_EFLAGS_CF;
}

static lib_i32 imul_sregs_same(const t_cpu *before, const t_cpu *after)
{
    return lib_memory_compare(&before->data.es, &after->data.es,
            sizeof(before->data.es)) == 0 &&
        lib_memory_compare(&before->data.cs, &after->data.cs,
            sizeof(before->data.cs)) == 0 &&
        lib_memory_compare(&before->data.ss, &after->data.ss,
            sizeof(before->data.ss)) == 0 &&
        lib_memory_compare(&before->data.ds, &after->data.ds,
            sizeof(before->data.ds)) == 0 &&
        lib_memory_compare(&before->data.fs, &after->data.fs,
            sizeof(before->data.fs)) == 0 &&
        lib_memory_compare(&before->data.gs, &after->data.gs,
            sizeof(before->data.gs)) == 0;
}

static lib_i32 imul_nonparticipants_same(const t_cpu *before,
    const t_cpu *after)
{
    return before->data.ecx == after->data.ecx &&
        before->data.edx == after->data.edx &&
        before->data.ebx == after->data.ebx &&
        before->data.esp == after->data.esp &&
        before->data.ebp == after->data.ebp &&
        before->data.esi == after->data.esi &&
        before->data.edi == after->data.edi;
}

static lib_i32 imul_nonarithmetic_flags_same(const t_cpu *before,
    const t_cpu *after)
{
    const lib_u32 arithmetic = VCPU_EFLAGS_CF | VCPU_EFLAGS_OF |
        VCPU_EFLAGS_SF | VCPU_EFLAGS_ZF | VCPU_EFLAGS_AF | VCPU_EFLAGS_PF;
    return (before->data.eflags & ~arithmetic) ==
        (after->data.eflags & ~arithmetic);
}

static lib_i32 imul_run(core_machine_cpu_profile profile, const lib_u8 *code,
    lib_u8 bytes, lib_u32 source, lib_u32 expected, lib_i32 overflow,
    lib_i32 dword)
{
    cpu_instruction_fixture state;
    t_cpu before, after;

    cpu_instruction_prepare(&state, profile);
    imul_seed(&state.cpu);
    state.cpu.data.ecx = source;
    if (code[1] == 0xc0u) state.cpu.data.eax = source;
    before = state.cpu;
    if (cpu_instruction_run(&state, code, bytes, &after) != LIB_STATUS_OK ||
        state.fault.valid || after.data.eip != bytes ||
        after.data.ecx != before.data.ecx ||
        after.data.edx != before.data.edx ||
        after.data.ebx != before.data.ebx ||
        after.data.ebp != before.data.ebp ||
        after.data.esi != before.data.esi ||
        after.data.edi != before.data.edi ||
        !imul_sregs_same(&before, &after) ||
        (dword ? after.data.eax != expected :
            after.data.eax != ((before.data.eax & 0xffff0000u) |
                (expected & 0xffffu))) ||
        !!X86_CPU_BIT_IS_SET(after.data.eflags, VCPU_EFLAGS_CF) != overflow ||
        !!X86_CPU_BIT_IS_SET(after.data.eflags, VCPU_EFLAGS_OF) != overflow ||
        !imul_nonarithmetic_flags_same(&before, &after)) return 0;
    return 1;
}

/* T337_REAL_UD_TERMINAL_CPU_OWNER: rejection is CPU-owned. */
static lib_i32 imul_expect_ud(core_machine_cpu_profile profile,
    const lib_u8 *code, lib_u8 bytes)
{
    cpu_instruction_fixture state;
    t_cpu before, after;

    cpu_instruction_prepare(&state, profile);
    imul_seed(&state.cpu);
    state.cpu.data.idtr.limit = 0x17u;
    before = state.cpu;
    return cpu_instruction_run(&state, code, bytes, &after) ==
            LIB_STATUS_INTERNAL_ERROR && state.fault.valid &&
        X86_CPU_BIT_IS_SET(state.fault.exception_mask, VCPUINS_EXCEPT_UD) &&
        lib_memory_compare(&before, &after, sizeof(before)) == 0;
}

static lib_i32 imul_test_defaults(void)
{
    static const lib_u8 iw[] = {0x69u, 0xc1u, 0xfeu, 0xffu};
    static const lib_u8 ib[] = {0x6bu, 0xc1u, 0xfeu};
    static const lib_u8 alias_iw[] = {0x69u, 0xc0u, 0xfeu, 0xffu};
    static const lib_u8 alias_ib[] = {0x6bu, 0xc0u, 0xfeu};
    static const lib_u8 overflow[] = {0x69u, 0xc1u, 0x00u, 0x40u};
    static const core_machine_cpu_profile profiles[] = {
        CORE_MACHINE_CPU_PROFILE_80186, CORE_MACHINE_CPU_PROFILE_80286,
        CORE_MACHINE_CPU_PROFILE_80386
    };
    lib_u8 profile;

    for (profile = 0u; profile < sizeof(profiles) / sizeof(profiles[0]);
            ++profile) {
        if (!imul_run(profiles[profile], iw, sizeof(iw), 0xb2b2fffeu,
                4u, 0, 0) ||
            !imul_run(profiles[profile], ib, sizeof(ib), 0xb2b2fffeu,
                4u, 0, 0) ||
            !imul_run(profiles[profile], alias_iw, sizeof(alias_iw),
                0xa1a10003u, 0xa1a1fffau, 0, 0) ||
            !imul_run(profiles[profile], alias_ib, sizeof(alias_ib),
                0xa1a10003u, 0xa1a1fffau, 0, 0) ||
            !imul_run(profiles[profile], overflow, sizeof(overflow),
                0xb2b20002u, 0x8000u, 1, 0)) return 0;
    }
    return 1;
}

static lib_i32 imul_test_attributes_and_rejects(void)
{
    static const lib_u8 dword_iw[] = {
        0x66u, 0x69u, 0xc1u, 0x00u, 0x00u, 0x00u, 0x40u
    };
    static const lib_u8 dword_ib[] = {0x66u, 0x6bu, 0xc1u, 0xfeu};
    static const lib_u8 inert[] = {0x67u, 0x6bu, 0xc1u, 0xfeu};
    static const lib_u8 combined[] = {
        0x66u, 0x67u, 0x69u, 0xc1u, 0xfeu, 0xffu, 0xffu, 0xffu
    };
    static const lib_u8 lock[][9] = {
        {0xf0u, 0x69u, 0xc1u, 0xfeu, 0xffu},
        {0xf0u, 0x6bu, 0xc1u, 0xfeu},
        {0xf0u, 0x66u, 0x69u, 0xc1u, 0xfeu, 0xffu, 0xffu, 0xffu},
        {0xf0u, 0x66u, 0x6bu, 0xc1u, 0xfeu},
        {0xf0u, 0x67u, 0x69u, 0xc1u, 0xfeu, 0xffu},
        {0xf0u, 0x67u, 0x6bu, 0xc1u, 0xfeu},
        {0xf0u, 0x66u, 0x67u, 0x69u, 0xc1u, 0xfeu, 0xffu, 0xffu, 0xffu},
        {0xf0u, 0x66u, 0x67u, 0x6bu, 0xc1u, 0xfeu}
    };
    static const lib_u8 lock_bytes[] = {5u, 4u, 8u, 5u, 6u, 5u, 9u, 6u};
    static const core_machine_cpu_profile pre386[] = {
        CORE_MACHINE_CPU_PROFILE_8086, CORE_MACHINE_CPU_PROFILE_80186,
        CORE_MACHINE_CPU_PROFILE_80286
    };
    lib_u8 index;

    if (!imul_run(CORE_MACHINE_CPU_PROFILE_80386, dword_iw,
            sizeof(dword_iw), 2u, 0x80000000u, 1, 1) ||
        !imul_run(CORE_MACHINE_CPU_PROFILE_80386, dword_ib,
            sizeof(dword_ib), 0xfffffffeu, 4u, 0, 1) ||
        !imul_run(CORE_MACHINE_CPU_PROFILE_80386, inert,
            sizeof(inert), 0xb2b2fffeu, 4u, 0, 0) ||
        !imul_run(CORE_MACHINE_CPU_PROFILE_80386, combined,
            sizeof(combined), 0xfffffffeu, 4u, 0, 1)) return 0;
    for (index = 0u; index < sizeof(lock_bytes); ++index)
        if (!imul_expect_ud(CORE_MACHINE_CPU_PROFILE_80386, lock[index],
                lock_bytes[index])) return 0;
    for (index = 0u; index < sizeof(pre386) / sizeof(pre386[0]); ++index)
        if (!imul_expect_ud(pre386[index], dword_iw, sizeof(dword_iw)) ||
            !imul_expect_ud(pre386[index], inert, sizeof(inert)) ||
            !imul_expect_ud(pre386[index], combined, sizeof(combined)))
            return 0;
    return 1;
}

static lib_i32 imul_test_memory_forms(void)
{
    static const lib_u8 word_iw[] = {
        0x69u, 0x06u, 0x00u, 0x40u, 0xfeu, 0xffu
    };
    static const lib_u8 word_ib[] = {
        0x6bu, 0x06u, 0x00u, 0x40u, 0xfeu
    };
    static const lib_u8 dword_iw[] = {
        0x66u, 0x67u, 0x69u, 0x05u, 0x00u, 0x40u, 0x00u, 0x00u,
        0xfeu, 0xffu, 0xffu, 0xffu
    };
    static const lib_u8 dword_ib[] = {
        0x66u, 0x67u, 0x6bu, 0x05u, 0x00u, 0x40u, 0x00u, 0x00u, 0xfeu
    };
    const lib_u32 dword_source = 0xfffffffeu;
    const lib_u16 word_source = 0xfffeu;
    const lib_u8 *codes[] = {word_iw, word_ib, dword_iw, dword_ib};
    const lib_u8 sizes[] = {
        sizeof(word_iw), sizeof(word_ib), sizeof(dword_iw), sizeof(dword_ib)
    };
    static const core_machine_cpu_profile word_profiles[] = {
        CORE_MACHINE_CPU_PROFILE_80186, CORE_MACHINE_CPU_PROFILE_80286,
        CORE_MACHINE_CPU_PROFILE_80386
    };
    lib_u8 form;

    for (form = 0u; form < sizeof(codes) / sizeof(codes[0]); ++form) {
        const lib_u8 profile_count = form < 2u ?
            sizeof(word_profiles) / sizeof(word_profiles[0]) : 1u;
        lib_u8 profile_index;

        for (profile_index = 0u; profile_index < profile_count;
                ++profile_index) {
            cpu_instruction_fixture state;
            t_cpu before, after;
            lib_u32 source_after = 0u;
            const lib_i32 dword = form >= 2u;

            cpu_instruction_prepare(&state, form < 2u ?
                word_profiles[profile_index] : CORE_MACHINE_CPU_PROFILE_80386);
            imul_seed(&state.cpu);
            before = state.cpu;
            if (cpu_instruction_write(&state, 0x4000u,
                    dword ? (const void *)&dword_source :
                        (const void *)&word_source,
                    dword ? 4u : 2u, CORE_MACHINE_CPU_MEMORY_ACCESS_DATA) !=
                    LIB_STATUS_OK ||
                cpu_instruction_run(&state, codes[form], sizes[form], &after) !=
                    LIB_STATUS_OK || state.fault.valid ||
                after.data.eip != sizes[form] ||
                (dword ? after.data.eax != 4u :
                    after.data.eax != ((before.data.eax & 0xffff0000u) | 4u)) ||
                !imul_nonparticipants_same(&before, &after) ||
                !imul_sregs_same(&before, &after) ||
                X86_CPU_BIT_IS_SET(after.data.eflags, VCPU_EFLAGS_CF) ||
                X86_CPU_BIT_IS_SET(after.data.eflags, VCPU_EFLAGS_OF) ||
                !imul_nonarithmetic_flags_same(&before, &after) ||
                cpu_instruction_read(&state, 0x4000u, &source_after,
                    dword ? 4u : 2u, CORE_MACHINE_CPU_MEMORY_ACCESS_DATA,
                    LIB_FALSE, LIB_FALSE) != LIB_STATUS_OK ||
                (dword ? source_after != dword_source :
                    (lib_u16)source_after != word_source)) return 0;
        }
    }
    return 1;
}

static lib_i32 imul_test_segments(void)
{
    static const lib_u8 codes[][7] = {
        {0x69u, 0x06u, 0x10u, 0x00u, 0xfeu, 0xffu, 0u},
        {0x69u, 0x46u, 0x00u, 0xfeu, 0xffu, 0u, 0u},
        {0x2eu, 0x69u, 0x06u, 0x10u, 0x00u, 0xfeu, 0xffu},
        {0x26u, 0x69u, 0x06u, 0x10u, 0x00u, 0xfeu, 0xffu},
        {0x64u, 0x69u, 0x06u, 0x10u, 0x00u, 0xfeu, 0xffu},
        {0x65u, 0x69u, 0x06u, 0x10u, 0x00u, 0xfeu, 0xffu}
    };
    static const lib_u8 bytes[] = {6u, 5u, 7u, 7u, 7u, 7u};
    lib_u8 form;

    for (form = 0u; form < sizeof(bytes); ++form) {
        cpu_instruction_fixture state;
        t_cpu before, after;
        t_cpu_data_sreg *segment;
        const lib_u16 selector = (lib_u16)(0x1000u + form * 0x1000u);
        const lib_u32 address = ((lib_u32)selector << 4u) + 0x10u;
        lib_u32 code_address = 0u;
        const lib_u16 source = 0xfffeu;

        cpu_instruction_prepare(&state, CORE_MACHINE_CPU_PROFILE_80386);
        imul_seed(&state.cpu);
        segment = &state.cpu.data.ds;
        if (form == 1u) segment = &state.cpu.data.ss;
        else if (form == 2u) segment = &state.cpu.data.cs;
        else if (form == 3u) segment = &state.cpu.data.es;
        else if (form == 4u) segment = &state.cpu.data.fs;
        else if (form == 5u) segment = &state.cpu.data.gs;
        segment->selector = selector;
        segment->base = (lib_u32)selector << 4u;
        if (form == 2u) code_address = segment->base;
        if (form == 1u) state.cpu.data.ebp = 0x00000010u;
        before = state.cpu;
        if (cpu_instruction_write(&state, code_address, codes[form],
                bytes[form], CORE_MACHINE_CPU_MEMORY_ACCESS_DATA) !=
                LIB_STATUS_OK ||
            cpu_instruction_write(&state, address, &source, sizeof(source),
                CORE_MACHINE_CPU_MEMORY_ACCESS_DATA) != LIB_STATUS_OK)
            return 0;
        core_machine_cpu_execution_refresh(&state.execution);
        after = state.cpu;
        if (state.execution.stop_requested || state.fault.valid ||
            after.data.eip != bytes[form] ||
            after.data.eax != ((before.data.eax & 0xffff0000u) | 4u) ||
            !imul_nonparticipants_same(&before, &after) ||
            !imul_sregs_same(&before, &after) ||
            !imul_nonarithmetic_flags_same(&before, &after)) return 0;
    }
    return 1;
}

static lib_i32 imul_test_67_sib_ss(void)
{
    static const lib_u8 code[] = {
        0x67u, 0x69u, 0x44u, 0x24u, 0x10u, 0xfeu, 0xffu
    };
    const lib_u16 source = 0xfffeu;
    cpu_instruction_fixture state;
    t_cpu before, after;

    cpu_instruction_prepare(&state, CORE_MACHINE_CPU_PROFILE_80386);
    imul_seed(&state.cpu);
    state.cpu.data.ss.selector = 0x2000u;
    state.cpu.data.ss.base = 0x20000u;
    state.cpu.data.esp = 0u;
    before = state.cpu;
    return cpu_instruction_write(&state, 0x20010u, &source, sizeof(source),
            CORE_MACHINE_CPU_MEMORY_ACCESS_DATA) == LIB_STATUS_OK &&
        cpu_instruction_run(&state, code, sizeof(code), &after) == LIB_STATUS_OK &&
        !state.fault.valid && after.data.eip == sizeof(code) &&
        after.data.eax == ((before.data.eax & 0xffff0000u) | 4u) &&
        imul_nonparticipants_same(&before, &after) &&
        imul_sregs_same(&before, &after);
}

static lib_i32 imul_test_vm86(void)
{
    static const lib_u8 code[] = {0x69u, 0xc1u, 0xfeu, 0xffu};
    cpu_instruction_fixture state;
    t_cpu before, after;

    cpu_instruction_prepare(&state, CORE_MACHINE_CPU_PROFILE_80386);
    imul_seed(&state.cpu);
    state.cpu.data.cr0 |= VCPU_CR0_PE;
    state.cpu.data.eflags = VCPU_EFLAGS_VM | VCPU_EFLAGS_IOPL | VCPU_EFLAGS_IF;
    state.cpu.data.cs.selector = 0u;
    state.cpu.data.cs.base = 0u;
    state.cpu.data.cs.limit = 0xffffu;
    state.cpu.data.cs.dpl = 3u;
    state.cpu.data.cs.flagValid = LIB_TRUE;
    state.cpu.data.cs.seg.exec.defsize = LIB_FALSE;
    state.cpu.data.ds.selector = 0u;
    state.cpu.data.ds.base = 0u;
    state.cpu.data.ds.limit = 0xffffu;
    state.cpu.data.ds.dpl = 3u;
    state.cpu.data.ds.flagValid = LIB_TRUE;
    state.cpu.data.es.selector = 0u;
    state.cpu.data.es.base = 0u;
    state.cpu.data.es.limit = 0xffffu;
    state.cpu.data.es.dpl = 3u;
    state.cpu.data.es.flagValid = LIB_TRUE;
    state.cpu.data.ss.selector = 0u;
    state.cpu.data.ss.base = 0u;
    state.cpu.data.ss.limit = 0xffffu;
    state.cpu.data.ss.dpl = 3u;
    state.cpu.data.ss.flagValid = LIB_TRUE;
    state.cpu.data.ss.seg.data.big = LIB_FALSE;
    before = state.cpu;
    return cpu_instruction_run(&state, code, sizeof(code), &after) ==
            LIB_STATUS_OK && !state.fault.valid &&
        after.data.eip == sizeof(code) &&
        after.data.eax == ((before.data.eax & 0xffff0000u) | 4u) &&
        imul_sregs_same(&before, &after) &&
        imul_nonparticipants_same(&before, &after);
}

static lib_i32 imul_test_synthetic_ss_limit(void)
{
    static const lib_u8 code[] = {0x69u, 0x46u, 0u, 0xfeu, 0xffu};
    cpu_instruction_fixture state;
    t_cpu before, after;

    cpu_instruction_prepare(&state, CORE_MACHINE_CPU_PROFILE_80386);
    imul_seed(&state.cpu);
    state.cpu.data.cr0 |= VCPU_CR0_PE;
    state.cpu.data.ebp = 0x10u;
    state.cpu.data.ss.limit = 0x0fu;
    before = state.cpu;
    return cpu_instruction_run(&state, code, sizeof(code), &after) ==
            LIB_STATUS_INTERNAL_ERROR && state.fault.valid &&
        X86_CPU_BIT_IS_SET(state.fault.exception_mask, VCPUINS_EXCEPT_DF) &&
        after.data.eip == 0u && after.data.eax == before.data.eax &&
        imul_nonparticipants_same(&before, &after) &&
        after.data.eflags == before.data.eflags &&
        imul_sregs_same(&before, &after);
}

static lib_bool imul_test_dword_byte_extremes(void)
{
    static const lib_u32 sources[] = {
        0u, 1u, 0xffffffffu, 0x40000000u, 0x7fffffffu, 0x80000000u
    };
    static const lib_i8 multipliers[] = {-128, -1, 0, 1, 4, 127};
    lib_u8 source_index, multiplier_index, memory, alias;

    for (source_index = 0u; source_index < sizeof(sources) / sizeof(sources[0]); ++source_index)
    for (multiplier_index = 0u; multiplier_index < sizeof(multipliers); ++multiplier_index)
    for (memory = 0u; memory < 2u; ++memory)
    for (alias = 0u; alias < (memory ? 1u : 2u); ++alias) {
        const lib_u32 source = sources[source_index];
        const lib_i64 signed_source = source & 0x80000000u ?
            (lib_i64)source - INT64_C(4294967296) : (lib_i64)source;
        const lib_i64 product = signed_source * multipliers[multiplier_index];
        const lib_u32 expected = (lib_u32)product;
        const lib_bool overflow = product < -INT64_C(2147483648) ||
            product > INT64_C(2147483647);
        lib_u8 code[8] = {0x66u, 0x6bu};
        lib_u8 bytes = 2u;
        cpu_instruction_fixture state;
        t_cpu before, after;
        lib_u32 source_after = 0u;

        code[bytes++] = memory ? 0x06u : alias ? 0xc0u : 0xc1u;
        if (memory) { code[bytes++] = 0u; code[bytes++] = 0x40u; }
        code[bytes++] = (lib_u8)multipliers[multiplier_index];
        cpu_instruction_prepare(&state, CORE_MACHINE_CPU_PROFILE_80386);
        imul_seed(&state.cpu);
        state.cpu.data.ecx = source;
        if (alias) state.cpu.data.eax = source;
        before = state.cpu;
        if (memory && cpu_instruction_write(&state, 0x4000u, &source,
                4u, CORE_MACHINE_CPU_MEMORY_ACCESS_DATA) != LIB_STATUS_OK)
            return LIB_FALSE;
        if (cpu_instruction_run(&state, code, bytes, &after) != LIB_STATUS_OK ||
                state.fault.valid || after.data.eip != bytes ||
                after.data.eax != expected ||
                !!(after.data.eflags & VCPU_EFLAGS_CF) != overflow ||
                !!(after.data.eflags & VCPU_EFLAGS_OF) != overflow ||
                !imul_nonparticipants_same(&before, &after) ||
                !imul_nonarithmetic_flags_same(&before, &after) ||
                !imul_sregs_same(&before, &after)) return LIB_FALSE;
        if (memory && (cpu_instruction_read(&state, 0x4000u, &source_after,
                4u, CORE_MACHINE_CPU_MEMORY_ACCESS_DATA, LIB_FALSE, LIB_FALSE) !=
                    LIB_STATUS_OK || source_after != source)) return LIB_FALSE;
    }
    return LIB_TRUE;
}

lib_i32 main(void)
{
    static const lib_u8 iw[] = {0x69u, 0xc1u, 0xfeu, 0xffu};
    static const lib_u8 ib[] = {0x6bu, 0xc1u, 0xfeu};

    if (!imul_expect_ud(CORE_MACHINE_CPU_PROFILE_8086, iw, sizeof(iw)) ||
        !imul_expect_ud(CORE_MACHINE_CPU_PROFILE_8086, ib, sizeof(ib)) ||
        !imul_test_defaults() || !imul_test_attributes_and_rejects() ||
        !imul_test_memory_forms() || !imul_test_segments() ||
        !imul_test_67_sib_ss() || !imul_test_vm86() ||
        !imul_test_synthetic_ss_limit() || !imul_test_dword_byte_extremes()) return 1;
    lib_c_printf("M5:T539:S31:CPU-IMUL-IMMEDIATE:OK\n");
    return 0;
}
