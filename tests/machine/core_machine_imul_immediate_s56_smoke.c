#include "type.h"

#include "core/machine/cpu.h"
#include "core/machine/machine_interface.h"
#include "../support/core_machine_cpu_fixture.h"

static C_VOID imul_s56_reset(C_VOID *owner)
{
    core_machine *machine = (core_machine *)owner;

    if (machine != STD_NULL)
        (C_VOID)test_core_machine_fixture_reset_real_mode(machine);
}

static const core_machine_execution_provider imul_s56_execution_provider = {
    imul_s56_reset,
    STD_NULL
};

static C_INT imul_s56_prepare(core_machine **machine,
    core_machine_cpu_profile profile)
{
    const core_machine_config config = {
        .memory_bytes = CORE_MACHINE_MINIMUM_MEMORY_BYTES,
        .cpu_profile = profile,
        .fpu_profile = CORE_MACHINE_FPU_PROFILE_NONE
    };

    return core_machine_create(&config, machine) == TYPE_STATUS_OK &&
        test_core_machine_fixture_bind_freeze_reset(*machine,
            &imul_s56_execution_provider, *machine) &&
        test_core_machine_fixture_prepare_real_mode_execution(*machine, 0u);
}

static C_VOID imul_s56_seed(t_cpu *cpu)
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

static C_INT imul_s56_sregs_same(const t_cpu *before, const t_cpu *after)
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

static C_INT imul_s56_nonparticipants_same(const t_cpu *before,
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

static C_INT imul_s56_nonarithmetic_flags_same(const t_cpu *before,
    const t_cpu *after)
{
    const type_unsigned_32 imul_flags = VCPU_EFLAGS_CF | VCPU_EFLAGS_OF |
        VCPU_EFLAGS_SF | VCPU_EFLAGS_ZF | VCPU_EFLAGS_AF | VCPU_EFLAGS_PF;

    return (before->data.eflags & ~imul_flags) ==
        (after->data.eflags & ~imul_flags);
}

static C_INT imul_s56_run(core_machine_cpu_profile profile,
    const type_unsigned_8 *code, type_unsigned_8 bytes, type_unsigned_32 source,
    type_unsigned_32 expected, C_INT overflow, C_INT dword)
{
    core_machine *machine = STD_NULL;
    core_machine_run_result result;
    core_machine_cpu_diagnostic diagnostic;
    t_cpu before;
    t_cpu after;
    type_status status;
    C_INT failed = !imul_s56_prepare(&machine, profile);

    if (!failed) {
        imul_s56_seed(&machine->executor_cpu);
        machine->executor_cpu.data.ecx = source;
        if (code[1] == 0xc0u)
            machine->executor_cpu.data.eax = source;
        before = machine->executor_cpu;
        failed = core_machine_memory_write(machine, 0u, code, bytes) !=
            TYPE_STATUS_OK;
        if (!failed) {
            status = core_machine_run(machine,
                (core_machine_run_budget){ 1u, 0u }, &result);
            after = test_core_machine_fixture_capture_cpu_after_run(machine);
            failed = core_machine_get_cpu_diagnostic(machine, &diagnostic) !=
                TYPE_STATUS_OK;
            failed |= status != TYPE_STATUS_OK;
            failed |= result.reason != CORE_MACHINE_STOP_BUDGET;
            failed |= diagnostic.first_fault.valid;
            failed |= after.data.eip != bytes;
            failed |= after.data.ecx != before.data.ecx;
            failed |= after.data.edx != before.data.edx;
            failed |= after.data.ebx != before.data.ebx;
            failed |= after.data.ebp != before.data.ebp;
            failed |= after.data.esi != before.data.esi;
            failed |= after.data.edi != before.data.edi;
            failed |= !imul_s56_sregs_same(&before, &after);
            failed |= dword ? after.data.eax != expected :
                after.data.eax != ((before.data.eax & 0xffff0000u) |
                    (expected & 0xffffu));
            failed |= !!TYPE_GET_BIT(after.data.eflags, VCPU_EFLAGS_CF) !=
                overflow;
            failed |= !!TYPE_GET_BIT(after.data.eflags, VCPU_EFLAGS_OF) !=
                overflow;
            failed |= (after.data.eflags & ~(VCPU_EFLAGS_CF | VCPU_EFLAGS_OF |
                VCPU_EFLAGS_SF | VCPU_EFLAGS_ZF | VCPU_EFLAGS_AF |
                VCPU_EFLAGS_PF)) != (before.data.eflags &
                ~(VCPU_EFLAGS_CF | VCPU_EFLAGS_OF | VCPU_EFLAGS_SF |
                VCPU_EFLAGS_ZF | VCPU_EFLAGS_AF | VCPU_EFLAGS_PF));
        }
    }
    core_machine_destroy(machine);
    return !failed;
}

static C_INT imul_s56_test_defaults(C_VOID)
{
    static const type_unsigned_8 imul_iw[] = { 0x69u, 0xc1u, 0xfeu, 0xffu };
    static const type_unsigned_8 imul_ib[] = { 0x6bu, 0xc1u, 0xfeu };
    static const type_unsigned_8 alias_iw[] = { 0x69u, 0xc0u, 0xfeu, 0xffu };
    static const type_unsigned_8 alias_ib[] = { 0x6bu, 0xc0u, 0xfeu };
    static const type_unsigned_8 overflow[] = { 0x69u, 0xc1u, 0x00u, 0x40u };
    static const core_machine_cpu_profile profiles[] = {
        CORE_MACHINE_CPU_PROFILE_80186,
        CORE_MACHINE_CPU_PROFILE_80286,
        CORE_MACHINE_CPU_PROFILE_80386
    };
    type_unsigned_8 profile;

    for (profile = 0u; profile != sizeof(profiles) / sizeof(profiles[0]);
            ++profile) {
        if (!imul_s56_run(profiles[profile], imul_iw, sizeof(imul_iw),
                0xb2b2fffeu, 4u, 0, 0) ||
            !imul_s56_run(profiles[profile], imul_ib, sizeof(imul_ib),
                0xb2b2fffeu, 4u, 0, 0) ||
            !imul_s56_run(profiles[profile], alias_iw, sizeof(alias_iw),
                0xa1a10003u, 0xa1a1fffau, 0, 0) ||
            !imul_s56_run(profiles[profile], alias_ib, sizeof(alias_ib),
                0xa1a10003u, 0xa1a1fffau, 0, 0) ||
            !imul_s56_run(profiles[profile], overflow, sizeof(overflow),
                0xb2b20002u, 0x8000u, 1, 0))
            return 0;
    }
    return 1;
}

static C_INT imul_s56_expect_ud(core_machine_cpu_profile profile,
    const type_unsigned_8 *code, type_unsigned_8 bytes)
{
    core_machine *machine = STD_NULL;
    core_machine_run_result result;
    core_machine_cpu_diagnostic diagnostic;
    t_cpu before;
    t_cpu after;
    C_INT failed = !imul_s56_prepare(&machine, profile);

    if (!failed) {
        imul_s56_seed(&machine->executor_cpu);
        failed = !test_core_machine_fixture_preflight_real_ud_terminal(machine);
        before = machine->executor_cpu;
        failed |= core_machine_memory_write(machine, 0u, code, bytes) !=
            TYPE_STATUS_OK;
        if (!failed) {
            failed = core_machine_run(machine,
                (core_machine_run_budget){ 1u, 0u }, &result) !=
                TYPE_STATUS_FAULT;
            after = test_core_machine_fixture_capture_cpu_after_run(machine);
            failed |= result.reason != CORE_MACHINE_STOP_FAULT;
            failed |= core_machine_get_cpu_diagnostic(machine, &diagnostic) !=
                TYPE_STATUS_OK;
            failed |= !diagnostic.first_fault.valid;
            failed |= !TYPE_GET_BIT(diagnostic.first_fault.exception_mask,
                VCPUINS_EXCEPT_UD);
            failed |= STD_MEMCMP(&before, &after, sizeof(before)) != 0;
        }
    }
    core_machine_destroy(machine);
    return !failed;
}

static C_INT imul_s56_test_attributes_and_rejects(C_VOID)
{
    static const type_unsigned_8 dword_iw[] = {
        0x66u, 0x69u, 0xc1u, 0x00u, 0x00u, 0x00u, 0x40u
    };
    static const type_unsigned_8 dword_ib[] = { 0x66u, 0x6bu, 0xc1u, 0xfeu };
    static const type_unsigned_8 inert[] = { 0x67u, 0x6bu, 0xc1u, 0xfeu };
    static const type_unsigned_8 combined[] = {
        0x66u, 0x67u, 0x69u, 0xc1u, 0xfeu, 0xffu, 0xffu, 0xffu
    };
    static const type_unsigned_8 lock[][9] = {
        { 0xf0u, 0x69u, 0xc1u, 0xfeu, 0xffu },
        { 0xf0u, 0x6bu, 0xc1u, 0xfeu },
        { 0xf0u, 0x66u, 0x69u, 0xc1u, 0xfeu, 0xffu, 0xffu, 0xffu },
        { 0xf0u, 0x66u, 0x6bu, 0xc1u, 0xfeu },
        { 0xf0u, 0x67u, 0x69u, 0xc1u, 0xfeu, 0xffu },
        { 0xf0u, 0x67u, 0x6bu, 0xc1u, 0xfeu },
        { 0xf0u, 0x66u, 0x67u, 0x69u, 0xc1u, 0xfeu, 0xffu, 0xffu, 0xffu },
        { 0xf0u, 0x66u, 0x67u, 0x6bu, 0xc1u, 0xfeu }
    };
    static const type_unsigned_8 lock_bytes[] = { 5u,4u,8u,5u,6u,5u,9u,6u };
    static const core_machine_cpu_profile pre386[] = {
        CORE_MACHINE_CPU_PROFILE_8086,
        CORE_MACHINE_CPU_PROFILE_80186,
        CORE_MACHINE_CPU_PROFILE_80286
    };
    type_unsigned_8 profile;

    if (!imul_s56_run(CORE_MACHINE_CPU_PROFILE_80386, dword_iw,
            sizeof(dword_iw), 2u, 0x80000000u, 1, 1)) {
        return 0;
    }
    if (!imul_s56_run(CORE_MACHINE_CPU_PROFILE_80386, dword_ib,
            sizeof(dword_ib), 0xfffffffeu, 4u, 0, 1)) {
        return 0;
    }
    if (!imul_s56_run(CORE_MACHINE_CPU_PROFILE_80386, inert, sizeof(inert),
            0xb2b2fffeu, 4u, 0, 0)) {
        return 0;
    }
    if (!imul_s56_run(CORE_MACHINE_CPU_PROFILE_80386, combined,
            sizeof(combined), 0xfffffffeu, 4u, 0, 1)) {
        return 0;
    }
    for (profile = 0u; profile != sizeof(lock_bytes); ++profile) {
        if (!imul_s56_expect_ud(CORE_MACHINE_CPU_PROFILE_80386, lock[profile],
                lock_bytes[profile]))
            return 0;
    }
    for (profile = 0u; profile != sizeof(pre386) / sizeof(pre386[0]);
            ++profile) {
        if (!imul_s56_expect_ud(pre386[profile], dword_iw,
                sizeof(dword_iw)) || !imul_s56_expect_ud(pre386[profile],
                inert, sizeof(inert)) || !imul_s56_expect_ud(pre386[profile],
                combined, sizeof(combined))) {
            return 0;
        }
    }
    return 1;
}

static C_INT imul_s56_test_memory_forms(C_VOID)
{
    static const type_unsigned_8 word_iw[] = {
        0x69u, 0x06u, 0x00u, 0x40u, 0xfeu, 0xffu
    };
    static const type_unsigned_8 word_ib[] = {
        0x6bu, 0x06u, 0x00u, 0x40u, 0xfeu
    };
    static const type_unsigned_8 dword_iw[] = {
        0x66u, 0x67u, 0x69u, 0x05u, 0x00u, 0x40u, 0x00u, 0x00u,
        0xfeu, 0xffu, 0xffu, 0xffu
    };
    static const type_unsigned_8 dword_ib[] = {
        0x66u, 0x67u, 0x6bu, 0x05u, 0x00u, 0x40u, 0x00u, 0x00u,
        0xfeu
    };
    const type_unsigned_32 dword_source = 0xfffffffeu;
    const type_unsigned_16 word_source = 0xfffeu;
    const type_unsigned_8 *codes[] = { word_iw, word_ib, dword_iw, dword_ib };
    const type_unsigned_8 sizes[] = {
        sizeof(word_iw), sizeof(word_ib), sizeof(dword_iw), sizeof(dword_ib)
    };
    static const core_machine_cpu_profile word_profiles[] = {
        CORE_MACHINE_CPU_PROFILE_80186, CORE_MACHINE_CPU_PROFILE_80286,
        CORE_MACHINE_CPU_PROFILE_80386
    };
    type_unsigned_8 form;

    for (form = 0u; form != sizeof(codes) / sizeof(codes[0]); ++form) {
        const type_unsigned_8 profile_count = form < 2u ?
            sizeof(word_profiles) / sizeof(word_profiles[0]) : 1u;
        type_unsigned_8 profile_index;

        for (profile_index = 0u; profile_index != profile_count;
            ++profile_index) {
        core_machine *machine = STD_NULL;
        core_machine_run_result result;
        core_machine_cpu_diagnostic diagnostic;
        t_cpu before;
        t_cpu after;
        type_unsigned_32 source_after = 0u;
        C_INT dword = form >= 2u;
        C_INT failed = !imul_s56_prepare(&machine, form < 2u ?
            word_profiles[profile_index] : CORE_MACHINE_CPU_PROFILE_80386);

        if (!failed) {
            imul_s56_seed(&machine->executor_cpu);
            before = machine->executor_cpu;
            failed = core_machine_memory_write(machine, 0u, codes[form],
                sizes[form]) != TYPE_STATUS_OK;
            failed |= core_machine_memory_write(machine, 0x4000u,
                dword ? (const C_VOID *)&dword_source : (const C_VOID *)&word_source,
                dword ? 4u : 2u) != TYPE_STATUS_OK;
            if (!failed) {
                failed = core_machine_run(machine,
                    (core_machine_run_budget){ 1u, 0u }, &result) !=
                    TYPE_STATUS_OK;
                after = test_core_machine_fixture_capture_cpu_after_run(machine);
                failed |= result.reason != CORE_MACHINE_STOP_BUDGET;
                failed |= core_machine_get_cpu_diagnostic(machine,
                    &diagnostic) != TYPE_STATUS_OK;
                failed |= diagnostic.first_fault.valid;
                failed |= after.data.eip != sizes[form];
                failed |= dword ? after.data.eax != 4u : after.data.eax !=
                    ((before.data.eax & 0xffff0000u) | 4u);
                failed |= !imul_s56_nonparticipants_same(&before, &after);
                failed |= !imul_s56_sregs_same(&before, &after);
                failed |= !!TYPE_GET_BIT(after.data.eflags, VCPU_EFLAGS_CF);
                failed |= !!TYPE_GET_BIT(after.data.eflags, VCPU_EFLAGS_OF);
                failed |= !imul_s56_nonarithmetic_flags_same(&before, &after);
                failed |= core_machine_memory_read(machine, 0x4000u,
                    &source_after, dword ? 4u : 2u) != TYPE_STATUS_OK;
                failed |= dword ? source_after != dword_source :
                    (type_unsigned_16)source_after != word_source;
            }
        }
        core_machine_destroy(machine);
        if (failed) {
            return 0;
        }
        }
    }
    return 1;
}

static C_INT imul_s56_test_segments(C_VOID)
{
    static const type_unsigned_8 codes[][7] = {
        { 0x69u, 0x06u, 0x10u, 0x00u, 0xfeu, 0xffu, 0u },
        { 0x69u, 0x46u, 0x00u, 0xfeu, 0xffu, 0u, 0u },
        { 0x2eu, 0x69u, 0x06u, 0x10u, 0x00u, 0xfeu, 0xffu },
        { 0x26u, 0x69u, 0x06u, 0x10u, 0x00u, 0xfeu, 0xffu },
        { 0x64u, 0x69u, 0x06u, 0x10u, 0x00u, 0xfeu, 0xffu },
        { 0x65u, 0x69u, 0x06u, 0x10u, 0x00u, 0xfeu, 0xffu }
    };
    static const type_unsigned_8 bytes[] = { 6u, 5u, 7u, 7u, 7u, 7u };
    type_unsigned_8 form;

    for (form = 0u; form != sizeof(bytes); ++form) {
        core_machine *machine = STD_NULL;
        core_machine_run_result result;
        core_machine_cpu_diagnostic diagnostic;
        t_cpu before;
        t_cpu after;
        t_cpu_data_sreg *segment;
        type_unsigned_16 selector = (type_unsigned_16)(0x1000u + form * 0x1000u);
        type_unsigned_32 address = ((type_unsigned_32)selector << 4u) + 0x10u;
        type_unsigned_32 code_address = 0u;
        const type_unsigned_16 source = 0xfffeu;
        C_INT failed = !imul_s56_prepare(&machine,
            CORE_MACHINE_CPU_PROFILE_80386);

        if (!failed) {
            imul_s56_seed(&machine->executor_cpu);
            segment = &machine->executor_cpu.data.ds;
            if (form == 1u)
                segment = &machine->executor_cpu.data.ss;
            else if (form == 2u)
                segment = &machine->executor_cpu.data.cs;
            else if (form == 3u)
                segment = &machine->executor_cpu.data.es;
            else if (form == 4u)
                segment = &machine->executor_cpu.data.fs;
            else if (form == 5u)
                segment = &machine->executor_cpu.data.gs;
            segment->selector = selector;
            segment->base = (type_unsigned_32)selector << 4u;
            if (form == 2u)
                code_address = segment->base;
            if (form == 1u)
                machine->executor_cpu.data.ebp = 0x00000010u;
            before = machine->executor_cpu;
            failed = core_machine_memory_write(machine, code_address, codes[form],
                bytes[form]) != TYPE_STATUS_OK;
            failed |= core_machine_memory_write(machine, address, &source,
                sizeof(source)) != TYPE_STATUS_OK;
            if (!failed) {
                failed = core_machine_run(machine,
                    (core_machine_run_budget){ 1u, 0u }, &result) !=
                    TYPE_STATUS_OK;
                after = test_core_machine_fixture_capture_cpu_after_run(machine);
                failed |= result.reason != CORE_MACHINE_STOP_BUDGET;
                failed |= core_machine_get_cpu_diagnostic(machine,
                    &diagnostic) != TYPE_STATUS_OK;
                failed |= diagnostic.first_fault.valid;
                failed |= after.data.eip != bytes[form];
            failed |= after.data.eax !=
                ((before.data.eax & 0xffff0000u) | 4u);
            failed |= !imul_s56_nonparticipants_same(&before, &after);
            failed |= !imul_s56_sregs_same(&before, &after);
            failed |= !imul_s56_nonarithmetic_flags_same(&before, &after);
            }
        }
        core_machine_destroy(machine);
        if (failed) {
            return 0;
        }
    }
    return 1;
}

static C_INT imul_s56_test_67_sib_ss(C_VOID)
{
    static const type_unsigned_8 code[] = {
        0x67u, 0x69u, 0x44u, 0x24u, 0x10u, 0xfeu, 0xffu
    };
    const type_unsigned_16 source = 0xfffeu;
    core_machine *machine = STD_NULL;
    core_machine_run_result result;
    core_machine_cpu_diagnostic diagnostic;
    t_cpu before;
    t_cpu after;
    C_INT failed = !imul_s56_prepare(&machine,
        CORE_MACHINE_CPU_PROFILE_80386);

    if (!failed) {
        imul_s56_seed(&machine->executor_cpu);
        machine->executor_cpu.data.ss.selector = 0x2000u;
        machine->executor_cpu.data.ss.base = 0x20000u;
        machine->executor_cpu.data.esp = 0x00000000u;
        before = machine->executor_cpu;
        failed = core_machine_memory_write(machine, 0u, code,
            sizeof(code)) != TYPE_STATUS_OK;
        failed |= core_machine_memory_write(machine, 0x20010u, &source,
            sizeof(source)) != TYPE_STATUS_OK;
        if (!failed) {
            failed = core_machine_run(machine,
                (core_machine_run_budget){ 1u, 0u }, &result) != TYPE_STATUS_OK;
            after = test_core_machine_fixture_capture_cpu_after_run(machine);
            failed |= result.reason != CORE_MACHINE_STOP_BUDGET;
            failed |= core_machine_get_cpu_diagnostic(machine, &diagnostic) !=
                TYPE_STATUS_OK;
            failed |= diagnostic.first_fault.valid;
            failed |= after.data.eip != sizeof(code);
            failed |= after.data.eax !=
                ((before.data.eax & 0xffff0000u) | 4u);
            failed |= after.data.ecx != before.data.ecx;
            failed |= after.data.edx != before.data.edx;
            failed |= after.data.ebx != before.data.ebx;
            failed |= after.data.ebp != before.data.ebp;
            failed |= after.data.esi != before.data.esi;
            failed |= after.data.edi != before.data.edi;
            failed |= !imul_s56_sregs_same(&before, &after);
        }
    }
    core_machine_destroy(machine);
    return !failed;
}

static C_INT imul_s56_test_protected_source_limits(C_VOID)
{
    static const type_unsigned_8 gdtr[] = { 0x1fu, 0u, 0x00u, 0x03u, 0u, 0u };
    static const type_unsigned_8 gdt[] = {
        0,0,0,0,0,0,0,0, 0xff,0xff,0,0x20,0,0x9a,0,0,
        0x0f,0,0,0x30,0,0x92,0,0, 0xff,0xff,0,0x40,0,0x92,0,0
    };
    static const type_unsigned_8 boot[] = {
        0x0f,1,0x16,0,1, 0xb8,1,0, 0x0f,1,0xf0, 0xb8,0x10,0,
        0x8e,0xd8, 0x8e,0xc0, 0xb8,0x18,0, 0x8e,0xd0, 0xbc,0,0x80,
        0xea,0,0,8,0
    };
    static const type_unsigned_8 code[] = { 0x69u, 0x06u, 0x10u, 0u, 0xfeu, 0xffu };
    static const type_unsigned_8 ss_code[] = { 0x69u, 0x46u, 0u, 0xfeu, 0xffu };
    static const type_unsigned_8 hlt = 0xf4u;
    type_unsigned_8 form;

    for (form = 0u; form != 2u; ++form) {
        core_machine *machine = STD_NULL;
        core_machine_run_result result;
        core_machine_cpu_diagnostic diagnostic;
        t_cpu before;
        t_cpu after;
        type_status status;
        C_INT failed = !imul_s56_prepare(&machine,
            CORE_MACHINE_CPU_PROFILE_80386);

        if (!failed) {
            failed = core_machine_memory_write(machine, 0x100u, gdtr,
                sizeof(gdtr)) != TYPE_STATUS_OK;
            failed |= core_machine_memory_write(machine, 0x300u, gdt,
                sizeof(gdt)) != TYPE_STATUS_OK;
            failed |= core_machine_memory_write(machine, 0u, boot,
                sizeof(boot)) != TYPE_STATUS_OK;
            failed |= core_machine_memory_write(machine, 0x2000u, &hlt,
                sizeof(hlt)) != TYPE_STATUS_OK;
            failed |= core_machine_run(machine,
                (core_machine_run_budget){ 96u, 0u }, &result) != TYPE_STATUS_OK;
            failed |= result.reason != CORE_MACHINE_STOP_WAITING_FOR_INTERRUPT;
        }
        if (!failed) {
            test_core_machine_fixture_resume_after_halt_at(machine, 0u);
            imul_s56_seed(&machine->executor_cpu);
            if (form) {
                machine->executor_cpu.data.ebp = 0x10u;
                machine->executor_cpu.data.ss.limit = 0x0fu;
            }
            before = machine->executor_cpu;
            failed = core_machine_memory_write(machine, 0x2000u,
                form ? ss_code : code, form ? sizeof(ss_code) : sizeof(code)) !=
                TYPE_STATUS_OK;
            status = core_machine_run(machine,
                (core_machine_run_budget){ 1u, 0u }, &result);
            failed |= status != TYPE_STATUS_FAULT;
            after = test_core_machine_fixture_capture_cpu_after_run(machine);
            failed |= result.reason != CORE_MACHINE_STOP_FAULT;
            failed |= core_machine_get_cpu_diagnostic(machine, &diagnostic) !=
                TYPE_STATUS_OK;
            failed |= !diagnostic.first_fault.valid;
            failed |= !TYPE_GET_BIT(diagnostic.first_fault.exception_mask,
                VCPUINS_EXCEPT_DF);
            failed |= after.data.eip != 0u;
            failed |= after.data.eax != before.data.eax;
            failed |= !imul_s56_nonparticipants_same(&before, &after);
            failed |= after.data.eflags != before.data.eflags;
            failed |= !imul_s56_sregs_same(&before, &after);
        }
        core_machine_destroy(machine);
        if (failed) {
            return 0;
        }
    }
    return 1;
}

static C_INT imul_s56_test_vm86(C_VOID)
{
    static const type_unsigned_8 code[] = { 0x69u, 0xc1u, 0xfeu, 0xffu };
    core_machine *machine = STD_NULL;
    core_machine_run_result result;
    core_machine_cpu_diagnostic diagnostic;
    t_cpu before;
    t_cpu after;
    C_INT failed = !imul_s56_prepare(&machine,
        CORE_MACHINE_CPU_PROFILE_80386);

    if (!failed) {
        imul_s56_seed(&machine->executor_cpu);
        machine->executor_cpu.data.cr0 |= VCPU_CR0_PE;
        machine->executor_cpu.data.eflags = VCPU_EFLAGS_VM |
            VCPU_EFLAGS_IOPL | VCPU_EFLAGS_IF;
        machine->executor_cpu.data.cs.selector = 0u;
        machine->executor_cpu.data.cs.base = 0u;
        machine->executor_cpu.data.cs.limit = 0xffffu;
        machine->executor_cpu.data.cs.dpl = 3u;
        machine->executor_cpu.data.cs.flagValid = TYPE_TRUE;
        machine->executor_cpu.data.cs.seg.exec.defsize = TYPE_FALSE;
        machine->executor_cpu.data.ds.selector = 0u;
        machine->executor_cpu.data.ds.base = 0u;
        machine->executor_cpu.data.ds.limit = 0xffffu;
        machine->executor_cpu.data.ds.dpl = 3u;
        machine->executor_cpu.data.ds.flagValid = TYPE_TRUE;
        machine->executor_cpu.data.es.selector = 0u;
        machine->executor_cpu.data.es.base = 0u;
        machine->executor_cpu.data.es.limit = 0xffffu;
        machine->executor_cpu.data.es.dpl = 3u;
        machine->executor_cpu.data.es.flagValid = TYPE_TRUE;
        machine->executor_cpu.data.ss.selector = 0u;
        machine->executor_cpu.data.ss.base = 0u;
        machine->executor_cpu.data.ss.limit = 0xffffu;
        machine->executor_cpu.data.ss.dpl = 3u;
        machine->executor_cpu.data.ss.flagValid = TYPE_TRUE;
        machine->executor_cpu.data.ss.seg.data.big = TYPE_FALSE;
        before = machine->executor_cpu;
        failed = core_machine_memory_write(machine, 0u, code,
            sizeof(code)) != TYPE_STATUS_OK;
        if (!failed) {
            failed = core_machine_run(machine,
                (core_machine_run_budget){ 1u, 0u }, &result) != TYPE_STATUS_OK;
            after = test_core_machine_fixture_capture_cpu_after_run(machine);
            failed |= result.reason != CORE_MACHINE_STOP_BUDGET;
            failed |= core_machine_get_cpu_diagnostic(machine, &diagnostic) !=
                TYPE_STATUS_OK;
            failed |= diagnostic.first_fault.valid;
            failed |= after.data.eip != sizeof(code);
            failed |= after.data.eax !=
                ((before.data.eax & 0xffff0000u) | 4u);
            failed |= after.data.ecx != before.data.ecx;
            failed |= after.data.edx != before.data.edx;
            failed |= after.data.ebx != before.data.ebx;
            failed |= after.data.ebp != before.data.ebp;
            failed |= after.data.esi != before.data.esi;
            failed |= after.data.edi != before.data.edi;
            failed |= !imul_s56_sregs_same(&before, &after);
        }
    }
    core_machine_destroy(machine);
    return !failed;
}

static C_INT imul_s56_test_irq_no_shadow(C_VOID)
{
    static const type_unsigned_8 register_code[] = {
        0x69u, 0xc1u, 0xfeu, 0xffu, 0x90u
    };
    static const type_unsigned_8 memory_code[] = {
        0x69u, 0x06u, 0x00u, 0x40u, 0xfeu, 0xffu, 0x90u
    };
    static const type_unsigned_8 hlt = 0xf4u;
    const type_unsigned_16 vector_offset = 0x0100u;
    const type_unsigned_16 vector_segment = 0u;
    const type_unsigned_16 source = 0xfffeu;
    type_unsigned_8 form;

    for (form = 0u; form != 2u; ++form) {
        const type_unsigned_8 *code = form ? memory_code : register_code;
        const type_unsigned_8 instruction_bytes = form ? 6u : 4u;
        core_machine *machine = STD_NULL;
        core_machine_pic_irq_source source_irq;
        core_machine_run_result result;
        t_cpu before;
        t_cpu after;
        type_unsigned_16 frame_ip = 0u;
        type_unsigned_16 frame_flags = 0u;
        C_INT failed = !imul_s56_prepare(&machine,
            CORE_MACHINE_CPU_PROFILE_80386);

        if (!failed) {
            imul_s56_seed(&machine->executor_cpu);
            machine->executor_cpu.data.ecx = 0xb2b2fffeu;
            machine->executor_cpu.data.eflags |= VCPU_EFLAGS_IF;
            machine->executor_cpu.data.idtr.limit = 0x017fu;
            before = machine->executor_cpu;
            failed = core_machine_memory_write(machine, 0u, code,
                form ? sizeof(memory_code) : sizeof(register_code)) !=
                TYPE_STATUS_OK;
            failed |= core_machine_memory_write(machine, 0x20u * 4u,
                &vector_offset, sizeof(vector_offset)) != TYPE_STATUS_OK;
            failed |= core_machine_memory_write(machine, 0x20u * 4u + 2u,
                &vector_segment, sizeof(vector_segment)) != TYPE_STATUS_OK;
            failed |= core_machine_memory_write(machine, 0x0100u, &hlt,
                sizeof(hlt)) != TYPE_STATUS_OK;
            if (form) {
                failed |= core_machine_memory_write(machine, 0x4000u, &source,
                    sizeof(source)) != TYPE_STATUS_OK;
            }
        }
        if (!failed) {
            STD_MEMSET(&source_irq, 0, sizeof(source_irq));
            machine->shared_pic_master.data.icw2 = 0x20u;
            core_machine_pic_irq_source_bind(&source_irq,
                &machine->shared_pic_master, &machine->shared_pic_slave, 0u);
            core_machine_pic_irq_source_assert(&source_irq);
            core_machine_pic_irq_source_deassert(&source_irq);
            failed = core_machine_run(machine,
                (core_machine_run_budget){ 2u, 0u }, &result) != TYPE_STATUS_OK;
            after = test_core_machine_fixture_capture_cpu_after_run(machine);
            failed |= result.reason != CORE_MACHINE_STOP_WAITING_FOR_INTERRUPT;
            failed |= after.data.eip != 0x0101u;
            failed |= after.data.eax !=
                ((before.data.eax & 0xffff0000u) | 4u);
            failed |= after.data.ecx != before.data.ecx;
            failed |= after.data.edx != before.data.edx;
            failed |= after.data.ebx != before.data.ebx;
            failed |= after.data.ebp != before.data.ebp;
            failed |= after.data.esi != before.data.esi;
            failed |= after.data.edi != before.data.edi;
            failed |= !imul_s56_sregs_same(&before, &after);
            failed |= core_machine_memory_read_physical(&machine->executor_memory,
                after.data.ss.base + (type_unsigned_16)after.data.esp,
                (type_virtual_address)&frame_ip, sizeof(frame_ip)) != TYPE_STATUS_OK;
            failed |= core_machine_memory_read_physical(&machine->executor_memory,
                after.data.ss.base + (type_unsigned_16)after.data.esp + 4u,
                (type_virtual_address)&frame_flags, sizeof(frame_flags)) !=
                TYPE_STATUS_OK;
            failed |= frame_ip != instruction_bytes;
            failed |= (frame_flags & ~(VCPU_EFLAGS_CF | VCPU_EFLAGS_OF |
                VCPU_EFLAGS_SF | VCPU_EFLAGS_ZF | VCPU_EFLAGS_AF |
                VCPU_EFLAGS_PF)) != ((before.data.eflags &
                ~(VCPU_EFLAGS_CF | VCPU_EFLAGS_OF | VCPU_EFLAGS_SF |
                VCPU_EFLAGS_ZF | VCPU_EFLAGS_AF | VCPU_EFLAGS_PF |
                VCPU_EFLAGS_RESERVED)) | 0x02u);
            failed |= TYPE_GET_BIT(frame_flags, VCPU_EFLAGS_CF);
            failed |= TYPE_GET_BIT(frame_flags, VCPU_EFLAGS_OF);
            failed |= !TYPE_GET_BIT(machine->shared_pic_master.data.isr,
                VPIC_ISR_IRQ(0u));
            failed |= TYPE_GET_BIT(machine->shared_pic_master.data.irr,
                VPIC_IRR_IRQ(0u));
        }
        if (failed) {
            core_machine_destroy(machine);
            return 0;
        }
        core_machine_destroy(machine);
    }
    return 1;
}

C_INT main(C_VOID)
{
    if (!imul_s56_expect_ud(CORE_MACHINE_CPU_PROFILE_8086,
            (const type_unsigned_8[]){ 0x69u, 0xc1u, 0xfeu, 0xffu }, 4u)) {
        return 1;
    }
    if (!imul_s56_expect_ud(CORE_MACHINE_CPU_PROFILE_8086,
            (const type_unsigned_8[]){ 0x6bu, 0xc1u, 0xfeu }, 3u)) {
        return 1;
    }
    if (!imul_s56_test_defaults()) {
        return 1;
    }
    if (!imul_s56_test_attributes_and_rejects() ||
        !imul_s56_test_memory_forms() || !imul_s56_test_segments() ||
        !imul_s56_test_67_sib_ss() || !imul_s56_test_protected_source_limits() ||
        !imul_s56_test_vm86() || !imul_s56_test_irq_no_shadow()) {
        return 1;
    }
    STD_PRINTF("M5:T316:S56:IMUL-IMM:OK\n");
    STD_PRINTF("M5:T401:S30:IMUL-IMMEDIATE-PROFILES:OK\n");
    return 0;
}
