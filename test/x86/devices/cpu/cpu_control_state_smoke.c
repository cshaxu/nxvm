#include "support/cpu_instruction_fixture.h"
#include <stdio.h>

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
        control_seed(&fixture);
        fixture.cpu.data.cr0 = 0x00a5000du;
        before = fixture.cpu;
        if (!control_run(&fixture, smsw, sizeof(smsw), &after) ||
            after.data.eax != 0xdead000du || after.data.cr0 !=
            before.data.cr0 || after.data.eflags != before.data.eflags) return 0;
        cpu_instruction_prepare(&fixture, profiles[profile]);
        control_seed(&fixture);
        fixture.cpu.data.eax = 0xdead000du;
        fixture.cpu.data.cr0 = 0x00a50000u;
        before = fixture.cpu;
        if (!control_run(&fixture, lmsw, sizeof(lmsw), &after) ||
            !control_state_equal(&before, &after) || after.data.cr0 !=
            0x00a5000du) return 0;
        cpu_instruction_prepare(&fixture, CORE_MACHINE_CPU_PROFILE_80386);
        control_seed(&fixture);
        fixture.cpu.data.cr0 = 0x00a5000cu;
        lib_memory_copy(fixture.memory + 0x0400u, &image, sizeof(image));
        before = fixture.cpu;
        if (!control_run(&fixture, smsw_memory, sizeof(smsw_memory), &after) ||
            !control_state_equal(&before, &after)) return 0;
        lib_memory_copy(&image, fixture.memory + 0x0400u, sizeof(image));
        if (image != 0x1122000cu) return 0;
        cpu_instruction_prepare(&fixture, CORE_MACHINE_CPU_PROFILE_80386);
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

int main(void)
{
    if (!control_test_clts()) {
        printf("control-state stage=clts\n");
        return 1;
    }
    if (!control_test_msw()) {
        printf("control-state stage=msw\n");
        return 1;
    }
    if (!control_test_msw_attributes()) {
        printf("control-state stage=msw-attributes\n");
        return 1;
    }
    if (!control_test_protected_state()) {
        printf("control-state stage=protected\n");
        return 1;
    }
    if (!control_test_privilege_and_rollback()) {
        printf("control-state stage=privilege-rollback\n");
        return 1;
    }
    if (!control_test_mov_cr()) {
        printf("control-state stage=mov-cr\n");
        return 1;
    }
    printf("M5:T539:S45:CONTROL-STATE:OK\n");
    return 0;
}
