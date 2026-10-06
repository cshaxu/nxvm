#include "support/cpu_instruction_fixture.h"
#include "lib/types/file.h"

static lib_i32 control_run(cpu_instruction_fixture *fixture,
    const lib_u8 *code, lib_u8 bytes, t_cpu *after)
{
    lib_memory_copy(fixture->memory, code, bytes);
    core_machine_cpu_execution_refresh(&fixture->execution);
    *after = fixture->cpu;
    return !fixture->execution.stop_requested && !fixture->fault.valid;
}

static lib_i32 control_fault(cpu_instruction_fixture *fixture,
    const lib_u8 *code, lib_u8 bytes, lib_u32 exception, t_cpu *after)
{
    (void)cpu_instruction_run(fixture, code, bytes, after);
    if (fixture->execution.cpu_profile >= CORE_MACHINE_CPU_PROFILE_80386 &&
        (fixture->cpu.data.cr0 & VCPU_CR0_PE) != 0u &&
        exception == VCPUINS_EXCEPT_GP) exception = VCPUINS_EXCEPT_DF;
    return fixture->execution.stop_requested && fixture->fault.valid &&
        (fixture->fault.exception_mask & exception) != 0u;
}

static void control_seed(cpu_instruction_fixture *fixture)
{
    t_cpu *cpu = &fixture->cpu;

    cpu->data.eax = 0xdeadbeefu;
    cpu->data.ecx = 0x11223344u;
    cpu->data.edx = 0x55667788u;
    cpu->data.ebx = 0x99aabbccu;
    cpu->data.esp = 0x8000u;
    cpu->data.ebp = 0x0120u;
    cpu->data.esi = 0x0010u;
    cpu->data.edi = 0x0020u;
    cpu->data.eflags = VCPU_EFLAGS_IF | VCPU_EFLAGS_CF | VCPU_EFLAGS_PF |
        VCPU_EFLAGS_AF | VCPU_EFLAGS_ZF | VCPU_EFLAGS_SF | VCPU_EFLAGS_OF;
}

static void control_enter_protected(cpu_instruction_fixture *fixture,
    lib_u8 cpl, lib_bool vm86)
{
    t_cpu *cpu = &fixture->cpu;

    cpu->data.cr0 |= VCPU_CR0_PE;
    cpu->data.cs.selector = (lib_u16)(0x0008u | cpl);
    cpu->data.cs.base = 0u;
    cpu->data.cs.limit = 0xffffu;
    cpu->data.cs.dpl = cpl;
    cpu->data.cs.flagValid = LIB_TRUE;
    cpu->data.cs.sregtype = SREG_CODE;
    cpu->data.cs.seg.executable = LIB_TRUE;
    if (vm86) {
        cpu->data.eflags |= VCPU_EFLAGS_VM;
        cpu->data.cs.dpl = 3u;
    }
}

static lib_i32 control_state_equal(const t_cpu *before, const t_cpu *after)
{
    return before->data.eax == after->data.eax &&
        before->data.ecx == after->data.ecx &&
        before->data.edx == after->data.edx &&
        before->data.ebx == after->data.ebx &&
        before->data.esp == after->data.esp &&
        before->data.ebp == after->data.ebp &&
        before->data.esi == after->data.esi &&
        before->data.edi == after->data.edi &&
        before->data.eflags == after->data.eflags;
}

static lib_i32 control_test_clts(void)
{
    static const core_machine_cpu_profile profiles[] = {
        CORE_MACHINE_CPU_PROFILE_80286, CORE_MACHINE_CPU_PROFILE_80386
    };
    static const lib_u8 forms[][5] = {
        {0x0fu,0x06u}, {0x66u,0x0fu,0x06u},
        {0x67u,0x0fu,0x06u}, {0x66u,0x67u,0x0fu,0x06u}
    };
    static const lib_u8 form_bytes[] = {2u,3u,3u,4u};
    lib_size profile, form;

    for (profile = 0u; profile != 2u; ++profile)
        for (form = 0u; form != (profile == 0u ? 1u : 4u); ++form) {
            cpu_instruction_fixture fixture;
            t_cpu before, after;

            cpu_instruction_prepare(&fixture, profiles[profile]);
            control_seed(&fixture);
            fixture.cpu.data.cr0 = VCPU_CR0_TS | 0x0du;
            before = fixture.cpu;
            if (!control_run(&fixture, forms[form], form_bytes[form], &after) ||
                after.data.eip != form_bytes[form] ||
                !control_state_equal(&before, &after) || after.data.cr0 !=
                (before.data.cr0 & ~VCPU_CR0_TS)) return 0;
        }
    for (profile = 0u; profile != 2u; ++profile) {
        cpu_instruction_fixture fixture;
        t_cpu before, after;

        cpu_instruction_prepare(&fixture, CORE_MACHINE_CPU_PROFILE_80386);
        control_seed(&fixture);
        control_enter_protected(&fixture, 3u, profile != 0u);
        fixture.cpu.data.cr0 |= VCPU_CR0_TS;
        before = fixture.cpu;
        if (!control_fault(&fixture, forms[0], form_bytes[0],
                VCPUINS_EXCEPT_GP, &after) || !control_state_equal(&before,
                &after) || after.data.cr0 != before.data.cr0) return 0;
    }
    return 1;
}

static lib_i32 control_test_msw(void)
{
    static const lib_u8 smsw[] = {0x0fu,0x01u,0xe0u};
    static const lib_u8 lmsw[] = {0x0fu,0x01u,0xf0u};
    static const lib_u8 smsw_memory[] = {0x0fu,0x01u,0x26u,0x00u,0x04u};
    static const lib_u8 lmsw_memory[] = {0x0fu,0x01u,0x36u,0x00u,0x04u};
    const core_machine_cpu_profile profiles[] = {
        CORE_MACHINE_CPU_PROFILE_80286, CORE_MACHINE_CPU_PROFILE_80386
    };
    lib_size profile;

    for (profile = 0u; profile != 2u; ++profile) {
        cpu_instruction_fixture fixture;
        t_cpu before, after;
        lib_u32 image = 0x11223344u;

        cpu_instruction_prepare(&fixture, profiles[profile]);
        fixture.cpu.data.eax = 0xdead0000u;
        before = fixture.cpu;
        if (!control_run(&fixture, smsw, sizeof(smsw), &after) ||
            after.data.eax != (profile == 0u ? 0xdeadfff0u : 0xdead0000u) ||
            after.data.cr0 != 0u || after.data.eflags != before.data.eflags) return 0;
        cpu_instruction_prepare(&fixture, profiles[profile]);
        control_seed(&fixture);
        fixture.cpu.data.cr0 = 0x00a5000du;
        before = fixture.cpu;
        if (!control_run(&fixture, smsw, sizeof(smsw), &after) ||
            after.data.eax != (profile == 0u ? 0xdeadfffdu : 0xdead000du) || after.data.cr0 !=
            before.data.cr0 || after.data.eflags != before.data.eflags) return 0;
        cpu_instruction_prepare(&fixture, profiles[profile]);
        control_seed(&fixture);
        fixture.cpu.data.eax = 0xdead000du;
        fixture.cpu.data.cr0 = 0x00a50000u;
        before = fixture.cpu;
        if (!control_run(&fixture, lmsw, sizeof(lmsw), &after) ||
            !control_state_equal(&before, &after) || after.data.cr0 !=
            0x00a5000du) return 0;
        fixture.cpu.data.eip = 0u;
        before = fixture.cpu;
        if (!control_run(&fixture, smsw, sizeof(smsw), &after) ||
            after.data.eax != (profile == 0u ? 0xdeadfffdu : 0xdead000du) ||
            after.data.cr0 != before.data.cr0 || after.data.eflags != before.data.eflags)
            return 0;
        cpu_instruction_prepare(&fixture, profiles[profile]);
        control_seed(&fixture);
        fixture.cpu.data.cr0 = 0x00a5000cu;
        lib_memory_copy(fixture.memory + 0x0400u, &image, sizeof(image));
        before = fixture.cpu;
        if (!control_run(&fixture, smsw_memory, sizeof(smsw_memory), &after) ||
            !control_state_equal(&before, &after)) return 0;
        lib_memory_copy(&image, fixture.memory + 0x0400u, sizeof(image));
        if (image != (profile == 0u ? 0x1122fffcu : 0x1122000cu)) return 0;
        cpu_instruction_prepare(&fixture, profiles[profile]);
        control_seed(&fixture);
        fixture.cpu.data.cr0 = 0x00a50000u;
        image = 0x1122000cu;
        lib_memory_copy(fixture.memory + 0x0400u, &image, sizeof(image));
        if (!control_run(&fixture, lmsw_memory, sizeof(lmsw_memory), &after) ||
            after.data.cr0 != 0x00a5000cu) return 0;
    }
    return 1;
}

static lib_i32 control_test_msw_attributes(void)
{
    static const lib_u8 forms[][6] = {
        {0x66u,0x0fu,0x01u,0xe0u}, {0x67u,0x0fu,0x01u,0xe0u},
        {0x66u,0x67u,0x0fu,0x01u,0xe0u},
        {0x66u,0x0fu,0x01u,0xf0u}, {0x67u,0x0fu,0x01u,0xf0u},
        {0x66u,0x67u,0x0fu,0x01u,0xf0u}
    };
    static const lib_u8 bytes[] = {4u,4u,5u,4u,4u,5u};
    lib_size index;

    for (index = 0u; index != 6u; ++index) {
        cpu_instruction_fixture fixture;
        t_cpu before, after;

        cpu_instruction_prepare(&fixture, CORE_MACHINE_CPU_PROFILE_80386);
        control_seed(&fixture);
        fixture.cpu.data.eax = 0xdead000du;
        fixture.cpu.data.cr0 = 0x00a50000u;
        before = fixture.cpu;
        if (!control_run(&fixture, forms[index], bytes[index], &after) ||
            after.data.eip != bytes[index] || after.data.eflags !=
            before.data.eflags) return 0;
        if (index < 3u) {
            if (after.data.eax != 0xdead0000u || after.data.cr0 !=
                before.data.cr0) return 0;
        } else if (!control_state_equal(&before, &after) || after.data.cr0 !=
            0x00a5000du) return 0;
    }
    return 1;
}

static lib_i32 control_test_protected_state(void)
{
    static const core_machine_cpu_profile profiles[] = {
        CORE_MACHINE_CPU_PROFILE_80286, CORE_MACHINE_CPU_PROFILE_80386
    };
    static const lib_u8 lmsw[] = {0x0fu,0x01u,0xf0u};
    static const lib_u8 clts[] = {0x0fu,0x06u};
    lib_size index;

    for (index = 0u; index != 2u; ++index) {
        cpu_instruction_fixture fixture;
        t_cpu before, after;

        cpu_instruction_prepare(&fixture, profiles[index]);
        control_seed(&fixture);
        control_enter_protected(&fixture, 0u, LIB_FALSE);
        fixture.cpu.data.eax = 0xdead0000u;
        fixture.cpu.data.cr0 |= VCPU_CR0_TS;
        before = fixture.cpu;
        if (!control_run(&fixture, lmsw, sizeof(lmsw), &after) ||
            !control_state_equal(&before, &after) || after.data.cr0 !=
            (before.data.cr0 & ~VCPU_CR0_TS)) return 0;
        cpu_instruction_prepare(&fixture, profiles[index]);
        control_seed(&fixture);
        control_enter_protected(&fixture, 0u, LIB_FALSE);
        fixture.cpu.data.cr0 |= VCPU_CR0_TS;
        before = fixture.cpu;
        if (!control_run(&fixture, clts, sizeof(clts), &after) ||
            !control_state_equal(&before, &after) || after.data.cr0 !=
            (before.data.cr0 & ~VCPU_CR0_TS)) return 0;
    }
    return 1;
}

static lib_i32 control_test_privilege_and_rollback(void)
{
    static const lib_u8 smsw[] = {0x0fu,0x01u,0xe0u};
    static const lib_u8 lmsw[] = {0x0fu,0x01u,0xf0u};
    static const lib_u8 smsw_memory[] = {0x0fu,0x01u,0x26u,0x10u,0u};
    static const lib_u8 lmsw_memory[] = {0x0fu,0x01u,0x36u,0x10u,0u};
    const lib_u8 *codes[] = {lmsw, lmsw};
    lib_size index;

    for (index = 0u; index != 2u; ++index) {
        cpu_instruction_fixture fixture;
        t_cpu before, after;

        cpu_instruction_prepare(&fixture, CORE_MACHINE_CPU_PROFILE_80386);
        control_seed(&fixture);
        control_enter_protected(&fixture, 3u, index != 0u);
        fixture.cpu.data.cr0 |= VCPU_CR0_TS;
        before = fixture.cpu;
        if (!control_fault(&fixture, codes[index], 3u, VCPUINS_EXCEPT_GP,
                &after) || !control_state_equal(&before, &after) ||
            after.data.cr0 != before.data.cr0) return 0;
    }
    {
        cpu_instruction_fixture fixture;
        t_cpu before, after;

        cpu_instruction_prepare(&fixture, CORE_MACHINE_CPU_PROFILE_80386);
        control_seed(&fixture);
        control_enter_protected(&fixture, 3u, LIB_FALSE);
        fixture.cpu.data.cr0 |= VCPU_CR0_TS;
        before = fixture.cpu;
        if (!control_run(&fixture, smsw, sizeof(smsw), &after) ||
            after.data.eax != 0xdead0009u || after.data.cr0 !=
            before.data.cr0 || after.data.eflags != before.data.eflags) return 0;
        cpu_instruction_prepare(&fixture, CORE_MACHINE_CPU_PROFILE_80386);
        control_seed(&fixture);
        control_enter_protected(&fixture, 0u, LIB_FALSE);
        fixture.cpu.data.ds.limit = 0x0fu;
        fixture.cpu.data.cr0 |= VCPU_CR0_TS;
        before = fixture.cpu;
        if (!control_fault(&fixture, smsw_memory, sizeof(smsw_memory),
                VCPUINS_EXCEPT_GP, &after) || !control_state_equal(&before,
                &after) || after.data.cr0 != before.data.cr0) return 0;
        cpu_instruction_prepare(&fixture, CORE_MACHINE_CPU_PROFILE_80386);
        control_seed(&fixture);
        control_enter_protected(&fixture, 0u, LIB_FALSE);
        fixture.cpu.data.ds.limit = 0x0fu;
        fixture.cpu.data.cr0 |= VCPU_CR0_TS;
        before = fixture.cpu;
        if (!control_fault(&fixture, lmsw_memory, sizeof(lmsw_memory),
                VCPUINS_EXCEPT_GP, &after) || !control_state_equal(&before,
                &after) || after.data.cr0 != before.data.cr0) return 0;
    }
    return 1;
}

static lib_i32 control_test_mov_cr(void)
{
    static const lib_u8 read[][5] = {
        {0x0fu,0x20u,0xc0u}, {0x0fu,0x20u,0xd0u}, {0x0fu,0x20u,0xd8u},
        {0x66u,0x0fu,0x20u,0xc0u}, {0x66u,0x0fu,0x20u,0xd0u},
        {0x66u,0x0fu,0x20u,0xd8u}
    };
    static const lib_u8 read_bytes[] = {3u,3u,3u,4u,4u,4u};
    static const lib_u8 write[][5] = {
        {0x0fu,0x22u,0xc0u}, {0x0fu,0x22u,0xd0u}, {0x0fu,0x22u,0xd8u},
        {0x66u,0x0fu,0x22u,0xc0u}, {0x66u,0x0fu,0x22u,0xd0u},
        {0x66u,0x0fu,0x22u,0xd8u}
    };
    static const lib_u8 write_bytes[] = {3u,3u,3u,4u,4u,4u};
    const lib_u32 values[] = {0x00000001u,0x12345678u,0x00123000u};
    static const lib_u8 invalid[][4] = {
        {0x0fu,0x20u,0xe0u}, {0x0fu,0x20u,0x00u},
        {0x0fu,0x22u,0xc0u}
    };
    lib_size index;

    for (index = 0u; index != 6u; ++index) {
        cpu_instruction_fixture fixture;
        t_cpu after;
        const lib_size control = index % 3u;

        cpu_instruction_prepare(&fixture, CORE_MACHINE_CPU_PROFILE_80386);
        control_seed(&fixture);
        fixture.cpu.data.cr0 = values[0];
        fixture.cpu.data.cr2 = values[1];
        fixture.cpu.data.cr3 = values[2];
        if (!control_run(&fixture, read[index], read_bytes[index], &after) ||
            after.data.eax != values[control]) return 0;
        cpu_instruction_prepare(&fixture, CORE_MACHINE_CPU_PROFILE_80386);
        control_seed(&fixture);
        fixture.cpu.data.eax = values[control];
        if (!control_run(&fixture, write[index], write_bytes[index], &after) ||
            (control == 0u && after.data.cr0 != values[control]) ||
            (control == 1u && after.data.cr2 != values[control]) ||
            (control == 2u && after.data.cr3 != values[control])) return 0;
    }
    for (index = 0u; index != 3u; ++index) {
        cpu_instruction_fixture fixture;
        t_cpu before, after;

        cpu_instruction_prepare(&fixture, CORE_MACHINE_CPU_PROFILE_80386);
        control_seed(&fixture);
        fixture.cpu.data.cr0 = 0x00000001u;
        fixture.cpu.data.cr2 = values[1];
        fixture.cpu.data.cr3 = values[2];
        fixture.cpu.data.eax = 0x80000000u;
        before = fixture.cpu;
        if (!control_fault(&fixture, invalid[index], sizeof(invalid[index]),
                VCPUINS_EXCEPT_UD, &after) || !control_state_equal(&before,
                &after) || after.data.cr0 != before.data.cr0 ||
            after.data.cr2 != before.data.cr2 || after.data.cr3 !=
            before.data.cr3) return 0;
    }
    {
        static const lib_u8 protected_read[] = {0x0fu,0x20u,0xc0u};
        cpu_instruction_fixture fixture;
        t_cpu before, after;

        cpu_instruction_prepare(&fixture, CORE_MACHINE_CPU_PROFILE_80386);
        control_seed(&fixture);
        control_enter_protected(&fixture, 3u, LIB_FALSE);
        fixture.cpu.data.cr2 = values[1];
        before = fixture.cpu;
        if (!control_fault(&fixture, protected_read, sizeof(protected_read),
                VCPUINS_EXCEPT_GP, &after) || !control_state_equal(&before,
                &after) || after.data.cr0 != before.data.cr0 ||
            after.data.cr2 != before.data.cr2 || after.data.cr3 !=
                before.data.cr3) return 0;
    }
    return 1;
}

static lib_i32 control_real_data_cache(const t_cpu_data_sreg *sreg,
    lib_u16 selector, t_cpu_data_sreg_type type)
{
    return sreg->flagValid && sreg->selector == selector &&
        sreg->base == (lib_u32)selector << 4u &&
        sreg->limit == 0xffffu && sreg->dpl == 0u &&
        sreg->sregtype == type && sreg->seg.accessed &&
        !sreg->seg.executable && sreg->seg.data.writable &&
        !sreg->seg.data.big && !sreg->seg.data.expdown;
}

/* The full cache assertions formerly mixed into the Core descriptor receiver
 * belong here: copied Core snapshots do not expose CPU cache bookkeeping. */
static lib_i32 control_test_leave_protected_mode(void)
{
    static const lib_u8 code[] = {
        0x0fu, 0x22u, 0xc0u, 0xeau, 0x0au, 0x00u, 0x00u, 0x00u,
        0x00u, 0x00u, 0xbbu, 0x48u, 0x00u, 0x8eu, 0xc3u, 0x8eu,
        0xd3u, 0x8eu, 0xdbu, 0xf4u
    };
    cpu_instruction_fixture fixture;
    t_cpu *cpu = &fixture.cpu;

    cpu_instruction_prepare(&fixture, CORE_MACHINE_CPU_PROFILE_80386);
    control_enter_protected(&fixture, 0u, LIB_FALSE);
    cpu->data.ds.base = 0u;
    cpu->data.ds.limit = 0xffffu;
    cpu->data.ds.selector = 0x0010u;
    cpu->data.ds.flagValid = LIB_TRUE;
    cpu->data.ds.sregtype = SREG_DATA;
    cpu->data.ds.seg.executable = LIB_FALSE;
    cpu->data.ds.seg.data.writable = LIB_TRUE;
    cpu->data.ds.dpl = 0u;
    cpu->data.cs.seg.exec.defsize = LIB_TRUE;
    cpu->data.ds.seg.data.big = LIB_TRUE;
    cpu->data.es.seg.data.big = LIB_TRUE;
    cpu->data.ss.seg.data.big = LIB_TRUE;
    cpu->data.eax = 0u;
    lib_memory_copy(fixture.memory, code, sizeof(code));
    for (lib_u32 index = 0u; index != 32u && !cpu->data.flagHalt &&
            !fixture.execution.stop_requested; ++index)
        core_machine_cpu_execution_refresh(&fixture.execution);
    return !fixture.execution.stop_requested && !fixture.fault.valid &&
        !fixture.delivered_exception.valid && cpu->data.flagHalt &&
        cpu->data.cr0 == 0u && cpu->data.cs.selector == 0u &&
        cpu->data.cs.base == 0u && cpu->data.cs.limit == 0xffffu &&
        cpu->data.cs.flagValid && cpu->data.cs.seg.accessed &&
        cpu->data.cs.seg.executable && !cpu->data.cs.seg.exec.defsize &&
        !cpu->data.cs.seg.exec.conform && cpu->data.cs.seg.exec.readable &&
        cpu->data.ebx == 0x00000048u &&
        control_real_data_cache(&cpu->data.es, 0x0048u, SREG_DATA) &&
        control_real_data_cache(&cpu->data.ss, 0x0048u, SREG_STACK) &&
        control_real_data_cache(&cpu->data.ds, 0x0048u, SREG_DATA);
}

/* Full private storage preservation belongs to the CPU, not a PC board
 * borrowing its execution context. Reject vector 6 to retain producer rollback. */
static lib_status control_cli_sti_read(void *opaque, lib_u32 address,
    void *destination, lib_u8 bytes,
    core_machine_cpu_memory_access_provenance provenance,
    lib_bool observe_only, lib_bool reset_fetch)
{
    if (address < 28u && (lib_u64)address + bytes > 24u)
        return LIB_STATUS_IO_ERROR;
    return cpu_instruction_read(opaque, address, destination, bytes,
        provenance, observe_only, reset_fetch);
}

static lib_i32 control_cli_sti_storage_preserved(const t_cpu *before,
    const t_cpu *after)
{
    return before->data.eax == after->data.eax &&
        before->data.ecx == after->data.ecx &&
        before->data.edx == after->data.edx &&
        before->data.ebx == after->data.ebx &&
        before->data.esp == after->data.esp &&
        before->data.ebp == after->data.ebp &&
        before->data.esi == after->data.esi &&
        before->data.edi == after->data.edi &&
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

static lib_i32 control_test_interrupt_control_storage(void)
{
    static const core_machine_cpu_profile profiles[] = {
        CORE_MACHINE_CPU_PROFILE_8086, CORE_MACHINE_CPU_PROFILE_80186,
        CORE_MACHINE_CPU_PROFILE_80286, CORE_MACHINE_CPU_PROFILE_80386
    };
    static const lib_u8 prefixes[][2] = {
        {0u,0u}, {0x66u,0u}, {0x67u,0u}, {0x66u,0x67u}
    };
    static const lib_u8 opcodes[] = {0xfau, 0xfbu, 0xf4u};
    const core_machine_cpu_bus_provider bus = {
        .read_memory = control_cli_sti_read,
        .write_memory = cpu_instruction_write,
        .interrupt_pending = cpu_instruction_interrupt_pending
    };
    cpu_instruction_fixture fixture;

    for (lib_size profile = 0u; profile != 4u; ++profile)
        for (lib_size form = 0u; form != 4u; ++form)
            for (lib_u8 lock = 0u; lock != 2u; ++lock)
                for (lib_size instruction = 0u; instruction != 3u; ++instruction) {
                    const lib_u8 opcode = opcodes[instruction];
                    lib_u8 code[4u] = {0u};
                    lib_u8 bytes = 0u;
                    lib_bool rejected = lock || (form != 0u && profile != 3u);
                    t_cpu before, after;
                    lib_status status;
                    lib_u32 expected;

                    if (lock && profile != 3u) continue;
                    if (lock) code[bytes++] = 0xf0u;
                    if (form != 0u) code[bytes++] = prefixes[form][0];
                    if (form == 3u) code[bytes++] = prefixes[form][1];
                    code[bytes++] = opcode;
                    cpu_instruction_prepare_with_bus(&fixture, profiles[profile],
                        &bus, &fixture);
                    control_seed(&fixture);
                    fixture.cpu.data.eflags |= VCPU_EFLAGS_DF;
                    if (opcode != 0xfau) fixture.cpu.data.eflags &= ~VCPU_EFLAGS_IF;
                    if (opcode == 0xf4u) fixture.cpu.data.eax = 0xaabbccddu;
                    before = fixture.cpu;
                    expected = opcode == 0xf4u ? before.data.eflags :
                        (before.data.eflags & ~VCPU_EFLAGS_IF) |
                        (opcode == 0xfbu ? VCPU_EFLAGS_IF : 0u);
                    status = cpu_instruction_run(&fixture, code, bytes, &after);
                    if (rejected) {
                        if (status != LIB_STATUS_INTERNAL_ERROR || !fixture.fault.valid ||
                            (fixture.fault.exception_mask & VCPUINS_EXCEPT_UD) == 0u ||
                            lib_memory_compare(&before, &after, sizeof(before)) != 0) {
                            lib_c_printf("CLI-STI storage profile=%zu form=%zu lock=%u opcode=%x status=%u fault=%x\n",
                                profile, form, lock, opcode, status, fixture.fault.exception_mask);
                            return 0;
                        }
                    } else if (status != LIB_STATUS_OK || fixture.fault.valid ||
                        after.data.eip != bytes || after.data.eflags != expected ||
                        after.data.flagHalt != (opcode == 0xf4u) ||
                        !control_cli_sti_storage_preserved(&before, &after)) {
                        lib_c_printf("CLI-STI storage profile=%zu form=%zu lock=%u opcode=%x status=%u flags=%x expected=%x\n",
                            profile, form, lock, opcode, status, after.data.eflags, expected);
                        return 0;
                    }
                }
    return 1;
}

static lib_i32 control_test_hlt_privilege_storage(void)
{
    static const lib_u8 code[] = {0xf4u};
    cpu_instruction_fixture fixture;
    t_cpu before, after;

    cpu_instruction_prepare(&fixture, CORE_MACHINE_CPU_PROFILE_80386);
    control_seed(&fixture);
    control_enter_protected(&fixture, 0u, LIB_FALSE);
    before = fixture.cpu;
    if (!control_run(&fixture, code, sizeof(code), &after) ||
        fixture.delivered_exception.valid || !after.data.flagHalt ||
        after.data.eip != 1u || after.data.eflags != before.data.eflags ||
        !control_cli_sti_storage_preserved(&before, &after))
        return 0;

    cpu_instruction_prepare(&fixture, CORE_MACHINE_CPU_PROFILE_80386);
    control_seed(&fixture);
    control_enter_protected(&fixture, 3u, LIB_FALSE);
    before = fixture.cpu;
    return control_fault(&fixture, code, sizeof(code), VCPUINS_EXCEPT_GP, &after) &&
        lib_memory_compare(&before, &after, sizeof(before)) == 0;
}

int main(void)
{
    if (!control_test_clts()) {
        lib_c_printf("control-state stage=clts\n");
        return 1;
    }
    if (!control_test_msw()) {
        lib_c_printf("control-state stage=msw\n");
        return 1;
    }
    if (!control_test_msw_attributes()) {
        lib_c_printf("control-state stage=msw-attributes\n");
        return 1;
    }
    if (!control_test_protected_state()) {
        lib_c_printf("control-state stage=protected\n");
        return 1;
    }
    if (!control_test_privilege_and_rollback()) {
        lib_c_printf("control-state stage=privilege-rollback\n");
        return 1;
    }
    if (!control_test_mov_cr()) {
        lib_c_printf("control-state stage=mov-cr\n");
        return 1;
    }
    if (!control_test_leave_protected_mode()) {
        lib_c_printf("control-state stage=leave-protected\n");
        return 1;
    }
    if (!control_test_interrupt_control_storage()) {
        lib_c_printf("control-state stage=interrupt-control-storage\n");
        return 1;
    }
    if (!control_test_hlt_privilege_storage()) {
        lib_c_printf("control-state stage=hlt-privilege-storage\n");
        return 1;
    }
    lib_c_printf("M5:T539:S45:CONTROL-STATE:OK\n");
    return 0;
}
