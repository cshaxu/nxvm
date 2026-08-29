#define main arpl_historical_main
#include "core_machine_arpl_smoke.c"
#undef main

#define ARPL_S53_GDT_POINTER 0x0100u
#define ARPL_S53_GDT_BASE 0x0300u
#define ARPL_S53_CODE_BASE 0x2000u

static C_INT arpl_s53_boot(arpl_machine *state,
    core_machine_cpu_profile profile, const type_unsigned_8 *program,
    STD_SIZE_T program_bytes)
{
    static const type_unsigned_8 gdt_pointer[] = {
        0x37u, 0x00u, 0x00u, 0x03u, 0x00u, 0x00u
    };
    static const type_unsigned_8 idt_pointer[] = {
        0x07u, 0x01u, 0x00u, 0x04u, 0x00u, 0x00u
    };
    static const type_unsigned_8 gdt[] = {
        0,0,0,0,0,0,0,0,
        0xffu,0xffu,0,0x20u,0,0x9au,0,0,
        0xffu,0xffu,0,0x30u,0,0x92u,0,0,
        0xffu,0xffu,0,0x40u,0,0x92u,0,0,
        0xffu,0xffu,0,0x50u,0,0x92u,0,0,
        0xffu,0xffu,0,0x60u,0,0x92u,0,0,
        0xffu,0xffu,0,0x70u,0,0x92u,0,0
    };
    static const type_unsigned_8 real_code_286[] = {
        0x0fu,0x01u,0x16u,0x00u,0x01u,0x0fu,0x01u,0x1eu,0x10u,0x01u,
        0xb8u,0x01u,0x00u,0x0fu,0x01u,0xf0u,
        0xb8u,0x10u,0x00u,0x8eu,0xd0u,
        0xb8u,0x18u,0x00u,0x8eu,0xd8u,
        0xb8u,0x20u,0x00u,0x8eu,0xc0u,
        0xeau,0x00u,0x00u,0x08u,0x00u
    };
    static const type_unsigned_8 real_code_386[] = {
        0x0fu,0x01u,0x16u,0x00u,0x01u,0x0fu,0x01u,0x1eu,0x10u,0x01u,
        0xb8u,0x01u,0x00u,0x0fu,0x01u,0xf0u,
        0xb8u,0x10u,0x00u,0x8eu,0xd0u,
        0xb8u,0x18u,0x00u,0x8eu,0xd8u,
        0xb8u,0x20u,0x00u,0x8eu,0xc0u,
        0xb8u,0x28u,0x00u,0x8eu,0xe0u,
        0xb8u,0x30u,0x00u,0x8eu,0xe8u,
        0xeau,0x00u,0x00u,0x08u,0x00u
    };
    const type_unsigned_8 *real_code = profile == CORE_MACHINE_CPU_PROFILE_80386 ?
        real_code_386 : real_code_286;
    STD_SIZE_T real_bytes = profile == CORE_MACHINE_CPU_PROFILE_80386 ?
        sizeof(real_code_386) : sizeof(real_code_286);
    type_unsigned_8 idt[0x108u] = { 0u };

    idt[13u * 8u] = 0x00u;
    idt[13u * 8u + 1u] = 0x01u;
    idt[13u * 8u + 2u] = 0x08u;
    idt[13u * 8u + 5u] = 0x86u;

    return arpl_prepare(state, profile) &&
        core_machine_memory_write(state->machine, ARPL_S53_GDT_POINTER,
            gdt_pointer, sizeof(gdt_pointer)) == TYPE_STATUS_OK &&
        core_machine_memory_write(state->machine, ARPL_S53_GDT_BASE, gdt,
            sizeof(gdt)) == TYPE_STATUS_OK &&
        core_machine_memory_write(state->machine, 0x0110u, idt_pointer,
            sizeof(idt_pointer)) == TYPE_STATUS_OK &&
        core_machine_memory_write(state->machine, 0x0400u, idt, sizeof(idt)) ==
            TYPE_STATUS_OK &&
        core_machine_memory_write(state->machine, 0u, real_code, real_bytes) ==
            TYPE_STATUS_OK &&
        core_machine_memory_write(state->machine, ARPL_S53_CODE_BASE + 0x100u,
            (const type_unsigned_8[]){ 0xf4u }, 1u) == TYPE_STATUS_OK &&
        core_machine_memory_write(state->machine, ARPL_S53_CODE_BASE, program,
            program_bytes) == TYPE_STATUS_OK;
}

static C_INT arpl_s53_run(arpl_machine *state, t_cpu *out_cpu,
    core_machine_cpu_diagnostic *out_diagnostic)
{
    core_machine_run_result result;

    return core_machine_run(state->machine, (core_machine_run_budget){ 64u, 0u },
            &result) == TYPE_STATUS_OK &&
        result.reason == CORE_MACHINE_STOP_WAITING_FOR_INTERRUPT &&
        core_machine_get_cpu_diagnostic(state->machine, out_diagnostic) ==
            TYPE_STATUS_OK && !out_diagnostic->first_fault.valid &&
        ((*out_cpu = test_core_machine_fixture_capture_cpu_after_run(
            state->machine)), 1);
}

static C_INT arpl_s53_sregs_same(const t_cpu *before, const t_cpu *after)
{
    return STD_MEMCMP(&before->data.es, &after->data.es,
            sizeof(before->data.es)) == 0 &&
        STD_MEMCMP(&before->data.cs, &after->data.cs,
            sizeof(before->data.cs)) == 0 &&
        STD_MEMCMP(&before->data.ss, &after->data.ss,
            sizeof(before->data.ss)) == 0 &&
        STD_MEMCMP(&before->data.ds, &after->data.ds,
            sizeof(before->data.ds)) == 0 &&
        STD_MEMCMP(&before->data.fs, &after->data.fs,
            sizeof(before->data.fs)) == 0 &&
        STD_MEMCMP(&before->data.gs, &after->data.gs,
            sizeof(before->data.gs)) == 0;
}

static type_unsigned_32 arpl_s53_register(const t_cpu *cpu, type_unsigned_8 register_index)
{
    switch (register_index) {
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

static C_VOID arpl_s53_set_register(t_cpu *cpu, type_unsigned_8 register_index,
    type_unsigned_32 value)
{
    switch (register_index) {
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

static C_INT arpl_s53_gprs_same_except(const t_cpu *before,
    const t_cpu *after, type_unsigned_8 destination)
{
    type_unsigned_8 index;

    for (index = 0u; index != 8u; ++index) {
        if (index != destination && arpl_s53_register(before, index) !=
            arpl_s53_register(after, index))
            return 0;
    }
    return 1;
}

static C_INT arpl_s53_nonstack_gprs_same_except(const t_cpu *before,
    const t_cpu *after, type_unsigned_8 destination)
{
    type_unsigned_8 index;

    for (index = 0u; index != 8u; ++index) {
        if (index != destination && index != 4u &&
            arpl_s53_register(before, index) != arpl_s53_register(after, index))
            return 0;
    }
    return 1;
}

static C_VOID arpl_s53_seed_gprs(t_cpu *cpu)
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

static C_INT arpl_s53_test_direct_matrix(C_VOID)
{
    type_unsigned_8 destination;

    for (destination = 0u; destination != 8u; ++destination) {
        type_unsigned_8 source;

        for (source = 0u; source != 8u; ++source) {
            static const type_unsigned_8 hlt = 0xf4u;
            type_unsigned_8 code[] = { 0x63u,
                (type_unsigned_8)(0xc0u | (source << 3u) | destination), 0xf4u };
            arpl_machine state;
            core_machine_cpu_diagnostic diagnostic;
            t_cpu before;
            t_cpu after;
            type_unsigned_16 destination_value = 0xa540u | 1u;
            type_unsigned_16 source_value = 0x5a00u | (source == destination ? 1u : 3u);
            type_unsigned_16 expected = source == destination ? source_value :
                (type_unsigned_16)((destination_value & ~VCPU_SELECTOR_RPL) |
                    (source_value & VCPU_SELECTOR_RPL));

            C_INT failed = !arpl_s53_boot(&state,
                CORE_MACHINE_CPU_PROFILE_80386, &hlt, sizeof(hlt));

            if (!failed)
                failed = !arpl_s53_run(&state, &after, &diagnostic);
            if (!failed) {
                test_core_machine_fixture_resume_after_halt_at(state.machine, 0u);
                arpl_s53_seed_gprs(&state.machine->executor_cpu);
                arpl_s53_set_register(&state.machine->executor_cpu, destination,
                    destination_value);
                arpl_s53_set_register(&state.machine->executor_cpu, source,
                    source_value);
                before = test_core_machine_fixture_capture_cpu_after_run(
                    state.machine);
                failed = core_machine_memory_write(state.machine,
                    ARPL_S53_CODE_BASE, code, sizeof(code)) != TYPE_STATUS_OK;
            }
            if (!failed)
                failed = !arpl_s53_run(&state, &after, &diagnostic);
            failed |= (arpl_s53_register(&after, destination) & 0xffff0000u) !=
                (arpl_s53_register(&before, destination) & 0xffff0000u);
            failed |= (arpl_s53_register(&after, destination) & 0xffffu) != expected;
            failed |= TYPE_GET_BIT(after.data.eflags, VCPU_EFLAGS_ZF) !=
                (source != destination);
            failed |= (after.data.eflags & ~VCPU_EFLAGS_ZF) !=
                (before.data.eflags & ~VCPU_EFLAGS_ZF);
            failed |= after.data.eip != 3u;
            failed |= !arpl_s53_gprs_same_except(&before, &after, destination);
            failed |= !arpl_s53_sregs_same(&before, &after);
            core_machine_destroy(state.machine);
            if (failed)
                return 0;
        }
    }
    return 1;
}

static C_INT arpl_s53_test_rpl_and_flags(C_VOID)
{
    static const type_unsigned_8 hlt = 0xf4u;
    static const type_unsigned_8 code[] = { 0x63u, 0xcau, 0xf4u };
    type_unsigned_8 change;

    for (change = 0u; change != 2u; ++change) {
        arpl_machine state;
        core_machine_cpu_diagnostic diagnostic;
        t_cpu before;
        t_cpu after;
        C_INT failed = !arpl_s53_boot(&state,
            CORE_MACHINE_CPU_PROFILE_80386, &hlt, sizeof(hlt));

        if (!failed)
            failed = !arpl_s53_run(&state, &after, &diagnostic);
        if (!failed) {
            test_core_machine_fixture_resume_after_halt_at(state.machine, 0u);
            arpl_s53_seed_gprs(&state.machine->executor_cpu);
            state.machine->executor_cpu.data.edx = 0xc3c31201u;
            state.machine->executor_cpu.data.ecx = 0xb2b25600u |
                (change ? 3u : 1u);
            state.machine->executor_cpu.data.eflags |= VCPU_EFLAGS_CF |
                VCPU_EFLAGS_PF | VCPU_EFLAGS_AF | VCPU_EFLAGS_SF |
                VCPU_EFLAGS_OF | VCPU_EFLAGS_IF | VCPU_EFLAGS_DF;
            before = test_core_machine_fixture_capture_cpu_after_run(
                state.machine);
            failed = core_machine_memory_write(state.machine,
                ARPL_S53_CODE_BASE, code, sizeof(code)) != TYPE_STATUS_OK;
        }
        if (!failed)
            failed = !arpl_s53_run(&state, &after, &diagnostic);
        failed |= (after.data.edx & 0xffff0000u) !=
            (before.data.edx & 0xffff0000u);
        failed |= (after.data.edx & 0xffffu) !=
            (change ? 0x1203u : 0x1201u);
        failed |= TYPE_GET_BIT(after.data.eflags, VCPU_EFLAGS_ZF) != change;
        failed |= (after.data.eflags & ~VCPU_EFLAGS_ZF) !=
            (before.data.eflags & ~VCPU_EFLAGS_ZF);
        failed |= after.data.eip != 3u;
        failed |= !arpl_s53_gprs_same_except(&before, &after, 2u);
        failed |= !arpl_s53_sregs_same(&before, &after);
        core_machine_destroy(state.machine);
        if (failed)
            return 0;
    }
    return 1;
}

static C_INT arpl_s53_test_memory_case(const type_unsigned_8 *program,
    type_unsigned_8 program_bytes, type_unsigned_32 address, type_unsigned_8 change)
{
    const type_unsigned_16 adjacent = 0x3ca5u;
    type_unsigned_16 destination = change ? 0x5a01u : 0x5a03u;
    type_unsigned_16 after_destination = 0u;
    type_unsigned_16 after_adjacent = 0u;
    arpl_machine state;
    t_cpu before;
    t_cpu after;
    core_machine_cpu_diagnostic diagnostic;
    static const type_unsigned_8 hlt = 0xf4u;
    C_INT failed = !arpl_s53_boot(&state, CORE_MACHINE_CPU_PROFILE_80386,
        &hlt, sizeof(hlt));

    if (!failed) {
        failed = !arpl_s53_run(&state, &after, &diagnostic);
    }
    if (!failed) {
        test_core_machine_fixture_resume_after_halt_at(state.machine, 0u);
        arpl_s53_seed_gprs(&state.machine->executor_cpu);
        state.machine->executor_cpu.data.ecx = 0xb2b20003u;
        if (program[0u] == 0x63u && program[1u] == 0x4eu)
            state.machine->executor_cpu.data.ebp = 0xe5e50400u;
        if (program[0u] == 0x67u || (program[0u] == 0x66u &&
            program[1u] == 0x67u))
            state.machine->executor_cpu.data.esi = 0x00000400u;
        before = test_core_machine_fixture_capture_cpu_after_run(state.machine);
        failed = core_machine_memory_write(state.machine, address, &destination,
            sizeof(destination)) != TYPE_STATUS_OK;
        failed |= core_machine_memory_write(state.machine, address + 0x10u,
            &adjacent, sizeof(adjacent)) != TYPE_STATUS_OK;
        failed |= core_machine_memory_write(state.machine, ARPL_S53_CODE_BASE,
            program, program_bytes) != TYPE_STATUS_OK;
    }
    if (!failed) {
        failed = !arpl_s53_run(&state, &after, &diagnostic);
        failed |= after.data.eip != program_bytes;
        failed |= after.data.ecx != before.data.ecx;
        failed |= TYPE_GET_BIT(after.data.eflags, VCPU_EFLAGS_ZF) != change;
        failed |= (after.data.eflags & ~VCPU_EFLAGS_ZF) !=
            (before.data.eflags & ~VCPU_EFLAGS_ZF);
        failed |= !arpl_s53_gprs_same_except(&before, &after, 8u);
        failed |= !arpl_s53_sregs_same(&before, &after);
        failed |= core_machine_memory_read_physical(&state.machine->executor_memory,
            address, (type_virtual_address)&after_destination,
            sizeof(after_destination)) != TYPE_STATUS_OK;
        failed |= after_destination != (change ? 0x5a03u : destination);
        failed |= core_machine_memory_read_physical(&state.machine->executor_memory,
            address + 0x10u, (type_virtual_address)&after_adjacent,
            sizeof(after_adjacent)) != TYPE_STATUS_OK;
        failed |= after_adjacent != adjacent;
    }
    core_machine_destroy(state.machine);
    return !failed;
}

static C_INT arpl_s53_test_memory_and_attributes(C_VOID)
{
    static const type_unsigned_8 ds[] = { 0xb9u,0x03u,0x00u,0x63u,0x0eu,0x00u,0x04u,0xf4u };
    static const type_unsigned_8 ss[] = { 0xbdu,0x00u,0x04u,0xb9u,0x03u,0x00u,0x63u,0x4eu,0x00u,0xf4u };
    static const type_unsigned_8 es[] = { 0xb9u,0x03u,0x00u,0x26u,0x63u,0x0eu,0x00u,0x04u,0xf4u };
    static const type_unsigned_8 fs[] = { 0xb9u,0x03u,0x00u,0x64u,0x63u,0x0eu,0x00u,0x04u,0xf4u };
    static const type_unsigned_8 gs[] = { 0xb9u,0x03u,0x00u,0x65u,0x63u,0x0eu,0x00u,0x04u,0xf4u };
    static const type_unsigned_8 address32[] = {
        0x66u,0xbeu,0x00u,0x04u,0x00u,0x00u,
        0xb9u,0x03u,0x00u,0x67u,0x63u,0x0eu,0xf4u
    };
    static const type_unsigned_8 combined[] = {
        0x66u,0xbeu,0x00u,0x04u,0x00u,0x00u,
        0xb9u,0x03u,0x00u,0x66u,0x67u,0x63u,0x0eu,0xf4u
    };
    static const type_unsigned_8 operand[] = {
        0xb9u,0x03u,0x00u,0x66u,0x63u,0x0eu,0x00u,0x04u,0xf4u
    };

    return arpl_s53_test_memory_case(ds + 3u, sizeof(ds) - 3u,
            0x00004400u, 1u) &&
        arpl_s53_test_memory_case(ss + 6u, sizeof(ss) - 6u,
            0x00003400u, 0u) &&
        arpl_s53_test_memory_case(es + 3u, sizeof(es) - 3u,
            0x00005400u, 1u) &&
        arpl_s53_test_memory_case(fs + 3u, sizeof(fs) - 3u,
            0x00006400u, 1u) &&
        arpl_s53_test_memory_case(gs + 3u, sizeof(gs) - 3u,
            0x00007400u, 1u) &&
        arpl_s53_test_memory_case(address32 + 9u, sizeof(address32) - 9u,
            0x00004400u, 1u) &&
        arpl_s53_test_memory_case(combined + 9u, sizeof(combined) - 9u,
            0x00004400u, 1u) &&
        arpl_s53_test_memory_case(operand + 3u, sizeof(operand) - 3u,
            0x00004400u, 1u);
}

static C_INT arpl_s53_expect_ud(core_machine_cpu_profile profile,
    const type_unsigned_8 *program, type_unsigned_8 program_bytes)
{
    arpl_machine state;
    core_machine_run_result result;
    core_machine_cpu_diagnostic diagnostic;
    t_cpu before;
    t_cpu after;
    type_unsigned_8 memory_before[8] = { 0xa1u,0xa2u,0xa3u,0xa4u,0xa5u,0xa6u,0xa7u,0xa8u };
    type_unsigned_8 memory_after[sizeof(memory_before)] = { 0u };
    C_INT failed = !arpl_prepare(&state, profile);

    if (!failed) {
        failed = !test_core_machine_fixture_prepare_real_mode_execution(
            state.machine, 0u);
        failed |= core_machine_memory_write(state.machine, 0u, program,
            program_bytes) != TYPE_STATUS_OK;
        failed |= core_machine_memory_write(state.machine, 0x0400u,
            memory_before, sizeof(memory_before)) != TYPE_STATUS_OK;
    }
    if (!failed) {
        failed |= !test_core_machine_fixture_preflight_real_ud_terminal(
            state.machine);
        before = test_core_machine_fixture_capture_cpu_after_run(state.machine);
        failed |= core_machine_run(state.machine,
            (core_machine_run_budget){ 1u, 0u }, &result) != TYPE_STATUS_FAULT;
        after = test_core_machine_fixture_capture_cpu_after_run(state.machine);
        failed |= core_machine_get_cpu_diagnostic(state.machine, &diagnostic) !=
            TYPE_STATUS_OK;
        failed |= result.reason != CORE_MACHINE_STOP_FAULT;
        failed |= !diagnostic.first_fault.valid;
        failed |= !TYPE_GET_BIT(diagnostic.first_fault.exception_mask,
            VCPUINS_EXCEPT_UD);
        failed |= STD_MEMCMP(&before, &after, sizeof(before)) != 0;
        failed |= core_machine_memory_read_physical(&state.machine->executor_memory,
            0x0400u, (type_virtual_address)memory_after,
            sizeof(memory_after)) != TYPE_STATUS_OK;
        failed |= STD_MEMCMP(memory_before, memory_after,
            sizeof(memory_before)) != 0;
    }
    core_machine_destroy(state.machine);
    return !failed;
}

static C_INT arpl_s53_test_rejections(C_VOID)
{
    static const core_machine_cpu_profile legacy[] = {
        CORE_MACHINE_CPU_PROFILE_8086, CORE_MACHINE_CPU_PROFILE_80186,
        CORE_MACHINE_CPU_PROFILE_80286
    };
    static const type_unsigned_8 attributes[][4] = {
        { 0x66u,0x63u,0xc8u,0u }, { 0x67u,0x63u,0xc8u,0u },
        { 0x66u,0x67u,0x63u,0xc8u }
    };
    static const type_unsigned_8 lock_forms[][5] = {
        { 0xf0u,0x63u,0xc8u,0u,0u },
        { 0xf0u,0x66u,0x63u,0xc8u,0u },
        { 0xf0u,0x67u,0x63u,0xc8u,0u },
        { 0xf0u,0x66u,0x67u,0x63u,0xc8u }
    };
    type_unsigned_8 profile;

    for (profile = 0u; profile != sizeof(legacy) / sizeof(legacy[0]); ++profile) {
        type_unsigned_8 form;

        for (form = 0u; form != sizeof(attributes) / sizeof(attributes[0]); ++form) {
            type_unsigned_8 bytes = form == 2u ? 4u : 3u;

            if (!arpl_s53_expect_ud(legacy[profile], attributes[form], bytes))
                return 0;
        }
    }
    for (profile = 0u; profile != sizeof(lock_forms) / sizeof(lock_forms[0]); ++profile) {
        type_unsigned_8 bytes = profile == 3u ? 5u : 4u;

        if (!arpl_s53_expect_ud(CORE_MACHINE_CPU_PROFILE_80386,
                lock_forms[profile], bytes))
            return 0;
    }
    return 1;
}

static C_INT arpl_s53_test_protected_read_limit(C_VOID)
{
    static const type_unsigned_8 hlt = 0xf4u;
    static const type_unsigned_8 program[] = { 0x63u,0x0eu,0x00u,0x04u };
    const type_unsigned_16 image = 0x5a01u;
    const type_unsigned_16 adjacent = 0x7e7eu;
    arpl_machine state;
    core_machine_run_result result;
    core_machine_cpu_diagnostic diagnostic;
    t_cpu before;
    t_cpu after;
    type_unsigned_16 observed = 0u;
    type_unsigned_16 observed_adjacent = 0u;
    type_unsigned_16 frame[4u] = { 0u };
    C_INT failed = !arpl_s53_boot(&state, CORE_MACHINE_CPU_PROFILE_80386,
        &hlt, sizeof(hlt));

    if (!failed) {
        failed = !arpl_s53_run(&state, &after, &diagnostic);
    }
    if (!failed) {
        test_core_machine_fixture_resume_after_halt_at(state.machine, 0u);
        arpl_s53_seed_gprs(&state.machine->executor_cpu);
        state.machine->executor_cpu.data.ecx = 0xb2b20003u;
        state.machine->executor_cpu.data.ds.limit = 0x000fu;
        before = test_core_machine_fixture_capture_cpu_after_run(state.machine);
        failed = core_machine_memory_write(state.machine, 0x00004400u, &image,
            sizeof(image)) != TYPE_STATUS_OK;
        failed |= core_machine_memory_write(state.machine, 0x00004410u, &adjacent,
            sizeof(adjacent)) != TYPE_STATUS_OK;
        failed |= core_machine_memory_write(state.machine, ARPL_S53_CODE_BASE,
            program, sizeof(program)) != TYPE_STATUS_OK;
    }
    if (!failed) {
        failed |= core_machine_run(state.machine,
            (core_machine_run_budget){ 64u, 0u }, &result) != TYPE_STATUS_OK;
        after = test_core_machine_fixture_capture_cpu_after_run(state.machine);
        failed |= core_machine_get_cpu_diagnostic(state.machine, &diagnostic) !=
            TYPE_STATUS_OK;
        failed |= diagnostic.first_fault.valid;
        failed |= !diagnostic.last_delivered_exception.valid;
        failed |= !TYPE_GET_BIT(diagnostic.last_delivered_exception.exception_mask,
            VCPUINS_EXCEPT_GP);
        failed |= diagnostic.last_delivered_exception.point.eip != 0u;
        failed |= after.data.eip != 0x00000101u;
        failed |= after.data.ecx != before.data.ecx;
        failed |= !arpl_s53_sregs_same(&before, &after);
        failed |= core_machine_memory_read_physical(&state.machine->executor_memory,
            0x00004400u, (type_virtual_address)&observed,
            sizeof(observed)) != TYPE_STATUS_OK;
        failed |= observed != image;
        failed |= core_machine_memory_read_physical(&state.machine->executor_memory,
            0x00004410u, (type_virtual_address)&observed_adjacent,
            sizeof(observed_adjacent)) != TYPE_STATUS_OK;
        failed |= observed_adjacent != adjacent;
        failed |= core_machine_memory_read_physical(&state.machine->executor_memory,
            after.data.ss.base + (type_unsigned_16)after.data.esp,
            (type_virtual_address)frame, sizeof(frame)) != TYPE_STATUS_OK;
        failed |= frame[0u] != 0u || frame[1u] != 0u || frame[2u] != 0x0008u;
        failed |= frame[3u] != (type_unsigned_16)before.data.eflags;
    }
    core_machine_destroy(state.machine);
    return !failed;
}

static C_INT arpl_s53_test_irq(C_VOID)
{
    static const type_unsigned_8 hlt = 0xf4u;
    static const type_unsigned_8 code[] = { 0xfbu, 0x63u, 0xc8u, 0x90u };
    type_unsigned_8 change;

    for (change = 0u; change != 2u; ++change) {
        arpl_machine state;
        core_machine_pic_irq_source source;
        core_machine_run_result result;
        core_machine_cpu_diagnostic diagnostic;
        t_cpu before;
        t_cpu after;
        type_status run_status = TYPE_STATUS_OK;
        type_unsigned_8 gate[8] = { 0u };
        type_unsigned_32 frame[3u] = { 0u };
        C_INT failed = !arpl_s53_boot(&state, CORE_MACHINE_CPU_PROFILE_80386,
            &hlt, sizeof(hlt));

        if (!failed) {
            failed = core_machine_run(state.machine,
                (core_machine_run_budget){ 64u, 0u }, &result) != TYPE_STATUS_OK;
            failed |= result.reason != CORE_MACHINE_STOP_WAITING_FOR_INTERRUPT;
        }
        if (!failed) {
            test_core_machine_fixture_resume_after_halt_at(state.machine, 0u);
            gate[1u] = 0x01u;
            gate[2u] = 0x08u;
            gate[5u] = 0x8eu;
            failed |= core_machine_memory_write(state.machine, 0x0500u, gate,
                sizeof(gate)) != TYPE_STATUS_OK;
            failed |= core_machine_memory_write(state.machine, ARPL_S53_CODE_BASE,
                code, sizeof(code)) != TYPE_STATUS_OK;
            arpl_s53_seed_gprs(&state.machine->executor_cpu);
            state.machine->executor_cpu.data.eax = 0xa1a10000u |
                (change ? 1u : 3u);
            state.machine->executor_cpu.data.ecx = 0xb2b20000u |
                (change ? 3u : 1u);
            state.machine->executor_cpu.data.eflags |= VCPU_EFLAGS_IF;
            before = test_core_machine_fixture_capture_cpu_after_run(
                state.machine);
            STD_MEMSET(&source, 0, sizeof(source));
            state.machine->shared_pic_master.data.icw2 = 0x20u;
            core_machine_pic_irq_source_bind(&source,
                &state.machine->shared_pic_master, &state.machine->shared_pic_slave,
                0u);
            core_machine_pic_irq_source_assert(&source);
            core_machine_pic_irq_source_deassert(&source);
            run_status = core_machine_run(state.machine,
                (core_machine_run_budget){ 2u, 0u }, &result);
            failed |= run_status != TYPE_STATUS_OK;
            after = test_core_machine_fixture_capture_cpu_after_run(state.machine);
            failed |= core_machine_get_cpu_diagnostic(state.machine, &diagnostic) !=
                TYPE_STATUS_OK;
            failed |= diagnostic.first_fault.valid;
            failed |= result.reason != CORE_MACHINE_STOP_BUDGET;
            failed |= after.data.eip != 0x100u;
            failed |= (after.data.eax & 0xffff0000u) !=
                (before.data.eax & 0xffff0000u);
            failed |= (after.data.eax & 0xffffu) != (change ? 3u : 3u);
            failed |= TYPE_GET_BIT(after.data.eflags, VCPU_EFLAGS_ZF) != change;
            failed |= (after.data.eflags & ~(VCPU_EFLAGS_ZF | VCPU_EFLAGS_IF)) !=
                (before.data.eflags & ~(VCPU_EFLAGS_ZF | VCPU_EFLAGS_IF));
            failed |= !arpl_s53_nonstack_gprs_same_except(&before, &after, 0u);
            failed |= !arpl_s53_sregs_same(&before, &after);
            failed |= !TYPE_GET_BIT(state.machine->shared_pic_master.data.isr,
                VPIC_ISR_IRQ(0u));
            failed |= TYPE_GET_BIT(state.machine->shared_pic_master.data.irr,
                VPIC_IRR_IRQ(0u));
            failed |= core_machine_memory_read_physical(&state.machine->executor_memory,
                after.data.ss.base + (type_unsigned_16)after.data.esp,
                (type_virtual_address)frame, sizeof(frame)) != TYPE_STATUS_OK;
            failed |= frame[0u] != 3u;
            failed |= (frame[2u] & ~VCPU_EFLAGS_ZF) !=
                (before.data.eflags & ~VCPU_EFLAGS_ZF);
            failed |= TYPE_GET_BIT(frame[2u], VCPU_EFLAGS_ZF) != change;
        }
        if (failed) {
            core_machine_destroy(state.machine);
            return 0;
        }
        core_machine_destroy(state.machine);
    }
    return 1;
}

static C_INT arpl_s53_test_legacy_and_mode(C_VOID)
{
    return !arpl_test_rejected_forms() && !arpl_test_metadata() &&
        !arpl_test_register_forms() && !arpl_test_memory_prefix_form();
}

C_INT main(C_VOID)
{
    if (!arpl_s53_test_direct_matrix()) {
        STD_FPRINTF(STD_STDERR, "M5:T316:S53:ARPL direct failed\n");
        return 1;
    }
    if (!arpl_s53_test_rpl_and_flags()) {
        STD_FPRINTF(STD_STDERR, "M5:T316:S53:ARPL flags failed\n");
        return 1;
    }
    if (!arpl_s53_test_memory_and_attributes()) {
        STD_FPRINTF(STD_STDERR, "M5:T316:S53:ARPL memory failed\n");
        return 1;
    }
    if (!arpl_s53_test_rejections()) {
        STD_FPRINTF(STD_STDERR, "M5:T316:S53:ARPL reject failed\n");
        return 1;
    }
    if (!arpl_s53_test_protected_read_limit()) {
        STD_FPRINTF(STD_STDERR, "M5:T316:S53:ARPL protected failed\n");
        return 1;
    }
    if (!arpl_s53_test_irq()) {
        STD_FPRINTF(STD_STDERR, "M5:T316:S53:ARPL irq failed\n");
        return 1;
    }
    if (!arpl_s53_test_legacy_and_mode()) {
        STD_FPRINTF(STD_STDERR, "M5:T316:S53:ARPL legacy failed\n");
        return 1;
    }
    STD_PRINTF("M5:T316:S53:ARPL:OK\n");
    STD_PRINTF("M5:T401:S28:ARPL-PROFILES:OK\n");
    return 0;
}
