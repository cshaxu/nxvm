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
    const lib_status status = cpu_instruction_run(fixture, code, bytes, after);

    if (fixture->execution.cpu_profile >= CORE_MACHINE_CPU_PROFILE_80386 &&
        (fixture->cpu.data.cr0 & VCPU_CR0_PE) != 0u) {
        return status == LIB_STATUS_OK &&
            core_machine_cpu_is_shutdown(&fixture->execution) &&
            !fixture->fault.valid && fixture->delivered_exception.valid &&
            fixture->delivered_exception.exception_mask ==
                VCPUINS_EXCEPT_SHUTDOWN;
    }
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
                    if (rejected) {
                        status = cpu_instruction_run(&fixture, code, bytes, &after);
                        if (status != LIB_STATUS_INTERNAL_ERROR || !fixture.fault.valid ||
                            (fixture.fault.exception_mask & (profile == 0u ?
                                VCPUINS_EXCEPT_UD : VCPUINS_EXCEPT_CE)) == 0u ||
                            lib_memory_compare(&before, &after, sizeof(before)) != 0) {
                            lib_c_printf("CLI-STI storage profile=%zu form=%zu lock=%u opcode=%x status=%u fault=%x\n",
                                profile, form, lock, opcode, status, fixture.fault.exception_mask);
                            return 0;
                        }
                    } else if ((status = cpu_instruction_run(&fixture, code, bytes, &after)) != LIB_STATUS_OK || fixture.fault.valid ||
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

static lib_bool control_test_486_paged_tables(void)
{
    static const lib_u8 load_ds[] = {0x8eu, 0xd8u};
    static const lib_u8 interrupt[] = {0xccu};
    static const lib_u8 descriptors[] = {
        0xffu, 0xffu, 0u, 0u, 0u, 0x9bu, 0u, 0u,
        0xffu, 0xffu, 0u, 0x90u, 0u, 0x92u, 0u, 0u
    };
    static const lib_u8 gate[] = {0u, 1u, 8u, 0u, 0u, 0x8eu, 0u, 0u};
    cpu_instruction_fixture fixture;
    t_cpu after;
    lib_u32 entry = 0x2003u;
    lib_u32 page;

    cpu_instruction_prepare(&fixture, CORE_MACHINE_CPU_PROFILE_80486SX);
    control_enter_protected(&fixture, 0u, LIB_FALSE);
    fixture.cpu.data.cr0 |= VCPU_CR0_PG | VCPU_CR0_WP;
    fixture.cpu.data.cr3 = 0x1000u;
    fixture.cpu.data.gdtr.base = 0x4000u;
    fixture.cpu.data.gdtr.limit = 0x17u;
    fixture.cpu.data.idtr.base = 0x5000u;
    fixture.cpu.data.idtr.limit = 0x7ffu;
    fixture.cpu.data.esp = 0x8000u;
    fixture.cpu.data.eax = 0x10u;
    lib_memory_copy(fixture.memory + 0x1000u, &entry, sizeof(entry));
    for (page = 0u; page < 16u; ++page) {
        entry = (page << 12u) | 3u;
        if (page == 4u || page == 5u) entry += 0x2000u;
        lib_memory_copy(fixture.memory + 0x2000u + page * 4u,
            &entry, sizeof(entry));
    }
    /* The unmapped table addresses remain zero: a physical-table bypass
     * cannot accidentally find a valid descriptor or interrupt gate. */
    lib_memory_copy(fixture.memory + 0x6008u, descriptors, sizeof(descriptors));
    lib_memory_copy(fixture.memory + 0x7018u, gate, sizeof(gate));
    lib_memory_copy(fixture.memory + 0x7070u, gate, sizeof(gate));
    if (!control_run(&fixture, load_ds, sizeof(load_ds), &after) ||
        fixture.delivered_exception.valid || after.data.ds.selector != 0x10u ||
        after.data.ds.base != 0x9000u || !after.data.ds.seg.accessed ||
        fixture.memory[0x6015u] != 0x93u || fixture.memory[0x4015u] != 0u)
        return LIB_FALSE;
    /* Already-accessed data and handler code descriptors must load from a
     * readonly GDT even with supervisor write protection enabled. */
    entry = 0x6001u;
    lib_memory_copy(fixture.memory + 0x2010u, &entry, sizeof(entry));
    fixture.cpu.data.eip = 0u;
    if (!control_run(&fixture, load_ds, sizeof(load_ds), &after) ||
        fixture.delivered_exception.valid || after.data.ds.base != 0x9000u)
        return LIB_FALSE;
    fixture.cpu.data.eip = 0u;
    if (!control_run(&fixture, interrupt, sizeof(interrupt), &after) ||
        core_machine_cpu_is_shutdown(&fixture.execution) ||
        after.data.eip != 0x100u || after.data.cs.selector != 8u)
        return LIB_FALSE;
    /* Clearing Accessed now requires a real supervisor write and PF(3).
     * Keep the PF handler descriptor accessed, so delivery itself succeeds. */
    fixture.memory[0x6015u] = 0x92u;
    fixture.cpu.data.eip = 0u;
    fixture.delivered_exception.valid = LIB_FALSE;
    if (!control_run(&fixture, load_ds, sizeof(load_ds), &after) ||
        !fixture.delivered_exception.valid ||
        fixture.delivered_exception.exception_mask != VCPUINS_EXCEPT_PF ||
        fixture.delivered_exception.exception_code != 3u ||
        after.data.cr2 != 0x4010u || after.data.eip != 0x100u ||
        after.data.ds.base != 0x9000u || fixture.memory[0x6015u] != 0x92u)
        return LIB_FALSE;
    return LIB_TRUE;
}

static lib_bool control_test_486_split_descriptor(void)
{
    static const lib_u8 code[] = {0x8eu, 0xd8u};
    static const lib_u8 descriptor[] = {
        0xffu, 0xffu, 0u, 0x90u, 0u, 0x92u, 0u, 0u
    };
    lib_u8 local;

    for (local = 0u; local < 2u; ++local) {
        cpu_instruction_fixture fixture;
        t_cpu after;
        lib_u32 entry = 0x2003u;
        lib_u32 page;

        cpu_instruction_prepare(&fixture, CORE_MACHINE_CPU_PROFILE_80486SX);
        control_enter_protected(&fixture, 0u, LIB_FALSE);
        fixture.cpu.data.cr0 |= VCPU_CR0_PG | VCPU_CR0_WP;
        fixture.cpu.data.cr3 = 0x1000u;
        fixture.cpu.data.gdtr.base = 0x4ff4u;
        fixture.cpu.data.gdtr.limit = 0x0fu;
        fixture.cpu.data.ldtr.base = 0x4ff4u;
        fixture.cpu.data.ldtr.limit = 0x0fu;
        fixture.cpu.data.ldtr.selector = 0x18u;
        fixture.cpu.data.ldtr.flagValid = LIB_TRUE;
        fixture.cpu.data.eax = local ? 0x0cu : 8u;
        lib_memory_copy(fixture.memory + 0x1000u, &entry, sizeof(entry));
        for (page = 0u; page < 16u; ++page) {
            entry = (page << 12u) | 3u;
            if (page == 4u) entry = 0x6003u;
            if (page == 5u) entry = 0xa003u;
            lib_memory_copy(fixture.memory + 0x2000u + page * 4u,
                &entry, sizeof(entry));
        }
        lib_memory_copy(fixture.memory + 0x6ffcu, descriptor, 4u);
        lib_memory_copy(fixture.memory + 0xa000u, descriptor + 4u, 4u);
        if (!control_run(&fixture, code, sizeof(code), &after) ||
            fixture.delivered_exception.valid || after.data.ds.base != 0x9000u ||
            after.data.ds.selector != fixture.cpu.data.ax ||
            fixture.memory[0xa001u] != 0x93u)
            return LIB_FALSE;
        /* Both pages may become readonly once Accessed is already set. */
        entry = 0x6001u;
        lib_memory_copy(fixture.memory + 0x2010u, &entry, sizeof(entry));
        entry = 0xa001u;
        lib_memory_copy(fixture.memory + 0x2014u, &entry, sizeof(entry));
        fixture.cpu.data.eip = 0u;
        if (!control_run(&fixture, code, sizeof(code), &after) ||
            fixture.delivered_exception.valid || after.data.ds.base != 0x9000u)
            return LIB_FALSE;
    }
    return LIB_TRUE;
}

static lib_bool control_test_486_page_permissions(void)
{
    lib_u32 combination;

    /* Every PDE/PTE RW/US combination, WP value, CPL and access direction.
     * Keep code, exception tables and stack in a separate writable directory. */
    for (combination = 0u; combination < 256u; ++combination) {
        const lib_u32 pde_flags = 1u | ((combination & 3u) << 1u);
        const lib_u32 pte_flags = 1u | (((combination >> 2u) & 3u) << 1u);
        const lib_bool wp = (combination & 16u) != 0u;
        const lib_u8 cpl = (lib_u8)((combination >> 5u) & 3u);
        const lib_bool write = (combination & 128u) != 0u;
        const lib_bool readonly = (pde_flags & pte_flags & 2u) == 0u;
        const lib_bool supervisor = (pde_flags & pte_flags & 4u) == 0u;
        const lib_bool fault = (cpl == 3u && supervisor) ||
            (write && readonly && (wp || cpl == 3u));
        const lib_u8 code[] = {write ? 0xa2u : 0xa0u, 0u, 0u};
        const lib_u8 descriptor[] = {
            0xffu, 0xffu, 0u, 0u, 0u, (lib_u8)(0x9bu | (cpl << 5u)), 0u, 0u
        };
        const lib_u8 gate[] = {
            0u, 1u, (lib_u8)(8u | cpl), 0u, 0u, 0x8eu, 0u, 0u
        };
        cpu_instruction_fixture fixture;
        t_cpu after;
        lib_u32 entry = 0x2007u;
        lib_u32 page;

        cpu_instruction_prepare(&fixture, CORE_MACHINE_CPU_PROFILE_80486SX);
        control_enter_protected(&fixture, cpl, LIB_FALSE);
        fixture.cpu.data.cr0 |= VCPU_CR0_PG | (wp ? VCPU_CR0_WP : 0u);
        fixture.cpu.data.cr3 = 0x1000u;
        fixture.cpu.data.gdtr.base = 0x4000u;
        fixture.cpu.data.gdtr.limit = 0x0fu;
        fixture.cpu.data.idtr.base = 0x5000u;
        fixture.cpu.data.idtr.limit = 0x7ffu;
        fixture.cpu.data.ds.base = 0x00400000u;
        fixture.cpu.data.ds.selector = (lib_u16)(0x10u | cpl);
        fixture.cpu.data.ds.dpl = cpl;
        fixture.cpu.data.ss.selector = (lib_u16)(0x10u | cpl);
        fixture.cpu.data.ss.dpl = cpl;
        fixture.cpu.data.esp = 0x8000u;
        fixture.cpu.data.eax = 0x5au;
        lib_memory_copy(fixture.memory + 0x1000u, &entry, sizeof(entry));
        entry = 0x3000u | pde_flags;
        lib_memory_copy(fixture.memory + 0x1004u, &entry, sizeof(entry));
        entry = 0x9000u | pte_flags;
        lib_memory_copy(fixture.memory + 0x3000u, &entry, sizeof(entry));
        for (page = 0u; page < 16u; ++page) {
            entry = (page << 12u) | 7u;
            lib_memory_copy(fixture.memory + 0x2000u + page * 4u,
                &entry, sizeof(entry));
        }
        lib_memory_copy(fixture.memory + 0x4008u, descriptor, sizeof(descriptor));
        lib_memory_copy(fixture.memory + 0x5070u, gate, sizeof(gate));
        fixture.memory[0x9000u] = 0xa5u;
        if (!control_run(&fixture, code, sizeof(code), &after) ||
            core_machine_cpu_is_shutdown(&fixture.execution) ||
            fixture.delivered_exception.valid != fault ||
            after.data.eip != (fault ? 0x100u : sizeof(code)) ||
            fixture.memory[0x9000u] != (write && !fault ? 0x5au : 0xa5u) ||
            after.data.al != (!write && !fault ? 0xa5u : 0x5au))
            goto failed;
        if (fault) {
            const lib_u32 error = 1u | (write ? 2u : 0u) | (cpl == 3u ? 4u : 0u);
            lib_u32 stacked_error = 0u;

            lib_memory_copy(&stacked_error, fixture.memory + after.data.sp,
                sizeof(stacked_error));
            if (fixture.delivered_exception.exception_mask != VCPUINS_EXCEPT_PF ||
                fixture.delivered_exception.exception_code != error ||
                fixture.delivered_exception.point.eip != 0u ||
                after.data.cr2 != 0x00400000u || stacked_error != error)
                goto failed;
        }
        continue;
failed:
        lib_c_printf("486-page-permissions combination=%u eip=%x al=%x "
            "fault=%x delivered=%x cr2=%x memory=%x\n", combination,
            after.data.eip, (lib_u32)after.data.al, fixture.fault.exception_mask,
            fixture.delivered_exception.exception_mask, after.data.cr2,
            (lib_u32)fixture.memory[0x9000u]);
        return LIB_FALSE;
    }
    return LIB_TRUE;
}

static lib_bool control_test_486_split_pages(void)
{
    static const lib_u8 descriptor[] = {
        0xffu, 0xffu, 0u, 0u, 0u, 0x9bu, 0u, 0u
    };
    static const lib_u8 gate[] = {0u, 1u, 8u, 0u, 0u, 0x8eu, 0u, 0u};
    lib_u32 combination;

    for (combination = 0u; combination < 6u; ++combination) {
        const lib_bool write = (combination & 1u) != 0u;
        const lib_u32 flags = combination < 2u ? 3u : combination < 4u ? 0u : 1u;
        const lib_bool fault = flags == 0u || (write && flags == 1u);
        const lib_u8 code[] = {0x66u, write ? 0xa3u : 0xa1u, 0xffu, 0x0fu};
        cpu_instruction_fixture fixture;
        t_cpu after;
        lib_u32 entry = 0x2003u;
        lib_u32 page;
        lib_u32 first, second;

        cpu_instruction_prepare(&fixture, CORE_MACHINE_CPU_PROFILE_80486SX);
        control_enter_protected(&fixture, 0u, LIB_FALSE);
        fixture.cpu.data.cr0 |= VCPU_CR0_PG | VCPU_CR0_WP;
        fixture.cpu.data.cr3 = 0x1000u;
        fixture.cpu.data.gdtr.base = 0x4000u;
        fixture.cpu.data.gdtr.limit = 0x0fu;
        fixture.cpu.data.idtr.base = 0x5000u;
        fixture.cpu.data.idtr.limit = 0x7ffu;
        fixture.cpu.data.ds.selector = 0x10u;
        fixture.cpu.data.ds.base = 0x00400000u;
        fixture.cpu.data.esp = 0x8000u;
        fixture.cpu.data.eax = 0xaabbccddu;
        lib_memory_copy(fixture.memory + 0x1000u, &entry, sizeof(entry));
        entry = 0x3003u;
        lib_memory_copy(fixture.memory + 0x1004u, &entry, sizeof(entry));
        entry = 0x9003u;
        lib_memory_copy(fixture.memory + 0x3000u, &entry, sizeof(entry));
        entry = 0xb000u | flags;
        lib_memory_copy(fixture.memory + 0x3004u, &entry, sizeof(entry));
        for (page = 0u; page < 16u; ++page) {
            entry = (page << 12u) | 3u;
            lib_memory_copy(fixture.memory + 0x2000u + page * 4u,
                &entry, sizeof(entry));
        }
        lib_memory_copy(fixture.memory + 0x4008u, descriptor, sizeof(descriptor));
        lib_memory_copy(fixture.memory + 0x5070u, gate, sizeof(gate));
        fixture.memory[0x9fffu] = 0x11u;
        fixture.memory[0xb000u] = 0x22u;
        fixture.memory[0xb001u] = 0x33u;
        fixture.memory[0xb002u] = 0x44u;
        if (!control_run(&fixture, code, sizeof(code), &after) ||
            fixture.delivered_exception.valid != fault ||
            after.data.eip != (fault ? 0x100u : sizeof(code)) ||
            after.data.eax != (!write && !fault ? 0x44332211u : 0xaabbccddu) ||
            fixture.memory[0x9fffu] != (write && !fault ? 0xddu : 0x11u) ||
            fixture.memory[0xb000u] != (write && !fault ? 0xccu : 0x22u) ||
            fixture.memory[0xb001u] != (write && !fault ? 0xbbu : 0x33u) ||
            fixture.memory[0xb002u] != (write && !fault ? 0xaau : 0x44u))
            return LIB_FALSE;
        if (fault) {
            if (fixture.delivered_exception.exception_mask != VCPUINS_EXCEPT_PF ||
                fixture.delivered_exception.exception_code !=
                    ((flags ? 1u : 0u) | (write ? 2u : 0u)) ||
                after.data.cr2 != 0x00401000u)
                return LIB_FALSE;
        } else {
            lib_memory_copy(&first, fixture.memory + 0x3000u, sizeof(first));
            lib_memory_copy(&second, fixture.memory + 0x3004u, sizeof(second));
            if ((first & 0x60u) != (write ? 0x60u : 0x20u) ||
                (second & 0x60u) != (write ? 0x60u : 0x20u))
                return LIB_FALSE;
        }
    }
    return LIB_TRUE;
}

static lib_bool control_test_486_vm86_pages(void)
{
    static const lib_u8 privileged[][3] = {
        {0x0fu, 0x20u, 0xc0u}, {0x0fu, 0x22u, 0xc0u},
        {0x0fu, 0x20u, 0xd0u}, {0x0fu, 0x22u, 0xd0u},
        {0x0fu, 0x20u, 0xd8u}, {0x0fu, 0x22u, 0xd8u},
        {0x0fu, 0x06u, 0x90u}, {0x0fu, 0x01u, 0xf0u}
    };
    static const lib_u8 descriptors[] = {
        0xffu, 0xffu, 0u, 0u, 0u, 0x9bu, 0u, 0u,
        0xffu, 0xffu, 0u, 0u, 0u, 0x93u, 0u, 0u
    };
    static const lib_u8 gate[] = {0u, 1u, 8u, 0u, 0u, 0x8eu, 0u, 0u};
    lib_u32 combination;

    for (combination = 0u; combination < 24u; ++combination) {
        const lib_bool control = combination >= 16u;
        const lib_u32 flags = 1u | ((combination & 3u) << 1u);
        const lib_bool write = (combination & 4u) != 0u;
        const lib_bool fault = control || !(flags & 4u) || (write && !(flags & 2u));
        const lib_u8 code[] = {write ? 0xa2u : 0xa0u, 0u, 0x90u};
        cpu_instruction_fixture fixture;
        t_cpu after;
        lib_u32 entry = 0x2007u;
        const lib_u32 esp0 = 0x7000u;
        const lib_u16 ss0 = 0x10u;
        lib_u32 page;

        cpu_instruction_prepare(&fixture, CORE_MACHINE_CPU_PROFILE_80486SX);
        fixture.cpu.data.cr0 |= VCPU_CR0_PE | VCPU_CR0_PG | VCPU_CR0_TS |
            ((combination & 8u) ? VCPU_CR0_WP : 0u);
        fixture.cpu.data.eflags |= VCPU_EFLAGS_VM | VCPU_EFLAGS_IOPL;
        fixture.cpu.data.cr3 = 0x1000u;
        fixture.cpu.data.gdtr.base = 0x4000u;
        fixture.cpu.data.gdtr.limit = 0x17u;
        fixture.cpu.data.idtr.base = 0x5000u;
        fixture.cpu.data.idtr.limit = 0x7ffu;
        fixture.cpu.data.tr.selector = 0x18u;
        fixture.cpu.data.tr.base = 0x6000u;
        fixture.cpu.data.tr.limit = 0x67u;
        fixture.cpu.data.tr.flagValid = LIB_TRUE;
        fixture.cpu.data.tr.sys.type = VCPU_DESC_SYS_TYPE_TSS_32_BUSY;
        fixture.cpu.data.esp = 0x8000u;
        fixture.cpu.data.eax = 0x5au;
        lib_memory_copy(fixture.memory + 0x1000u, &entry, sizeof(entry));
        for (page = 0u; page < 16u; ++page) {
            entry = (page << 12u) | 7u;
            if (page == 9u) entry = 0xa000u | flags;
            lib_memory_copy(fixture.memory + 0x2000u + page * 4u,
                &entry, sizeof(entry));
        }
        lib_memory_copy(fixture.memory + 0x4008u, descriptors, sizeof(descriptors));
        lib_memory_copy(fixture.memory + 0x5070u, gate, sizeof(gate));
        lib_memory_copy(fixture.memory + 0x5068u, gate, sizeof(gate));
        lib_memory_copy(fixture.memory + 0x6004u, &esp0, sizeof(esp0));
        lib_memory_copy(fixture.memory + 0x6008u, &ss0, sizeof(ss0));
        fixture.memory[0xa000u] = 0xa5u;
        if (!control_run(&fixture, control ? privileged[combination - 16u] : code,
                sizeof(code), &after) ||
            core_machine_cpu_is_shutdown(&fixture.execution) ||
            fixture.delivered_exception.valid != fault ||
            after.data.eip != (fault ? 0x100u : sizeof(code)) ||
            fixture.memory[0xa000u] != (write && !fault ? 0x5au : 0xa5u) ||
            after.data.al != (!write && !fault ? 0xa5u : 0x5au))
            return LIB_FALSE;
        if (fault &&
            (fixture.delivered_exception.exception_mask !=
                 (control ? VCPUINS_EXCEPT_GP : VCPUINS_EXCEPT_PF) ||
             fixture.delivered_exception.exception_code != (control ? 0u : write ? 7u : 5u) ||
             after.data.cr2 != (control ? 0u : 0x9000u) || after.data.ss.selector != ss0 ||
             after.data.cs.selector != 8u || (after.data.eflags & VCPU_EFLAGS_VM)))
            return LIB_FALSE;
        if (after.data.cr0 != (0x60000010u | VCPU_CR0_PE | VCPU_CR0_PG |
                VCPU_CR0_TS | ((combination & 8u) ? VCPU_CR0_WP : 0u)) ||
            after.data.cr3 != 0x1000u)
            return LIB_FALSE;
        if (!fault && !(after.data.eflags & VCPU_EFLAGS_VM)) return LIB_FALSE;
    }
    return LIB_TRUE;
}

static lib_bool control_test_486_registers(void)
{
    static const lib_u8 write_cr0[] = {0x0fu, 0x22u, 0xc0u};
    static const lib_u8 write_cr3[] = {0x0fu, 0x22u, 0xd8u};
    static const lib_u8 read_cr3[] = {0x0fu, 0x20u, 0xd8u};
    static const lib_u8 lmsw[] = {0x0fu, 0x01u, 0xf0u};
    static const lib_u8 clts[] = {0x0fu, 0x06u};
    static const lib_u32 controls[] = {
        VCPU_CR0_ET,
        VCPU_CR0_ET | VCPU_CR0_CD,
        VCPU_CR0_ET | VCPU_CR0_CD | VCPU_CR0_NW,
        VCPU_CR0_ET | VCPU_CR0_NE | VCPU_CR0_WP | VCPU_CR0_AM |
            VCPU_CR0_MP | VCPU_CR0_EM | VCPU_CR0_TS,
        0u, 0x1ffaffc0u
    };
    static const lib_u32 invalid[] = {
        VCPU_CR0_ET | VCPU_CR0_PG,
        VCPU_CR0_ET | VCPU_CR0_NW
    };
    lib_size index;

    for (index = 0u; index < sizeof(controls) / sizeof(controls[0]); ++index) {
        cpu_instruction_fixture fixture;
        t_cpu after;

        cpu_instruction_prepare(&fixture, CORE_MACHINE_CPU_PROFILE_80486SX);
        fixture.cpu.data.eax = controls[index];
        if (!control_run(&fixture, write_cr0, sizeof(write_cr0), &after) ||
            fixture.delivered_exception.valid ||
            after.data.cr0 != ((controls[index] & 0xe005003fu) | VCPU_CR0_ET))
            return LIB_FALSE;
    }
    for (index = 0u; index < sizeof(invalid) / sizeof(invalid[0]); ++index) {
        cpu_instruction_fixture fixture;

        cpu_instruction_prepare(&fixture, CORE_MACHINE_CPU_PROFILE_80486SX);
        fixture.cpu.data.esp = 0x8000u;
        fixture.cpu.data.eax = invalid[index];
        if (!cpu_instruction_expect_real_fault(&fixture, write_cr0,
                sizeof(write_cr0), 13u))
            return LIB_FALSE;
    }
    for (index = 0u; index < 4u; ++index) {
        cpu_instruction_fixture fixture;
        t_cpu after;
        const lib_u32 directory = 0x12000u | ((lib_u32)index << 3u);

        cpu_instruction_prepare(&fixture, CORE_MACHINE_CPU_PROFILE_80486SX);
        fixture.cpu.data.eax = directory;
        if (!control_run(&fixture, write_cr3, sizeof(write_cr3), &after) ||
            fixture.delivered_exception.valid || after.data.cr3 != directory)
            return LIB_FALSE;
        fixture.cpu.data.eip = 0u;
        fixture.cpu.data.eax = 0u;
        if (!control_run(&fixture, read_cr3, sizeof(read_cr3), &after) ||
            after.data.eax != directory)
            return LIB_FALSE;
    }
    {
        cpu_instruction_fixture fixture;
        t_cpu after;
        const lib_u32 upper = VCPU_CR0_ET | VCPU_CR0_NE | VCPU_CR0_WP |
            VCPU_CR0_AM | VCPU_CR0_CD | VCPU_CR0_NW;

        cpu_instruction_prepare(&fixture, CORE_MACHINE_CPU_PROFILE_80486SX);
        fixture.cpu.data.cr0 = upper;
        fixture.cpu.data.eax = VCPU_CR0_MP | VCPU_CR0_EM | VCPU_CR0_TS;
        if (!control_run(&fixture, lmsw, sizeof(lmsw), &after) ||
            after.data.cr0 != (upper | fixture.cpu.data.eax))
            return LIB_FALSE;
        fixture.cpu.data.eip = 0u;
        if (!control_run(&fixture, clts, sizeof(clts), &after) ||
            after.data.cr0 != (upper | VCPU_CR0_MP | VCPU_CR0_EM))
            return LIB_FALSE;
    }
    return LIB_TRUE;
}

int main(void)
{
    if (!control_test_486_split_pages()) {
        lib_c_printf("control-state stage=486-split-pages\n");
        return 1;
    }
    if (!control_test_486_vm86_pages()) {
        lib_c_printf("control-state stage=486-vm86-pages\n");
        return 1;
    }
    if (!control_test_486_split_descriptor()) {
        lib_c_printf("control-state stage=486-split-descriptor\n");
        return 1;
    }
    if (!control_test_486_page_permissions()) {
        lib_c_printf("control-state stage=486-page-permissions\n");
        return 1;
    }
    if (!control_test_486_paged_tables()) {
        lib_c_printf("control-state stage=486-paged-tables\n");
        return 1;
    }
    if (!control_test_486_registers()) {
        lib_c_printf("control-state stage=486-registers\n");
        return 1;
    }
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
    lib_c_printf("CONTROL-STATE:OK\n");
    return 0;
}
