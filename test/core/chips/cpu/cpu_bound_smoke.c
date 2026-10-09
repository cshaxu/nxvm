#include "support/cpu_instruction_fixture.h"
#include "lib/types/file.h"
/* REAL_UD_TERMINAL_CPU_OWNER: invalid BOUND stops at the CPU. */

static lib_i32 bound_same_state(const t_cpu *before, const t_cpu *after)
{
    return before->data.eax == after->data.eax &&
        before->data.ecx == after->data.ecx &&
        before->data.edx == after->data.edx &&
        before->data.ebx == after->data.ebx &&
        before->data.esp == after->data.esp &&
        before->data.ebp == after->data.ebp &&
        before->data.esi == after->data.esi &&
        before->data.edi == after->data.edi &&
        before->data.eflags == after->data.eflags &&
        lib_memory_compare(&before->data.es, &after->data.es,
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

static lib_i32 bound_check_success(core_machine_cpu_profile profile,
    const lib_u8 *code, lib_u8 bytes, const void *pair,
    lib_u8 pair_bytes, lib_u32 index)
{
    cpu_instruction_fixture state;
    t_cpu before, after;
    lib_u8 pair_after[8] = {0};

    cpu_instruction_prepare(&state, profile);
    state.cpu.data.eax = index;
    state.cpu.data.eflags = VCPU_EFLAGS_CF | VCPU_EFLAGS_PF |
        VCPU_EFLAGS_AF | VCPU_EFLAGS_ZF | VCPU_EFLAGS_SF |
        VCPU_EFLAGS_IF | VCPU_EFLAGS_DF | VCPU_EFLAGS_OF;
    lib_memory_copy(state.memory + 0x0400u, pair, pair_bytes);
    before = state.cpu;
    if (cpu_instruction_run(&state, code, bytes, &after) != LIB_STATUS_OK ||
        state.fault.valid || after.data.eip != bytes ||
        !bound_same_state(&before, &after)) return 0;
    lib_memory_copy(pair_after, state.memory + 0x0400u, pair_bytes);
    return lib_memory_compare(pair_after, pair, pair_bytes) == 0;
}

static lib_i32 bound_test_width_and_attributes(void)
{
    static const lib_u8 word[] = {0x62u,0x06u,0x00u,0x04u};
    static const lib_u8 dword[] = {0x66u,0x62u,0x06u,0x00u,0x04u};
    static const lib_u8 address[] = {
        0x67u,0x62u,0x05u,0x00u,0x04u,0x00u,0x00u
    };
    static const lib_u8 combined[] = {
        0x66u,0x67u,0x62u,0x05u,0x00u,0x04u,0x00u,0x00u
    };
    const lib_i16 pair16[] = {-2,2};
    const lib_i32 pair32[] = {-4,4};
    static const core_machine_cpu_profile profiles[] = {
        CORE_MACHINE_CPU_PROFILE_80186, CORE_MACHINE_CPU_PROFILE_80286,
        CORE_MACHINE_CPU_PROFILE_80386
    };
    lib_u8 profile;

    for (profile = 0u; profile != 3u; ++profile)
        if (!bound_check_success(profiles[profile], word, sizeof(word),
            pair16, sizeof(pair16), 0xa1a10001u)) return 0;
    return bound_check_success(CORE_MACHINE_CPU_PROFILE_80386, dword,
            sizeof(dword), pair32, sizeof(pair32), 1u) &&
        bound_check_success(CORE_MACHINE_CPU_PROFILE_80386, address,
            sizeof(address), pair16, sizeof(pair16), 1u) &&
        bound_check_success(CORE_MACHINE_CPU_PROFILE_80386, combined,
            sizeof(combined), pair32, sizeof(pair32), 1u) &&
        bound_check_success(CORE_MACHINE_CPU_PROFILE_80386, word,
            sizeof(word), pair16, sizeof(pair16), 0xfffeu) &&
        bound_check_success(CORE_MACHINE_CPU_PROFILE_80386, word,
            sizeof(word), pair16, sizeof(pair16), 2u);
}

static lib_i32 bound_test_segments(void)
{
    static const lib_u8 forms[][5] = {
        {0x62u,0x06u,0x10u,0u,0u},
        {0x62u,0x46u,0u,0u,0u},
        {0x2eu,0x62u,0x06u,0x10u,0u},
        {0x26u,0x62u,0x06u,0x10u,0u},
        {0x64u,0x62u,0x06u,0x10u,0u},
        {0x65u,0x62u,0x06u,0x10u,0u}
    };
    static const lib_u8 bytes[] = {4u,3u,5u,5u,5u,5u};
    const lib_i16 pair[] = {-2,2};
    lib_u8 form;

    for (form = 0u; form != 6u; ++form) {
        cpu_instruction_fixture state;
        t_cpu before, after;
        t_cpu_data_sreg *segment;
        const lib_u16 selector = (lib_u16)(0x1000u + form * 0x1000u);
        const lib_u32 base = (lib_u32)selector << 4u;

        cpu_instruction_prepare(&state, CORE_MACHINE_CPU_PROFILE_80386);
        segment = form == 1u ? &state.cpu.data.ss :
            form == 2u ? &state.cpu.data.cs :
            form == 3u ? &state.cpu.data.es :
            form == 4u ? &state.cpu.data.fs :
            form == 5u ? &state.cpu.data.gs : &state.cpu.data.ds;
        if (core_machine_cpu_execution_load_segment(&state.execution,
                segment, selector) != 0) return 0;
        state.cpu.data.eax = 1u;
        if (form == 1u) state.cpu.data.ebp = 0x10u;
        lib_memory_copy(state.memory + base + 0x10u, pair, sizeof(pair));
        lib_memory_copy(state.memory + state.cpu.data.cs.base,
            forms[form], bytes[form]);
        before = state.cpu;
        core_machine_cpu_execution_refresh(&state.execution);
        after = state.cpu;
        if (state.execution.stop_requested || state.fault.valid ||
            after.data.eip != bytes[form] ||
            !bound_same_state(&before, &after) ||
            lib_memory_compare(state.memory + base + 0x10u, pair,
                sizeof(pair)) != 0) return 0;
    }
    return 1;
}

static lib_i32 bound_test_sib_ss(void)
{
    static const lib_u8 code[] = {0x67u,0x62u,0x44u,0x24u,0x10u};
    const lib_i16 pair[] = {-2,2};
    cpu_instruction_fixture state;
    t_cpu before, after;

    cpu_instruction_prepare(&state, CORE_MACHINE_CPU_PROFILE_80386);
    if (core_machine_cpu_execution_load_segment(&state.execution,
            &state.cpu.data.ss, 0x1800u) != 0) return 0;
    state.cpu.data.eax = 1u;
    state.cpu.data.esp = 0x8000u;
    lib_memory_copy(state.memory + 0x20010u, pair, sizeof(pair));
    before = state.cpu;
    if (cpu_instruction_run(&state, code, sizeof(code), &after) !=
            LIB_STATUS_OK || state.fault.valid ||
        after.data.eip != sizeof(code) ||
        !bound_same_state(&before, &after)) return 0;
    return lib_memory_compare(state.memory + 0x20010u, pair,
        sizeof(pair)) == 0;
}

static lib_i32 bound_test_vm86(void)
{
    static const lib_u8 code[] = {0x62u,0x06u,0x00u,0x04u};
    const lib_i16 pair[] = {-2,2};
    cpu_instruction_fixture state;
    t_cpu before, after;

    cpu_instruction_prepare(&state, CORE_MACHINE_CPU_PROFILE_80386);
    state.cpu.data.cr0 |= VCPU_CR0_PE;
    state.cpu.data.eflags = VCPU_EFLAGS_VM | VCPU_EFLAGS_IOPL |
        VCPU_EFLAGS_IF | VCPU_EFLAGS_CF;
    state.cpu.data.cs.dpl = 3u;
    state.cpu.data.ds.dpl = 3u;
    state.cpu.data.ss.dpl = 3u;
    state.cpu.data.eax = 1u;
    lib_memory_copy(state.memory + 0x0400u, pair, sizeof(pair));
    before = state.cpu;
    if (cpu_instruction_run(&state, code, sizeof(code), &after) !=
            LIB_STATUS_OK || state.fault.valid ||
        after.data.eip != sizeof(code) ||
        !bound_same_state(&before, &after)) return 0;
    return lib_memory_compare(state.memory + 0x0400u, pair,
        sizeof(pair)) == 0;
}

static lib_i32 bound_expect_ud(core_machine_cpu_profile profile,
    const lib_u8 *code, lib_u8 bytes)
{
    cpu_instruction_fixture state;
    const lib_u8 pair[] = {0xfeu,0xffu,0x02u,0x00u};

    cpu_instruction_prepare(&state, profile);
    lib_memory_copy(state.memory + 0x0400u, pair, sizeof(pair));
    return cpu_instruction_expect_real_fault(&state, code, bytes, 6u) &&
        lib_memory_compare(state.memory + 0x0400u, pair,
            sizeof(pair)) == 0;
}

static lib_i32 bound_test_rejections(void)
{
    static const lib_u8 word_register[] = {0x62u,0xc0u};
    static const lib_u8 dword_register[] = {0x66u,0x62u,0xc0u};
    static const lib_u8 locked[] = {0xf0u,0x62u,0x06u,0x00u,0x04u};
    static const lib_u8 prefix[][6] = {
        {0x66u,0x62u,0x06u,0x00u,0x04u,0u},
        {0x67u,0x62u,0x06u,0x00u,0x04u,0u},
        {0x66u,0x67u,0x62u,0x06u,0x00u,0x04u}
    };
    static const core_machine_cpu_profile legacy[] = {
        CORE_MACHINE_CPU_PROFILE_8086, CORE_MACHINE_CPU_PROFILE_80186,
        CORE_MACHINE_CPU_PROFILE_80286
    };
    static const lib_u8 bare[] = {0x62u,0x06u,0x00u,0x04u};
    lib_u8 profile, form;

    if (!bound_expect_ud(CORE_MACHINE_CPU_PROFILE_80386,
            word_register, sizeof(word_register)) ||
        !bound_expect_ud(CORE_MACHINE_CPU_PROFILE_80386,
            dword_register, sizeof(dword_register)) ||
        !bound_expect_ud(CORE_MACHINE_CPU_PROFILE_80386,
            locked, sizeof(locked)) ||
        !bound_expect_ud(CORE_MACHINE_CPU_PROFILE_8086,
            bare, sizeof(bare))) return 0;
    for (profile = 0u; profile != 3u; ++profile)
        for (form = 0u; form != 3u; ++form)
            if (!bound_expect_ud(legacy[profile], prefix[form],
                    form == 2u ? 6u : 5u)) return 0;
    return 1;
}

lib_i32 main(void)
{
    if (!bound_test_width_and_attributes()) {
        lib_c_fprintf(lib_c_stderr, "BOUND width or address attributes failed\n");
        return 1;
    }
    if (!bound_test_rejections()) {
        lib_c_fprintf(lib_c_stderr, "BOUND invalid forms were not #UD\n");
        return 1;
    }
    if (!bound_test_segments() || !bound_test_sib_ss() ||
        !bound_test_vm86()) {
        lib_c_fprintf(lib_c_stderr, "BOUND segment route failed\n");
        return 1;
    }
    lib_c_printf("%s\n", "BOUND-REGISTER-UD:OK");
    return 0;
}
