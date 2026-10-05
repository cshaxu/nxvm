#include "support/cpu_instruction_fixture.h"
#include "lib/types/file.h"
/* T337_REAL_UD_TERMINAL_CPU_OWNER: STOS prefix rejection is CPU-owned. */

static void stos_seed(cpu_instruction_fixture *state)
{
    t_cpu *cpu = &state->cpu;

    cpu->data.eax = 0xaabb3344u; cpu->data.ecx = 0x11225566u;
    cpu->data.edx = 0x778899aau; cpu->data.ebx = 0xbbccdEeu;
    cpu->data.esp = 0x8000u; cpu->data.ebp = 0x120u;
    cpu->data.esi = 0x10u; cpu->data.edi = 0x20u;
    cpu->data.eflags = VCPU_EFLAGS_CF | VCPU_EFLAGS_ZF;
    cpu->data.cs.base = 0u; cpu->data.es.base = 0x20000u;
    cpu->data.fs.base = 0x40000u;
}

static lib_i32 stos_others_same(const t_cpu *before, const t_cpu *after)
{
    return before->data.eax == after->data.eax &&
        before->data.ecx == after->data.ecx &&
        before->data.edx == after->data.edx &&
        before->data.ebx == after->data.ebx &&
        before->data.esp == after->data.esp &&
        before->data.ebp == after->data.ebp &&
        before->data.esi == after->data.esi &&
        before->data.eflags == after->data.eflags;
}

static lib_u32 stos_written(lib_u32 initial, lib_u8 width)
{
    if (width == 1u) return (initial & 0xffffff00u) | 0x44u;
    if (width == 2u) return (initial & 0xffff0000u) | 0x3344u;
    return 0xaabb3344u;
}

static lib_i32 stos_case(core_machine_cpu_profile profile, const lib_u8 *code,
    lib_u8 bytes, lib_u8 width, lib_bool address32, lib_bool decrement)
{
    cpu_instruction_fixture state;
    t_cpu before, after;
    lib_u32 image = 0xa5a5a5a5u;
    lib_u32 index = address32 ? 0x1020u : 0x20u;
    lib_u32 destination = 0x20000u + index;
    lib_i32 failed;

    cpu_instruction_prepare(&state, profile);
    stos_seed(&state);
    if (address32) state.cpu.data.edi = index;
    if (decrement) state.cpu.data.eflags |= VCPU_EFLAGS_DF;
    lib_memory_copy(state.memory + destination, &image, width);
    before = state.cpu;
    failed = cpu_instruction_run(&state, code, bytes, &after) != LIB_STATUS_OK ||
        state.fault.valid || after.data.eip != bytes ||
        !stos_others_same(&before, &after) ||
        after.data.edi != (address32 ? index + (decrement ?
        -(lib_i32)width : width) : ((before.data.edi & 0xffff0000u) |
        (lib_u16)(index + (decrement ? -(lib_i32)width : width))));
    lib_memory_copy(&image, state.memory + destination, width);
    if (failed) lib_c_printf("STOS case code=%02x width=%u fault=%u\n",
        code[0], width, state.fault.valid);
    return !failed && image == stos_written(0xa5a5a5a5u, width);
}

static lib_i32 stos_rep_case(core_machine_cpu_profile profile,
    const lib_u8 *code, lib_u8 bytes, lib_u8 width, lib_bool address32,
    lib_u8 count, lib_bool decrement)
{
    cpu_instruction_fixture state;
    t_cpu before, after;
    lib_u32 index = address32 ? 0x1020u : 0x20u;
    lib_u32 image, location;
    lib_u8 item, slots = count == 0u ? 1u : count;
    lib_i32 failed = 0;

    cpu_instruction_prepare(&state, profile);
    stos_seed(&state);
    if (decrement) {
        index += (count - 1u) * width;
        state.cpu.data.eflags |= VCPU_EFLAGS_DF;
    }
    state.cpu.data.edi = address32 ? index :
        (state.cpu.data.edi & 0xffff0000u) | index;
    state.cpu.data.ecx = address32 ? count :
        (state.cpu.data.ecx & 0xffff0000u) | count;
    for (item = 0u; item != slots; ++item) {
        image = 0xa5a5a5a5u;
        location = 0x20000u + index - (decrement ?
            (count - 1u) * width : 0u) + item * width;
        lib_memory_copy(state.memory + location, &image, width);
    }
    lib_memory_copy(state.memory, code, bytes);
    before = state.cpu;
    for (item = 0u; item != (count == 0u ? 1u : count); ++item) {
        core_machine_cpu_execution_refresh(&state.execution);
        if (state.execution.stop_requested) return 0;
    }
    after = state.cpu;
    failed |= state.fault.valid || after.data.eip != bytes ||
        after.data.eax != before.data.eax ||
        after.data.edx != before.data.edx ||
        after.data.ebx != before.data.ebx ||
        after.data.esp != before.data.esp ||
        after.data.ebp != before.data.ebp ||
        after.data.esi != before.data.esi ||
        after.data.eflags != before.data.eflags ||
        after.data.ecx != (address32 ? 0u : before.data.ecx & 0xffff0000u) ||
        after.data.edi != (address32 ? index + (decrement ?
        -(lib_i32)(count * width) : count * width) :
        ((before.data.edi & 0xffff0000u) | (lib_u16)(index +
        (decrement ? -(lib_i32)(count * width) : count * width))));
    for (item = 0u; item != slots; ++item) {
        location = 0x20000u + index - (decrement ?
            (count - 1u) * width : 0u) + item * width;
        image = 0xa5a5a5a5u;
        lib_memory_copy(&image, state.memory + location, width);
        failed |= image != (count == 0u ? 0xa5a5a5a5u :
            stos_written(0xa5a5a5a5u, width));
    }
    if (failed) lib_c_printf("STOS REP code=%02x width=%u count=%u\n",
        code[0], width, count);
    return !failed;
}

static lib_i32 stos_expect_ud(core_machine_cpu_profile profile,
    const lib_u8 *code, lib_u8 bytes)
{
    cpu_instruction_fixture state;
    t_cpu before, after;
    lib_u16 image = 0xa5a5u;

    cpu_instruction_prepare(&state, profile);
    stos_seed(&state);
    lib_memory_copy(state.memory + 0x20020u, &image, sizeof(image));
    state.cpu.data.idtr.limit = 0x17u;
    before = state.cpu;
    (void)cpu_instruction_run(&state, code, bytes, &after);
    if (!state.fault.valid ||
        !(state.fault.exception_mask & VCPUINS_EXCEPT_UD) ||
        after.data.eip != 0u || !stos_others_same(&before, &after) ||
        after.data.edi != before.data.edi) {
        lib_c_printf("STOS UD profile=%u code=%02x fault=%u mask=%u eip=%u\n",
            profile, code[0], state.fault.valid,
            state.fault.exception_mask, after.data.eip);
        return 0;
    }
    lib_memory_copy(&image, state.memory + 0x20020u, sizeof(image));
    return image == 0xa5a5u;
}

static lib_i32 stos_test_rejections(void)
{
    static const lib_u8 codes[][5] = {
        {0x66u,0xaau}, {0x67u,0xabu}, {0x66u,0x67u,0xabu},
        {0xf0u,0xaau}, {0xf0u,0xf3u,0xabu}, {0xf0u,0x66u,0xabu},
        {0xf0u,0x67u,0xaau}, {0xf0u,0x66u,0x67u,0xabu},
        {0xf0u,0xf3u,0x66u,0xabu}, {0xf0u,0xf3u,0x67u,0xaau},
        {0xf0u,0xf3u,0x66u,0x67u,0xabu}
    };
    static const lib_u8 bytes[] = {2u,2u,3u,2u,3u,3u,3u,4u,4u,4u,5u};
    static const core_machine_cpu_profile legacy[] = {
        CORE_MACHINE_CPU_PROFILE_8086, CORE_MACHINE_CPU_PROFILE_80186,
        CORE_MACHINE_CPU_PROFILE_80286
    };
    lib_u8 form, profile;

    for (profile = 0u; profile != sizeof(legacy) / sizeof(legacy[0]); ++profile)
        for (form = 0u; form != 3u; ++form)
            if (!stos_expect_ud(legacy[profile], codes[form], bytes[form]))
                return 0;
    for (form = 3u; form != sizeof(bytes); ++form)
        if (!stos_expect_ud(CORE_MACHINE_CPU_PROFILE_80386,
            codes[form], bytes[form])) return 0;
    return 1;
}

static void stos_prepare_protected(cpu_instruction_fixture *state,
    lib_u32 es_limit)
{
    cpu_instruction_prepare(state, CORE_MACHINE_CPU_PROFILE_80386);
    stos_seed(state);
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

static lib_i32 stos_test_protected(void)
{
    static const lib_u8 codes[][2] = {{0xaau, 0u}, {0x66u, 0xabu}};
    static const lib_u8 rep[] = {0xf3u, 0xaau};
    lib_u8 form;

    for (form = 0u; form != 2u; ++form) {
        cpu_instruction_fixture state;
        t_cpu before, after;
        lib_u32 image = 0xa5a5a5a5u;
        lib_u8 width = form == 0u ? 1u : 4u;

        stos_prepare_protected(&state, 0x0fu);
        state.cpu.data.edi = 0x10u;
        lib_memory_copy(state.memory + 0x3010u, &image, width);
        lib_memory_copy(state.memory + 0x2000u, codes[form],
            form == 0u ? 1u : 2u);
        before = state.cpu;
        core_machine_cpu_execution_refresh(&state.execution);
        after = state.cpu;
        if (!state.execution.stop_requested || !state.fault.valid ||
            !(state.fault.exception_mask & VCPUINS_EXCEPT_DF) ||
            after.data.eip != 0u || !stos_others_same(&before, &after) ||
            after.data.edi != before.data.edi ||
            lib_memory_compare(&before.data.es, &after.data.es,
                sizeof(before.data.es)) != 0 ||
            lib_memory_compare(state.memory + 0x3010u, &image, width) != 0)
            return 0;
    }
    {
        cpu_instruction_fixture state;
        t_cpu before, after;
        static const lib_u8 first = 0xa5u, second = 0xa5u;

        stos_prepare_protected(&state, 0x10u);
        state.cpu.data.edi = 0x10u;
        state.cpu.data.ecx = 0x11220003u;
        state.memory[0x3010u] = first;
        state.memory[0x3011u] = second;
        lib_memory_copy(state.memory + 0x2000u, rep, sizeof(rep));
        before = state.cpu;
        core_machine_cpu_execution_refresh(&state.execution);
        if (state.execution.stop_requested) return 0;
        core_machine_cpu_execution_refresh(&state.execution);
        after = state.cpu;
        if (!state.execution.stop_requested || !state.fault.valid ||
            !(state.fault.exception_mask & VCPUINS_EXCEPT_DF) ||
            after.data.eip != 0u || after.data.eax != before.data.eax ||
            after.data.ecx != 0x11220002u || after.data.edi != 0x11u ||
            after.data.edx != before.data.edx ||
            after.data.ebx != before.data.ebx ||
            after.data.esp != before.data.esp ||
            after.data.ebp != before.data.ebp ||
            after.data.esi != before.data.esi ||
            after.data.eflags != before.data.eflags ||
            lib_memory_compare(&before.data.es, &after.data.es,
                sizeof(before.data.es)) != 0 ||
            state.memory[0x3010u] != 0x44u || state.memory[0x3011u] != second)
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
    static const lib_u8 aa = 0xaau, ab = 0xabu;
    static const lib_u8 dword[] = {0x66u, 0xabu};
    static const lib_u8 address[] = {0x67u, 0xaau};
    static const lib_u8 both[] = {0x66u, 0x67u, 0xabu};
    static const lib_u8 cs[] = {0x2eu, 0xaau}, fs[] = {0x64u, 0xaau};
    static const lib_u8 repb[] = {0xf3u, 0xaau};
    static const lib_u8 repw[] = {0xf3u, 0xabu};
    static const lib_u8 repd[] = {0xf3u, 0x66u, 0xabu};
    static const lib_u8 rep67[] = {0xf3u, 0x67u, 0xaau};
    static const lib_u8 rep66_67[] = {0xf3u, 0x66u, 0x67u, 0xabu};
    lib_u8 profile;

    for (profile = 0u; profile != sizeof(profiles) / sizeof(profiles[0]); ++profile)
        if (!stos_case(profiles[profile], &aa, 1u, 1u, LIB_FALSE, LIB_FALSE) ||
            !stos_case(profiles[profile], &ab, 1u, 2u, LIB_FALSE, LIB_FALSE) ||
            !stos_rep_case(profiles[profile], repb, 2u, 1u, LIB_FALSE, 0u,
                LIB_FALSE) || !stos_rep_case(profiles[profile], repb, 2u,
                1u, LIB_FALSE, 1u, LIB_FALSE) ||
            !stos_rep_case(profiles[profile], repw, 2u, 2u, LIB_FALSE, 3u,
                LIB_FALSE)) {
            lib_c_printf("STOS stage=profile profile=%u\n", profile);
            return 1;
        }
    if (!stos_case(CORE_MACHINE_CPU_PROFILE_80386, dword, 2u, 4u,
        LIB_FALSE, LIB_FALSE) || !stos_case(CORE_MACHINE_CPU_PROFILE_80386,
        address, 2u, 1u, LIB_TRUE, LIB_FALSE) ||
        !stos_case(CORE_MACHINE_CPU_PROFILE_80386, both, 3u, 4u,
        LIB_TRUE, LIB_FALSE) || !stos_case(CORE_MACHINE_CPU_PROFILE_80386,
        cs, 2u, 1u, LIB_FALSE, LIB_FALSE) ||
        !stos_case(CORE_MACHINE_CPU_PROFILE_80386, fs, 2u, 1u,
        LIB_FALSE, LIB_FALSE) || !stos_case(CORE_MACHINE_CPU_PROFILE_80386,
        &ab, 1u, 2u, LIB_FALSE, LIB_TRUE)) {
        lib_c_printf("STOS stage=attributes\n");
        return 1;
    }
    if (!stos_rep_case(CORE_MACHINE_CPU_PROFILE_80386, repd, 3u, 4u,
        LIB_FALSE, 3u, LIB_FALSE) ||
        !stos_rep_case(CORE_MACHINE_CPU_PROFILE_80386, rep67, 3u, 1u,
        LIB_TRUE, 3u, LIB_FALSE) ||
        !stos_rep_case(CORE_MACHINE_CPU_PROFILE_80386, rep66_67, 4u, 4u,
        LIB_TRUE, 3u, LIB_FALSE) ||
        !stos_rep_case(CORE_MACHINE_CPU_PROFILE_80386, repw, 2u, 2u,
        LIB_FALSE, 3u, LIB_TRUE)) {
        lib_c_printf("STOS stage=rep-attributes\n");
        return 1;
    }
    if (!stos_test_rejections()) {
        lib_c_printf("STOS stage=rejections\n");
        return 1;
    }
    if (!stos_test_protected()) {
        lib_c_printf("STOS stage=protected\n");
        return 1;
    }
    lib_c_printf("M5:T316:S34:STOS:OK\n");
    lib_c_printf("M5:T401:S17:STOS-PROFILES:OK\n");
    return 0;
}
