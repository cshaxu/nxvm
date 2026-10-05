#include "support/cpu_instruction_fixture.h"
#include "lib/types/file.h"
/* T337_REAL_UD_TERMINAL_CPU_OWNER: MOVS prefix rejection is CPU-owned. */

static void movs_seed(cpu_instruction_fixture *state)
{
    t_cpu *cpu = &state->cpu;

    cpu->data.eax = 0xaabb3344u;
    cpu->data.ecx = 0x11225566u;
    cpu->data.edx = 0x778899aau;
    cpu->data.ebx = 0xbbccddeeU;
    cpu->data.esp = 0x8000u;
    cpu->data.ebp = 0x120u;
    cpu->data.esi = 0x10u;
    cpu->data.edi = 0x20u;
    cpu->data.eflags = VCPU_EFLAGS_CF | VCPU_EFLAGS_ZF;
    cpu->data.ds.base = 0x10000u;
    cpu->data.es.base = 0x20000u;
    cpu->data.ss.base = 0x30000u;
    cpu->data.fs.base = 0x40000u;
    cpu->data.gs.base = 0x50000u;
}

static lib_bool movs_nonindexes_same(const t_cpu *before, const t_cpu *after,
    lib_bool count_changes)
{
    return before->data.eax == after->data.eax &&
        (count_changes || before->data.ecx == after->data.ecx) &&
        before->data.edx == after->data.edx &&
        before->data.ebx == after->data.ebx &&
        before->data.esp == after->data.esp &&
        before->data.ebp == after->data.ebp &&
        before->data.eflags == after->data.eflags;
}

static lib_i32 movs_single(core_machine_cpu_profile profile,
    const lib_u8 *code, lib_u8 bytes, lib_u8 width, lib_bool address32,
    lib_bool decrement, lib_u32 source_address, lib_u32 destination_address,
    lib_u32 source)
{
    cpu_instruction_fixture state;
    t_cpu before, after;
    lib_u32 source_after = 0u;
    lib_u32 destination = 0xa5a5a5a5u, destination_after = 0u;
    lib_u32 offset = address32 ? 0x1000u : 0u;

    cpu_instruction_prepare(&state, profile);
    movs_seed(&state);
    state.cpu.data.esi += offset;
    state.cpu.data.edi += offset;
    if (decrement) state.cpu.data.eflags |= VCPU_EFLAGS_DF;
    lib_memory_copy(state.memory + source_address, &source, width);
    lib_memory_copy(state.memory + destination_address, &destination, width);
    before = state.cpu;
    if (cpu_instruction_run(&state, code, bytes, &after) != LIB_STATUS_OK ||
        state.fault.valid || after.data.eip != bytes ||
        !movs_nonindexes_same(&before, &after, LIB_FALSE) ||
        after.data.esi != before.data.esi +
            (decrement ? -(lib_i32)width : width) ||
        after.data.edi != before.data.edi +
            (decrement ? -(lib_i32)width : width)) return 0;
    lib_memory_copy(&source_after, state.memory + source_address, width);
    lib_memory_copy(&destination_after, state.memory + destination_address, width);
    return source_after == (source & (width == 1u ? 0xffu :
            width == 2u ? 0xffffu : 0xffffffffu)) &&
        destination_after == (source & (width == 1u ? 0xffu :
            width == 2u ? 0xffffu : 0xffffffffu));
}

static lib_i32 movs_test_single(void)
{
    static const core_machine_cpu_profile profiles[] = {
        CORE_MACHINE_CPU_PROFILE_8086, CORE_MACHINE_CPU_PROFILE_80186,
        CORE_MACHINE_CPU_PROFILE_80286, CORE_MACHINE_CPU_PROFILE_80386
    };
    static const lib_u8 movsb = 0xa4u, movsw = 0xa5u;
    static const lib_u8 dword[] = {0x66u, 0xa5u};
    static const lib_u8 addr[] = {0x67u, 0xa4u};
    static const lib_u8 both[] = {0x66u, 0x67u, 0xa5u};
    static const lib_u8 cs[] = {0x2eu, 0xa4u};
    static const lib_u8 ss[] = {0x36u, 0xa4u};
    static const lib_u8 fs[] = {0x64u, 0xa4u};
    static const lib_u8 gs[] = {0x65u, 0xa4u};
    static const lib_u8 es[] = {0x26u, 0xa4u};
    lib_u8 profile;

    for (profile = 0u; profile != sizeof(profiles) / sizeof(profiles[0]); ++profile)
        if (!movs_single(profiles[profile], &movsb, 1u, 1u, LIB_FALSE,
            LIB_FALSE, 0x10010u, 0x20020u, 0x12345678u) ||
            !movs_single(profiles[profile], &movsw, 1u, 2u, LIB_FALSE,
            LIB_FALSE, 0x10010u, 0x20020u, 0x12345678u)) return 0;
    return movs_single(CORE_MACHINE_CPU_PROFILE_80386, dword, 2u, 4u,
            LIB_FALSE, LIB_FALSE, 0x10010u, 0x20020u, 0x12345678u) &&
        movs_single(CORE_MACHINE_CPU_PROFILE_80386, addr, 2u, 1u,
            LIB_TRUE, LIB_FALSE, 0x11010u, 0x21020u, 0x12345678u) &&
        movs_single(CORE_MACHINE_CPU_PROFILE_80386, both, 3u, 4u,
            LIB_TRUE, LIB_FALSE, 0x11010u, 0x21020u, 0x12345678u) &&
        movs_single(CORE_MACHINE_CPU_PROFILE_80386, cs, 2u, 1u,
            LIB_FALSE, LIB_FALSE, 0x10u, 0x20020u, 0x1100u) &&
        movs_single(CORE_MACHINE_CPU_PROFILE_80386, ss, 2u, 1u,
            LIB_FALSE, LIB_FALSE, 0x30010u, 0x20020u, 0x1101u) &&
        movs_single(CORE_MACHINE_CPU_PROFILE_80386, fs, 2u, 1u,
            LIB_FALSE, LIB_FALSE, 0x40010u, 0x20020u, 0x1102u) &&
        movs_single(CORE_MACHINE_CPU_PROFILE_80386, gs, 2u, 1u,
            LIB_FALSE, LIB_FALSE, 0x50010u, 0x20020u, 0x1103u) &&
        movs_single(CORE_MACHINE_CPU_PROFILE_80386, es, 2u, 1u,
            LIB_FALSE, LIB_FALSE, 0x20010u, 0x20020u, 0x1104u) &&
        movs_single(CORE_MACHINE_CPU_PROFILE_80386, &movsw, 1u, 2u,
            LIB_FALSE, LIB_TRUE, 0x10010u, 0x20020u, 0x1105u);
}

static lib_i32 movs_rep(core_machine_cpu_profile profile, const lib_u8 *code,
    lib_u8 bytes, lib_u8 width, lib_bool address32, lib_u8 count,
    lib_bool decrement)
{
    cpu_instruction_fixture state;
    t_cpu before, after;
    static const lib_u32 source[] = {0x11223344u, 0x55667788u, 0x99aabbccu};
    lib_u32 source_index = address32 ? 0x1010u : 0x10u;
    lib_u32 destination_index = address32 ? 0x1020u : 0x20u;
    lib_u32 destination = 0xa5a5a5a5u;
    lib_u8 slots = count == 0u ? 1u : count;
    lib_u8 index;

    cpu_instruction_prepare(&state, profile);
    movs_seed(&state);
    if (decrement) {
        source_index += (count - 1u) * width;
        destination_index += (count - 1u) * width;
        state.cpu.data.eflags |= VCPU_EFLAGS_DF;
    }
    state.cpu.data.esi = source_index;
    state.cpu.data.edi = destination_index;
    state.cpu.data.ecx = address32 ? count : 0x11220000u | count;
    for (index = 0u; index != slots; ++index) {
        lib_u32 step = (decrement ? count - 1u - index : index) * width;
        lib_u32 source_address = 0x10000u + source_index -
            (decrement ? (count - 1u) * width : 0u) + step;
        lib_u32 destination_address = 0x20000u + destination_index -
            (decrement ? (count - 1u) * width : 0u) + step;
        lib_memory_copy(state.memory + source_address, &source[index], width);
        lib_memory_copy(state.memory + destination_address, &destination, width);
    }
    lib_memory_copy(state.memory, code, bytes);
    before = state.cpu;
    for (index = 0u; index != slots; ++index) {
        core_machine_cpu_execution_refresh(&state.execution);
        if (state.execution.stop_requested) {
            lib_c_printf("MOVS rep stop profile=%u code=%02x count=%u step=%u fault=%u\n",
                profile, code[0], count, index, state.fault.valid);
            return 0;
        }
    }
    after = state.cpu;
    if (state.fault.valid || after.data.eip != bytes ||
        !movs_nonindexes_same(&before, &after, LIB_TRUE) ||
        after.data.ecx != (address32 ? 0u : before.data.ecx & 0xffff0000u) ||
        after.data.esi != (address32 ? source_index +
            (decrement ? -(lib_i32)(count * width) : count * width) :
            (lib_u16)(source_index + (decrement ?
                -(lib_i32)(count * width) : count * width))) ||
        after.data.edi != (address32 ? destination_index +
            (decrement ? -(lib_i32)(count * width) : count * width) :
            (lib_u16)(destination_index + (decrement ?
                -(lib_i32)(count * width) : count * width)))) {
        lib_c_printf("MOVS rep state profile=%u code=%02x count=%u eip=%u ecx=%08x esi=%08x edi=%08x\n",
            profile, code[0], count, after.data.eip, after.data.ecx,
            after.data.esi, after.data.edi);
        return 0;
    }
    for (index = 0u; index != slots; ++index) {
        lib_u32 step = index * width;
        lib_u32 source_address = 0x10000u + source_index -
            (decrement ? (count - 1u) * width : 0u) + step;
        lib_u32 destination_address = 0x20000u + destination_index -
            (decrement ? (count - 1u) * width : 0u) + step;
        lib_u8 element = decrement && count != 0u ? count - 1u - index : index;
        lib_u32 source_after = 0u, destination_after = 0u;

        lib_memory_copy(&source_after, state.memory + source_address, width);
        lib_memory_copy(&destination_after, state.memory + destination_address,
            width);
        if (source_after != (source[element] & (width == 1u ? 0xffu :
                width == 2u ? 0xffffu : 0xffffffffu)) ||
            destination_after != (count == 0u ?
                (destination & (width == 1u ? 0xffu :
                    width == 2u ? 0xffffu : 0xffffffffu)) :
                (source[element] & (width == 1u ? 0xffu :
                    width == 2u ? 0xffffu : 0xffffffffu)))) {
            lib_c_printf("MOVS rep memory profile=%u code=%02x count=%u slot=%u source=%08x destination=%08x\n",
                profile, code[0], count, index, source_after, destination_after);
            return 0;
        }
    }
    return 1;
}

static lib_i32 movs_test_rep(void)
{
    static const core_machine_cpu_profile profiles[] = {
        CORE_MACHINE_CPU_PROFILE_8086, CORE_MACHINE_CPU_PROFILE_80186,
        CORE_MACHINE_CPU_PROFILE_80286, CORE_MACHINE_CPU_PROFILE_80386
    };
    static const lib_u8 rep_movsb[] = {0xf3u, 0xa4u};
    static const lib_u8 rep_movsw[] = {0xf3u, 0xa5u};
    static const lib_u8 rep_movsd[] = {0x66u, 0xf3u, 0xa5u};
    static const lib_u8 rep_addr32_movsb[] = {0xf3u, 0x67u, 0xa4u};
    static const lib_u8 rep_addr32_movsd[] = {0xf3u, 0x66u, 0x67u, 0xa5u};
    lib_u8 profile;

    for (profile = 0u; profile != sizeof(profiles) / sizeof(profiles[0]); ++profile)
        if (!movs_rep(profiles[profile], rep_movsb, 2u, 1u, LIB_FALSE, 0u,
            LIB_FALSE) || !movs_rep(profiles[profile], rep_movsb, 2u, 1u,
            LIB_FALSE, 1u, LIB_FALSE) || !movs_rep(profiles[profile],
            rep_movsb, 2u, 1u, LIB_FALSE, 3u, LIB_FALSE) ||
            !movs_rep(profiles[profile], rep_movsw, 2u, 2u, LIB_FALSE,
            3u, LIB_FALSE)) return 0;
    return movs_rep(CORE_MACHINE_CPU_PROFILE_80386, rep_movsd, 3u, 4u,
            LIB_FALSE, 3u, LIB_FALSE) &&
        movs_rep(CORE_MACHINE_CPU_PROFILE_80386, rep_addr32_movsb, 3u, 1u,
            LIB_TRUE, 3u, LIB_FALSE) &&
        movs_rep(CORE_MACHINE_CPU_PROFILE_80386, rep_addr32_movsd, 4u, 4u,
            LIB_TRUE, 3u, LIB_FALSE) &&
        movs_rep(CORE_MACHINE_CPU_PROFILE_80386, rep_movsw, 2u, 2u,
            LIB_FALSE, 3u, LIB_TRUE);
}

static lib_i32 movs_rejection(core_machine_cpu_profile profile,
    const lib_u8 *code, lib_u8 bytes)
{
    cpu_instruction_fixture state;
    t_cpu before, after;
    lib_u16 source = 0x3344u;
    lib_u16 destination = 0xa5a5u;

    cpu_instruction_prepare(&state, profile);
    movs_seed(&state);
    state.cpu.data.ecx = 3u;
    state.cpu.data.idtr.limit = 0x17u;
    lib_memory_copy(state.memory + 0x10010u, &source, sizeof(source));
    lib_memory_copy(state.memory + 0x20020u, &destination,
        sizeof(destination));
    before = state.cpu;
    if (cpu_instruction_run(&state, code, bytes, &after) !=
            LIB_STATUS_INTERNAL_ERROR || !state.fault.valid ||
        !(state.fault.exception_mask & VCPUINS_EXCEPT_UD) ||
        after.data.eip != 0u ||
        !movs_nonindexes_same(&before, &after, LIB_FALSE) ||
        after.data.esi != before.data.esi ||
        after.data.edi != before.data.edi) return 0;
    lib_memory_copy(&source, state.memory + 0x10010u, sizeof(source));
    lib_memory_copy(&destination, state.memory + 0x20020u,
        sizeof(destination));
    return source == 0x3344u && destination == 0xa5a5u;
}

static lib_i32 movs_test_rejections(void)
{
    static const core_machine_cpu_profile profiles[] = {
        CORE_MACHINE_CPU_PROFILE_8086, CORE_MACHINE_CPU_PROFILE_80186,
        CORE_MACHINE_CPU_PROFILE_80286
    };
    static const lib_u8 prefixes[][3] = {
        {0x66u, 0xa4u}, {0x67u, 0xa5u}, {0x66u, 0x67u, 0xa5u}
    };
    static const lib_u8 prefix_bytes[] = {2u, 2u, 3u};
    static const lib_u8 locks[][3] = {
        {0xf0u, 0xa4u}, {0xf0u, 0xf3u, 0xa5u}
    };
    static const lib_u8 lock_bytes[] = {2u, 3u};
    lib_u8 profile, form;

    for (profile = 0u; profile != sizeof(profiles) / sizeof(profiles[0]); ++profile)
        for (form = 0u; form != sizeof(prefix_bytes); ++form)
            if (!movs_rejection(profiles[profile], prefixes[form],
                prefix_bytes[form])) return 0;
    for (form = 0u; form != sizeof(lock_bytes); ++form)
        if (!movs_rejection(CORE_MACHINE_CPU_PROFILE_80386,
            locks[form], lock_bytes[form])) return 0;
    return 1;
}

static lib_i32 movs_test_protected_limits(void)
{
    static const lib_u8 codes[][2] = {{0xa4u, 0u}, {0x66u, 0xa5u}};
    lib_u8 form;

    for (form = 0u; form != 2u; ++form) {
        cpu_instruction_fixture state;
        t_cpu before, after;
        lib_u32 source = 0x11223344u;
        lib_u32 destination = 0xa5a5a5a5u;
        lib_u32 source_address = form == 0u ? 0x3010u : 0x4010u;
        lib_u32 destination_address = form == 0u ? 0x3020u : 0x3010u;
        lib_u8 width = form == 0u ? 1u : 4u;

        cpu_instruction_prepare(&state, CORE_MACHINE_CPU_PROFILE_80386);
        movs_seed(&state);
        state.cpu.data.cr0 |= VCPU_CR0_PE;
        state.cpu.data.cs.flagValid = LIB_TRUE;
        state.cpu.data.cs.selector = 0x08u;
        state.cpu.data.cs.sregtype = SREG_CODE;
        state.cpu.data.cs.base = 0x2000u;
        state.cpu.data.cs.limit = 0xffffu;
        state.cpu.data.cs.seg.executable = LIB_TRUE;
        state.cpu.data.cs.seg.exec.readable = LIB_TRUE;
        state.cpu.data.ds.flagValid = LIB_TRUE;
        state.cpu.data.ds.selector = form == 0u ? 0x10u : 0x18u;
        state.cpu.data.ds.sregtype = SREG_DATA;
        state.cpu.data.ds.base = form == 0u ? 0x3000u : 0x4000u;
        state.cpu.data.ds.limit = form == 0u ? 0x0fu : 0xffffu;
        state.cpu.data.ds.seg.data.writable = LIB_TRUE;
        state.cpu.data.es.flagValid = LIB_TRUE;
        state.cpu.data.es.selector = 0x10u;
        state.cpu.data.es.sregtype = SREG_DATA;
        state.cpu.data.es.base = 0x3000u;
        state.cpu.data.es.limit = 0x0fu;
        state.cpu.data.es.seg.data.writable = LIB_TRUE;
        state.cpu.data.ss.flagValid = LIB_TRUE;
        state.cpu.data.ss.selector = 0x18u;
        state.cpu.data.ss.sregtype = SREG_STACK;
        state.cpu.data.ss.base = 0x4000u;
        state.cpu.data.ss.limit = 0xffffu;
        state.cpu.data.ss.seg.data.writable = LIB_TRUE;
        lib_memory_copy(state.memory + source_address, &source, width);
        lib_memory_copy(state.memory + destination_address, &destination, width);
        lib_memory_copy(state.memory + 0x2000u, codes[form],
            form == 0u ? 1u : 2u);
        before = state.cpu;
        core_machine_cpu_execution_refresh(&state.execution);
        after = state.cpu;
        if (!state.execution.stop_requested || !state.fault.valid ||
            !(state.fault.exception_mask & VCPUINS_EXCEPT_DF) ||
            after.data.eip != 0u ||
            !movs_nonindexes_same(&before, &after, LIB_FALSE) ||
            after.data.esi != before.data.esi ||
            after.data.edi != before.data.edi ||
            lib_memory_compare(&before.data.ds, &after.data.ds,
                sizeof(before.data.ds)) != 0 ||
            lib_memory_compare(&before.data.es, &after.data.es,
                sizeof(before.data.es)) != 0 ||
            lib_memory_compare(state.memory + source_address, &source,
                width) != 0 ||
            lib_memory_compare(state.memory + destination_address,
                &destination, width) != 0) return 0;
    }
    return 1;
}

lib_i32 main(void)
{
    if (!movs_test_single()) {
        lib_c_printf("MOVS stage=single\n");
        return 1;
    }
    if (!movs_test_rep()) {
        lib_c_printf("MOVS stage=rep\n");
        return 1;
    }
    if (!movs_test_rejections()) {
        lib_c_printf("MOVS stage=rejections\n");
        return 1;
    }
    if (!movs_test_protected_limits()) {
        lib_c_printf("MOVS stage=protected\n");
        return 1;
    }
    lib_c_printf("M5:T316:S33:MOVS:OK\n");
    lib_c_printf("M5:T401:S15:MOVS-PROFILES:OK\n");
    return 0;
}
