#include "support/cpu_instruction_fixture.h"
#include "lib/types/file.h"
/* T337_REAL_UD_TERMINAL_CPU_OWNER: rejected ARPL stops at the CPU. */

static void arpl_prepare(cpu_instruction_fixture *state,
    core_machine_cpu_profile profile)
{
    cpu_instruction_prepare(state, profile);
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
    state->cpu.data.ds.limit = 0xffffu;
    state->cpu.data.ds.seg.data.writable = LIB_TRUE;
    state->cpu.data.es = state->cpu.data.ds;
    state->cpu.data.es.selector = 0x18u;
    state->cpu.data.es.base = 0x4000u;
    state->cpu.data.ss = state->cpu.data.ds;
    state->cpu.data.ss.selector = 0x20u;
    state->cpu.data.ss.base = 0x5000u;
    state->cpu.data.ss.sregtype = SREG_STACK;
    state->cpu.data.fs = state->cpu.data.ds;
    state->cpu.data.fs.selector = 0x28u;
    state->cpu.data.fs.base = 0x6000u;
    state->cpu.data.gs = state->cpu.data.ds;
    state->cpu.data.gs.selector = 0x30u;
    state->cpu.data.gs.base = 0x7000u;
}

static lib_u32 arpl_register(const t_cpu *cpu, lib_u8 index)
{
    switch (index) {
    case 0u: return cpu->data.eax;
    case 1u: return cpu->data.ecx;
    case 2u: return cpu->data.edx;
    case 3u: return cpu->data.ebx;
    case 4u: return cpu->data.esp;
    case 5u: return cpu->data.ebp;
    case 6u: return cpu->data.esi;
    case 7u: return cpu->data.edi;
    default: return 0u;
    }
}

static void arpl_set_register(t_cpu *cpu, lib_u8 index, lib_u32 value)
{
    switch (index) {
    case 0u: cpu->data.eax = value; break;
    case 1u: cpu->data.ecx = value; break;
    case 2u: cpu->data.edx = value; break;
    case 3u: cpu->data.ebx = value; break;
    case 4u: cpu->data.esp = value; break;
    case 5u: cpu->data.ebp = value; break;
    case 6u: cpu->data.esi = value; break;
    case 7u: cpu->data.edi = value; break;
    default: break;
    }
}

static void arpl_seed(t_cpu *cpu)
{
    cpu->data.eax = 0xa1a10000u;
    cpu->data.ecx = 0xb2b20000u;
    cpu->data.edx = 0xc3c30000u;
    cpu->data.ebx = 0xd4d40000u;
    cpu->data.esp = 0x00008000u;
    cpu->data.ebp = 0xe5e50000u;
    cpu->data.esi = 0xf6f60000u;
    cpu->data.edi = 0x97970000u;
}

static lib_i32 arpl_same_other_registers(const t_cpu *before,
    const t_cpu *after, lib_u8 destination)
{
    lib_u8 index;

    for (index = 0u; index != 8u; ++index)
        if (index != destination && arpl_register(before, index) !=
            arpl_register(after, index)) return 0;
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

static lib_i32 arpl_execute(cpu_instruction_fixture *state,
    const lib_u8 *code, lib_u8 bytes, t_cpu *after)
{
    lib_memory_copy(state->memory + state->cpu.data.cs.base, code, bytes);
    core_machine_cpu_execution_refresh(&state->execution);
    *after = state->cpu;
    return !state->execution.stop_requested && !state->fault.valid;
}

static lib_i32 arpl_test_registers(void)
{
    lib_u8 destination, source;

    for (destination = 0u; destination != 8u; ++destination)
        for (source = 0u; source != 8u; ++source) {
            cpu_instruction_fixture state;
            t_cpu before, after;
            const lib_u16 dst = 0xa541u;
            const lib_u16 src = 0x5a00u | (source == destination ? 1u : 3u);
            const lib_u16 expected = source == destination ? src :
                (lib_u16)((dst & ~VCPU_SELECTOR_RPL) |
                    (src & VCPU_SELECTOR_RPL));
            const lib_u8 code[] = {0x63u,
                (lib_u8)(0xc0u | (source << 3u) | destination)};

            arpl_prepare(&state, CORE_MACHINE_CPU_PROFILE_80386);
            arpl_seed(&state.cpu);
            arpl_set_register(&state.cpu, destination, dst);
            arpl_set_register(&state.cpu, source, src);
            before = state.cpu;
            if (!arpl_execute(&state, code, sizeof(code), &after) ||
                (arpl_register(&after, destination) & 0xffff0000u) !=
                    (arpl_register(&before, destination) & 0xffff0000u) ||
                (arpl_register(&after, destination) & 0xffffu) != expected ||
                X86_CPU_BIT_IS_SET(after.data.eflags, VCPU_EFLAGS_ZF) !=
                    (source != destination) ||
                (after.data.eflags & ~VCPU_EFLAGS_ZF) !=
                    (before.data.eflags & ~VCPU_EFLAGS_ZF) ||
                after.data.eip != 2u ||
                !arpl_same_other_registers(&before, &after, destination))
                return 0;
        }
    return 1;
}

static lib_i32 arpl_test_flags(void)
{
    static const lib_u8 code[] = {0x63u,0xcau};
    lib_u8 change;

    for (change = 0u; change != 2u; ++change) {
        cpu_instruction_fixture state;
        t_cpu before, after;

        arpl_prepare(&state, CORE_MACHINE_CPU_PROFILE_80386);
        arpl_seed(&state.cpu);
        state.cpu.data.edx = 0xc3c31201u;
        state.cpu.data.ecx = 0xb2b25600u | (change ? 3u : 1u);
        state.cpu.data.eflags |= VCPU_EFLAGS_CF | VCPU_EFLAGS_PF |
            VCPU_EFLAGS_AF | VCPU_EFLAGS_SF | VCPU_EFLAGS_OF |
            VCPU_EFLAGS_IF | VCPU_EFLAGS_DF;
        before = state.cpu;
        if (!arpl_execute(&state, code, sizeof(code), &after) ||
            (after.data.edx & 0xffff0000u) !=
                (before.data.edx & 0xffff0000u) ||
            (after.data.edx & 0xffffu) != (change ? 0x1203u : 0x1201u) ||
            X86_CPU_BIT_IS_SET(after.data.eflags, VCPU_EFLAGS_ZF) != change ||
            (after.data.eflags & ~VCPU_EFLAGS_ZF) !=
                (before.data.eflags & ~VCPU_EFLAGS_ZF) ||
            !arpl_same_other_registers(&before, &after, 2u) ||
            after.data.eip != sizeof(code)) return 0;
    }
    return 1;
}

static lib_i32 arpl_test_286_forms(void)
{
    static const lib_u8 adjust[] = {0x63u,0xc8u};
    static const lib_u8 prefix_memory[] = {0x26u,0x63u,0x0eu,0x00u,0x04u};
    lib_u8 form;

    for (form = 0u; form != 3u; ++form) {
        cpu_instruction_fixture state;
        t_cpu after;
        lib_u16 value = 1u;

        arpl_prepare(&state, CORE_MACHINE_CPU_PROFILE_80286);
        state.cpu.data.eax = form == 1u ? 3u : 1u;
        state.cpu.data.ecx = form == 1u ? 1u : 3u;
        lib_memory_copy(state.memory + 0x4400u, &value, sizeof(value));
        if (!arpl_execute(&state, form == 2u ? prefix_memory : adjust,
                form == 2u ? sizeof(prefix_memory) : sizeof(adjust), &after))
            return 0;
        lib_memory_copy(&value, state.memory + 0x4400u, sizeof(value));
        if (form == 2u) {
            if (value != 3u || !(after.data.eflags & VCPU_EFLAGS_ZF))
                return 0;
        } else if ((after.data.eax & 0xffffu) != 3u ||
            X86_CPU_BIT_IS_SET(after.data.eflags, VCPU_EFLAGS_ZF) !=
                (form == 0u)) return 0;
    }
    return 1;
}

static lib_i32 arpl_test_memory_case(const lib_u8 *code, lib_u8 bytes,
    lib_u32 address, lib_bool change)
{
    cpu_instruction_fixture state;
    t_cpu before, after;
    const lib_u16 adjacent = 0x3ca5u;
    lib_u16 value = change ? 0x5a01u : 0x5a03u;

    arpl_prepare(&state, CORE_MACHINE_CPU_PROFILE_80386);
    arpl_seed(&state.cpu);
    state.cpu.data.ecx = 0xb2b20003u;
    state.cpu.data.ebp = 0xe5e50400u;
    state.cpu.data.esi = 0x00000400u;
    lib_memory_copy(state.memory + address, &value, sizeof(value));
    lib_memory_copy(state.memory + address + 0x10u, &adjacent,
        sizeof(adjacent));
    before = state.cpu;
    if (!arpl_execute(&state, code, bytes, &after)) return 0;
    lib_memory_copy(&value, state.memory + address, sizeof(value));
    return value == 0x5a03u &&
        lib_memory_compare(state.memory + address + 0x10u, &adjacent,
            sizeof(adjacent)) == 0 &&
        after.data.ecx == before.data.ecx && after.data.eip == bytes &&
        X86_CPU_BIT_IS_SET(after.data.eflags, VCPU_EFLAGS_ZF) == change &&
        (after.data.eflags & ~VCPU_EFLAGS_ZF) ==
            (before.data.eflags & ~VCPU_EFLAGS_ZF) &&
        arpl_same_other_registers(&before, &after, 8u);
}

static lib_i32 arpl_test_memory(void)
{
    static const lib_u8 ds[] = {0x63u,0x0eu,0x00u,0x04u};
    static const lib_u8 ss[] = {0x63u,0x4eu,0x00u};
    static const lib_u8 es[] = {0x26u,0x63u,0x0eu,0x00u,0x04u};
    static const lib_u8 fs[] = {0x64u,0x63u,0x0eu,0x00u,0x04u};
    static const lib_u8 gs[] = {0x65u,0x63u,0x0eu,0x00u,0x04u};
    static const lib_u8 address32[] = {0x67u,0x63u,0x0eu};
    static const lib_u8 combined[] = {0x66u,0x67u,0x63u,0x0eu};
    static const lib_u8 operand[] = {0x66u,0x63u,0x0eu,0x00u,0x04u};

    return arpl_test_memory_case(ds, sizeof(ds), 0x3400u, LIB_TRUE) &&
        arpl_test_memory_case(ss, sizeof(ss), 0x5400u, LIB_FALSE) &&
        arpl_test_memory_case(es, sizeof(es), 0x4400u, LIB_TRUE) &&
        arpl_test_memory_case(fs, sizeof(fs), 0x6400u, LIB_TRUE) &&
        arpl_test_memory_case(gs, sizeof(gs), 0x7400u, LIB_TRUE) &&
        arpl_test_memory_case(address32, sizeof(address32), 0x3400u,
            LIB_TRUE) &&
        arpl_test_memory_case(combined, sizeof(combined), 0x3400u,
            LIB_TRUE) &&
        arpl_test_memory_case(operand, sizeof(operand), 0x3400u,
            LIB_TRUE);
}

static lib_i32 arpl_expect_ud(core_machine_cpu_profile profile,
    const lib_u8 *code, lib_u8 bytes)
{
    cpu_instruction_fixture state;
    t_cpu before, after;
    lib_u8 memory_before[8] = {0xa1u,0xa2u,0xa3u,0xa4u,
        0xa5u,0xa6u,0xa7u,0xa8u};

    cpu_instruction_prepare(&state, profile);
    state.cpu.data.idtr.limit = 0x17u;
    lib_memory_copy(state.memory + 0x0400u, memory_before,
        sizeof(memory_before));
    before = state.cpu;
    (void)cpu_instruction_run(&state, code, bytes, &after);
    return state.execution.stop_requested && state.fault.valid &&
        (state.fault.exception_mask & VCPUINS_EXCEPT_UD) &&
        lib_memory_compare(&before, &after, sizeof(before)) == 0 &&
        lib_memory_compare(state.memory + 0x0400u, memory_before,
            sizeof(memory_before)) == 0;
}

static lib_i32 arpl_test_rejections(void)
{
    static const lib_u8 basic[] = {0x63u,0xc8u};
    static const core_machine_cpu_profile legacy[] = {
        CORE_MACHINE_CPU_PROFILE_8086, CORE_MACHINE_CPU_PROFILE_80186,
        CORE_MACHINE_CPU_PROFILE_80286
    };
    static const lib_u8 attributes[][4] = {
        {0x66u,0x63u,0xc8u,0u}, {0x67u,0x63u,0xc8u,0u},
        {0x66u,0x67u,0x63u,0xc8u}
    };
    static const lib_u8 locks[][5] = {
        {0xf0u,0x63u,0xc8u,0u,0u},
        {0xf0u,0x66u,0x63u,0xc8u,0u},
        {0xf0u,0x67u,0x63u,0xc8u,0u},
        {0xf0u,0x66u,0x67u,0x63u,0xc8u}
    };
    lib_u8 profile, form;

    for (profile = 0u; profile != 3u; ++profile)
        if (!arpl_expect_ud(legacy[profile], basic, sizeof(basic))) return 0;
    for (profile = 0u; profile != 3u; ++profile)
        for (form = 0u; form != 3u; ++form)
            if (!arpl_expect_ud(legacy[profile], attributes[form],
                form == 2u ? 4u : 3u)) return 0;
    for (form = 0u; form != 4u; ++form)
        if (!arpl_expect_ud(CORE_MACHINE_CPU_PROFILE_80386, locks[form],
            form == 3u ? 5u : 4u)) return 0;
    return 1;
}

static lib_i32 arpl_test_protected_limit(void)
{
    static const lib_u8 code[] = {0x63u,0x0eu,0x00u,0x04u};
    cpu_instruction_fixture state;
    t_cpu before, after;
    const lib_u16 value = 0x5a01u;

    arpl_prepare(&state, CORE_MACHINE_CPU_PROFILE_80386);
    arpl_seed(&state.cpu);
    state.cpu.data.ecx = 3u;
    state.cpu.data.ds.limit = 0x000fu;
    state.cpu.data.idtr.limit = 0u;
    lib_memory_copy(state.memory + 0x3400u, &value, sizeof(value));
    before = state.cpu;
    lib_memory_copy(state.memory + 0x2000u, code, sizeof(code));
    core_machine_cpu_execution_refresh(&state.execution);
    after = state.cpu;
    return state.execution.stop_requested && state.fault.valid &&
        /* No IDT gate: the original #GP becomes terminal #DF here. */
        (state.fault.exception_mask & VCPUINS_EXCEPT_DF) &&
        after.data.eip == before.data.eip &&
        after.data.ecx == before.data.ecx &&
        after.data.eflags == before.data.eflags &&
        arpl_same_other_registers(&before, &after, 8u) &&
        lib_memory_compare(state.memory + 0x3400u, &value,
            sizeof(value)) == 0;
}

static lib_i32 arpl_test_metadata(void)
{
    core_machine_cpu_instruction_metadata metadata =
        core_machine_cpu_instruction_metadata_get(
            CORE_MACHINE_CPU_INSTRUCTION_PRIMARY, 0x63u, 0xc8u);

    return metadata.valid && metadata.minimum_cpu ==
        CORE_MACHINE_CPU_PROFILE_80286;
}

lib_i32 main(void)
{
    lib_i32 registers = arpl_test_registers();
    lib_i32 flags = arpl_test_flags();
    lib_i32 forms286 = arpl_test_286_forms();
    lib_i32 limit = arpl_test_protected_limit();
    lib_i32 memory = arpl_test_memory();
    lib_i32 rejections = arpl_test_rejections();
    lib_i32 metadata = arpl_test_metadata();

    if (!registers || !flags || !forms286 || !limit || !memory ||
        !rejections || !metadata) {
        lib_c_fprintf(lib_c_stderr,
            "M5:T539:S40:ARPL CPU failed register=%d flags=%d 286=%d limit=%d memory=%d reject=%d metadata=%d\n",
            registers, flags, forms286, limit, memory, rejections,
            metadata);
        return 1;
    }
    lib_c_printf("M5:T539:S40:ARPL-CPU:OK\n");
    return 0;
}
